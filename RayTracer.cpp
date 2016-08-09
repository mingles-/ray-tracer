#include "RayTracer.h"

int windowX = 640;
int windowY = 480;

/*
** std::vector is a data format similar with list in most of  script language, which allows users to change its size after claiming.
** The difference is that std::vector is based on array rather than list, so it is not so effective when you try to insert a new element, but faster while calling for values randomly or add elements by order.
*/
std::vector<Object*> objects;

// Lighting constants
const glm::vec3 lightSource(-6, 6, 2);
const glm::vec3 lightIntensity(1, 1, 1);
//const float specularIntensity = 10.0;

void cleanup() {
	for(unsigned int i = 0; i < objects.size(); ++i){
		if(objects[i]){
			delete objects[i];
		}
	}
}

/*
** TODO: Function for testing intersection against all the objects in the scene
**
** If an object is hit then the IntersectionInfo object should contain
** the information about the intersection. Returns true if any object is hit,
** false otherwise
**
*/
bool CheckIntersection(const Ray &ray, IntersectInfo &info) {

    float minDepth = LONG_MAX;
    int closestObject = 0;

    for (int i = 0; i < objects.size(); i++ ){
        if (objects[i] -> Intersect(ray, info)) {
            if (info.time < minDepth) {
                minDepth = info.time;
                closestObject = i;
            }
        }
    }

    return objects[closestObject]->Intersect(ray, info);

	//You need to add your own solution for this funtion, to replace the 'return true'.
	//Runing the function Intersect() of each of the objects would be one of the options.
	//Each time when intersect happens, the payload of ray will need to be updated, but that's no business of this function.
	//To make it clear, keyword const prevents the function from changing parameter ray.
}

// checks to see if the object hit by the ray is in shadow
bool hasShadow(const IntersectInfo info) {

    // setting an offset slightly above the surface for floating point errors
    float threshold = 0.01;

    Ray light = Ray(info.hitPoint, lightSource - info.hitPoint);
    glm::vec3 withOffset = light(threshold);
    Ray shadow = Ray(withOffset, lightSource);

    float lightDistance = glm::length(lightSource - shadow.origin);
    IntersectInfo shadowIntersect;

    // checking to see if there is intersections between shadow ray and objects and if that intersect isn't behind light source
    for (int i=0; i<objects.size(); i++) {
        if (objects[i]->Intersect(shadow, shadowIntersect) && shadowIntersect.time < lightDistance) {
            return true;
        }
    }

    return false;
}

glm::vec3 GetPhong(const Ray &ray, IntersectInfo &info, bool inShadow) {

    glm::vec3 illumination;
    glm::vec3 specular;
    glm::vec3 diffuse;
    glm::vec3 ambient;

    glm::vec3 n = info.normal;
    glm::vec3 l = glm::normalize(lightSource - info.hitPoint);
    glm::vec3 v = glm::normalize(ray.origin - info.hitPoint);

    // calculate diffuse
    float cosTheta = fmax(0, glm::dot(l, n));
    diffuse[0] = lightIntensity[0] * info.material->diffuse[0] * cosTheta;
    diffuse[1] = lightIntensity[1] * info.material->diffuse[1] * cosTheta;
    diffuse[2] = lightIntensity[2] * info.material->diffuse[2] * cosTheta;

    // calculate ambient
    ambient[0] = lightIntensity[0] * info.material->ambient[0];
    ambient[1] = lightIntensity[1] * info.material->ambient[1];
    ambient[2] = lightIntensity[2] * info.material->ambient[2];

    // calculate specular
    glm::vec3 r = glm::normalize((2.0f * n * glm::dot(l, n)) - l);
    float cosAlpha = fmax(0, glm::dot(r, v));
    specular[0] = lightIntensity[0] * info.material->specular[0] * pow(cosAlpha, info.material->specularIntensity);
    specular[1] = lightIntensity[1] * info.material->specular[1] * pow(cosAlpha, info.material->specularIntensity);
    specular[2] = lightIntensity[2] * info.material->specular[2] * pow(cosAlpha, info.material->specularIntensity);

    if (!inShadow) {
        // if not in shadow, add them all up
        illumination[0] = specular[0] + diffuse[0] + ambient[0];
        illumination[1] = specular[1] + diffuse[1] + ambient[1];
        illumination[2] = specular[2] + diffuse[2] + ambient[2];

    } else {
        // if in shadow, just return the ambient illumination
        illumination[0] = ambient[0];
        illumination[1] = ambient[1];
        illumination[2] = ambient[2];

    }


    return illumination;

}

glm::vec3 GetReflection(const Ray &ray, IntersectInfo &info, Payload &payload, glm::vec3 color) {

    if (payload.numBounces < 5) {

        // setting an offset slightly above the surface for floating point errors
        float threshold = 0.01;

        // create reflection ray
        glm::vec3 direction = ray.direction - 2 * (glm::dot(ray.direction, info.normal)) * info.normal;
        glm::vec3 withOffset = Ray(info.hitPoint, direction)(threshold);
        Ray reflection = Ray(withOffset, direction);
        CastRay(reflection, payload);
        payload.numBounces += 1;
    }

    // get reflected color
    float reflection = info.material->reflection;
    glm::vec3 reflColor = reflection * payload.color;
    glm::vec3 reflectMix = reflColor + ((1-reflection) * color);

    return reflectMix;
}

glm::vec3 GetRefraction(const Ray &ray, IntersectInfo &info, Payload &payload, glm::vec3 reflectMix) {

    if (info.material->refraction > 0 || !payload.hasRefracted) {

        payload.hasRefracted = true;
        float refractionLevel = 0;
        float ratio = -1.0 / info.material->refraction;

        // Compute the direction of the refraction ray
        float radialDistance =  1.0 - powf(ratio, 2.0) * (1.0 - powf(glm::dot(info.normal,-ray.direction), 2.0));

        if (radialDistance > 0) {

            // setting an offset slightly above the surface for floating point errors
            float threshold = 0.01;
            refractionLevel = info.material->refractiveIndex;

            // create reflection ray
            glm::vec3 direction = (ratio * (glm::dot(info.normal,-ray.direction)) - sqrtf(radialDistance)) * info.normal - (ratio * -ray.direction);
            glm::vec3 withOffset = Ray(info.hitPoint, direction)(threshold);
            Ray refraction = Ray(withOffset, direction);

            CastRay(refraction, payload);

        }

        glm::vec3 refractionMix = (refractionLevel * payload.color) + ((1-refractionLevel) * reflectMix);

        return  refractionMix;

    } else {

        return reflectMix;
    }
}


/*
** TODO: Recursive ray-casting function. It might be the most important Function in this demo cause it's the one decides the color of pixels.
**
** This function is called for each pixel, and each time a ray is reflected/used
** for shadow testing. The Payload object can be used to record information about
** the ray trace such as the current color and the number of bounces performed.
** This function should return either the time of intersection with an object
** or minus one to indicate no intersection.
*/
//	The function CastRay() will have to deal with light(), shadow() and reflection(). The impement of them would also be important.
float CastRay(Ray &ray, Payload &payload) {

	IntersectInfo info;

	if (CheckIntersection(ray,info)) {
		/* TODO: Set payload color based on object materials, not direction */

        // COLOR & SHADOWS
        glm::vec3 color = GetPhong(ray, info, hasShadow(info));

        // REFLECTION
        glm::vec3 reflectMix = GetReflection(ray, info, payload, color);

        // REFRACTION
        glm::vec3 refractMix = GetRefraction(ray, info, payload, reflectMix);

        payload.color =  refractMix;

		return info.time;

	} else {

		payload.color = glm::vec3(0.0f);
		// The Ray from camera hits nothing so nothing will be seen. In this case, the pixel should be totally black.
		return -1.0f;
	}
}


// Render Function

// This is the main render function, it draws pixels onto the display
// using GL_POINTS. It is called every time an update is required.

// This function transforms each pixel into the space of the virtual
// scene and casts a ray from the camera in that direction using CastRay
// And for rendering,
// 1)Clear the screen so we can draw a new frame
// 2)Cast a ray into the scene for each pixel on the screen and use the returned color to render the pixel
// 3)Flush the pipeline so that the instructions we gave are performed.

void Render()  {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// Clear OpenGL Window

	//	Three parameters of lookat(vec3 eye, vec3 center, vec3 up).
	glm::mat4 viewMatrix = glm::lookAt(glm::vec3(-10.0f,10.0f,10.0f), glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,1.0f,0.0f));
	glm::mat4 projMatrix = glm::perspective(45.0f, (float)windowX / (float)windowY, 1.0f, 10000.0f);

	glBegin(GL_POINTS);	//Using GL_POINTS mode. In this mode, every vertex specified is a point.
	//	Reference https://en.wikibooks.org/wiki/OpenGL_Programming/GLStart/Tut3 if interested.

	for(int x = 0; x < windowX; ++x)
		for(int y = 0; y < windowY; ++y){//Cover the entire display zone pixel by pixel, but without showing.
			float pixelX =  2*((x+0.5f)/windowX)-1;	//Actually, (pixelX, pixelY) are the relative position of the point(x, y).
			float pixelY = -2*((y+0.5f)/windowY)+1;	//The displayzone will be decribed as a 2.0f x 2.0f platform and coordinate origin is the center of the display zone.

			//	Decide the direction of each of the ray.
			glm::vec4 worldNear = glm::inverse(viewMatrix) * glm::inverse(projMatrix) * glm::vec4(pixelX, pixelY, -1, 1);
			glm::vec4 worldFar  = glm::inverse(viewMatrix) * glm::inverse(projMatrix) * glm::vec4(pixelX, pixelY,  1, 1);
			glm::vec3 worldNearPos = glm::vec3(worldNear.x, worldNear.y, worldNear.z) / worldNear.w;
			glm::vec3 worldFarPos  = glm::vec3(worldFar.x, worldFar.y, worldFar.z) / worldFar.w;

			Payload payload;
			Ray ray(worldNearPos, glm::normalize(glm::vec3(worldFarPos - worldNearPos))); //Ray(const glm::vec3 &origin, const glm::vec3 &direction)

			if(CastRay(ray,payload) > 0.0f){
				glColor3f(payload.color.x,payload.color.y,payload.color.z);
			}
			else {
				glColor3f(1,0,0);
			}

			glVertex3f(pixelX,pixelY,0.0f);
		}

	glEnd();
	glFlush();
}



int main(int argc, char **argv) {

  	//initialise OpenGL
	glutInit(&argc, argv);
	//Define the window size with the size specifed at the top of this file
	glutInitWindowSize(windowX, windowY);

	//Create the window for drawing
	glutCreateWindow("RayTracer");
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

	//Set the function demoDisplay (defined above) as the function that
	//is called when the window must display.
	glutDisplayFunc(Render);

	//	TODO: Add Objects to scene
	//	This part is related to function CheckIntersection().
	//	Being added into scene means that the object will take part in the intersection checking, so try to make these two connected to each other.


    glm::mat4 sphereTransform(0.0f);
    Material blueSphereMaterial(glm::vec3(0.1, 0.1, 0.1), glm::vec3(0.1, 0.6, 1), glm::vec3(0.1, 0.1, 0.1), 25.0, 0.1, 0, 1);
    Material whiteSphereMaterial(glm::vec3(0.3, 0.3, 0.3), glm::vec3(1, 1, 1), glm::vec3(0.6, 0.6, 0.6), 50.0, 0.1, 0, 1);

    Material refractedWhiteSphereMaterial(glm::vec3(0.3, 0.3, 0.3), glm::vec3(1, 1, 1), glm::vec3(0.6, 0.6, 0.6), 50.0, 0.1, 1, 0.6);
    Material refractedRedSphereMaterial(glm::vec3(0.3, 0.1, 0.1), glm::vec3(0.6, 0.3, 0.1), glm::vec3(0.3, 0.1, 0.1), 50.0, 0.1, 1, 0.6);
    Material refractedGreenSphereMaterial(glm::vec3(0.1, 0.3, 0.1), glm::vec3(0.1, 0.9, 0.1), glm::vec3(0.1, 0.3, 0.1), 50.0, 0.1, 1, 0.6);

		Material refractedBlueSphereMaterial(glm::vec3(0.1, 0.1, 0.1), glm::vec3(0.1, 0.6, 1), glm::vec3(0.1, 0.1, 0.1), 50.0, 0.1, 1, 0.6);

    glm::mat4 planeTransform(0.0f);
    Material planeMaterial(glm::vec3(0.09, 0.09, 0.09), glm::vec3(1, 1, 1), glm::vec3(0, 0, 0), 25.0, 0.8, 0, 1);
    Material floorMaterial(glm::vec3(0.09, 0.09, 0.09), glm::vec3(1, 1, 1), glm::vec3(0, 0, 0), 25.0, 0.1, 0, 1);

    glm::mat4 triangleTransform(0.0f);
    Material triangleMaterial(glm::vec3(0.1, 0.1, 0.1), glm::vec3(0.6, 0.2, 0.2), glm::vec3(0.1, 0.1, 0.1), 25.0, 0.1, 0, 1);


    Sphere blueSphere3(sphereTransform, blueSphereMaterial, glm::vec3(-2.5, 0.5, 0.5), 0.5);
    Sphere blueSphere5(sphereTransform, blueSphereMaterial, glm::vec3(-2.5, 0.5, 1.5), 0.5);
    Sphere blueSphere7(sphereTransform, blueSphereMaterial, glm::vec3(-3.5, 0.5, 1.5), 0.5);
    Sphere blueSphere8(sphereTransform, blueSphereMaterial, glm::vec3(-3.5, 0.5, 0.5), 0.5);
    Sphere blueSphere9(sphereTransform, blueSphereMaterial, glm::vec3(-3.5, 0.5, 2.5), 0.5);

    Sphere blueSphere2(sphereTransform, blueSphereMaterial, glm::vec3(-0.5, 0.5, 1.5), 0.5);
    Sphere blueSphere4(sphereTransform, blueSphereMaterial, glm::vec3(-0.5, 0.5, 2.5), 0.5);
    Sphere blueSphere6(sphereTransform, blueSphereMaterial, glm::vec3(-1.5, 0.5, 2.5), 0.5);
    Sphere blueSphere10(sphereTransform, blueSphereMaterial, glm::vec3(-0.5, 0.5, 3.5), 0.5);
    Sphere blueSphere11(sphereTransform, blueSphereMaterial, glm::vec3(-1.5, 0.5, 3.5), 0.5);
    Sphere blueSphere12(sphereTransform, blueSphereMaterial, glm::vec3(-2.5, 0.5, 3.5), 0.5);

    Sphere blueSphere13(sphereTransform, blueSphereMaterial, glm::vec3(-4.5, 0.5, 0.5), 0.5);
    Sphere blueSphere14(sphereTransform, blueSphereMaterial, glm::vec3(-4.5, 0.5, 1.5), 0.5);
    Sphere blueSphere15(sphereTransform, blueSphereMaterial, glm::vec3(-4.5, 0.5, 2.5), 0.5);
    Sphere blueSphere16(sphereTransform, blueSphereMaterial, glm::vec3(-4.5, 0.5, 3.5), 0.5);

    Sphere blueSphere17(sphereTransform, blueSphereMaterial, glm::vec3(-0.5, 0.5, 4.5), 0.5);
    Sphere blueSphere18(sphereTransform, blueSphereMaterial, glm::vec3(-1.5, 0.5, 4.5), 0.5);
    Sphere blueSphere19(sphereTransform, blueSphereMaterial, glm::vec3(-2.5, 0.5, 4.5), 0.5);
    Sphere blueSphere20(sphereTransform, blueSphereMaterial, glm::vec3(-3.5, 0.5, 4.5), 0.5);



    Sphere whiteSphere(sphereTransform, whiteSphereMaterial, glm::vec3(-0.5, 0.5, 0.5), 0.5);
    Sphere whiteSphere2(sphereTransform, whiteSphereMaterial, glm::vec3(-1.5, 0.5, 1.5), 0.5);
    Sphere whiteSphere3(sphereTransform, whiteSphereMaterial, glm::vec3(-2.5, 0.5, 2.5), 0.5);
    Sphere whiteSphere4(sphereTransform, whiteSphereMaterial, glm::vec3(-3.5, 0.5, 3.5), 0.5);
    Sphere whiteSphere5(sphereTransform, whiteSphereMaterial, glm::vec3(-4.5, 0.5, 4.5), 0.5);

    Plane plane1(planeTransform, floorMaterial, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    Plane plane2(planeTransform, planeMaterial, glm::vec3(0, 0, 0), glm::vec3(-1, 0, 0));
    Plane plane3(planeTransform, planeMaterial, glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));

    Triangle triangle(triangleTransform, blueSphereMaterial, glm::vec3(-1,0,5),glm::vec3(-1,0,7),glm::vec3(-1,2,6));

    Sphere refractedWhiteSphere(sphereTransform, refractedWhiteSphereMaterial, glm::vec3(-3.8, 0.75, 3.4), 0.75);
    Sphere refractedRedSphere(sphereTransform, refractedRedSphereMaterial, glm::vec3(-4.5, 1, 1.8), 1);
    Sphere refractedGreenSphere(sphereTransform, refractedGreenSphereMaterial, glm::vec3(-2, 1.5, 2), 1.5);

		Sphere blueSphere(sphereTransform, refractedBlueSphereMaterial, glm::vec3(-2.3, 0.6, 3.9), 0.6);

    objects.push_back(&refractedWhiteSphere);
    objects.push_back(&refractedRedSphere);
    objects.push_back(&refractedGreenSphere);
    objects.push_back(&blueSphere);
    // objects.push_back(&blueSphere5);
    // objects.push_back(&blueSphere6);
		//
    // objects.push_back(&blueSphere7);
    // objects.push_back(&blueSphere8);
    // objects.push_back(&blueSphere9);
    // objects.push_back(&blueSphere10);
    // objects.push_back(&blueSphere11);
    // objects.push_back(&blueSphere12);
		//
    // objects.push_back(&blueSphere13);
    // objects.push_back(&blueSphere14);
    // objects.push_back(&blueSphere15);
    // objects.push_back(&blueSphere16);
		//
    // objects.push_back(&blueSphere17);
    // objects.push_back(&blueSphere18);
    // objects.push_back(&blueSphere19);
    // objects.push_back(&blueSphere20);

    // objects.push_back(&whiteSphere);
    // objects.push_back(&whiteSphere2);
    // objects.push_back(&whiteSphere3);
    // objects.push_back(&whiteSphere4);
    // objects.push_back(&whiteSphere5);

//    objects.push_back(&refractedWhiteSphere);
//    objects.push_back(&refractedRedSphere);
//    objects.push_back(&refractedGreenSphere);
//
   objects.push_back(&triangle);

    objects.push_back(&plane1);
    objects.push_back(&plane2);
    objects.push_back(&plane3);

    atexit(cleanup);
    glutMainLoop();
}
