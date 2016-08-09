—RUNNING THE PROJECT—
Should be the same as mentioned in the handout. cd into /cw2 and type “make run”.

—SPHERE & INTERSECTIONS—
My initial approach was to get a shape just appearing on screen. I implemented the sphere object, following the discriminant functions in the lecture slides.
Knowing the position the camera was located at and was pointing towards, I was able to set the sphere to be seen at origin (0,0,0). After sufficient fiddling with the discriminant function, displaying nothing for no intersection and displaying white for >0 intersections, I was eventually able to get a white circle to appear on screen. 

—PHONG ILLUMINATION & SHADOWS—
This was further built upon by applying Phong Illumination to the sphere. The illumination is implemented in the “GetPhong()) method. It simply calculates an ambient, diffuse and specular value before returning the three summed together. 

The diffuse value is calculated by the intensity of the light at the point. If the angle is near the normal, the intensity increases. The ambient value is calculated by again the intensity of the light at the point. This value is used for shadows too. For this to be used solely for shadows, a shadow ray is cast from the intersection point to the light source. When there is an intersection before the light source, we cast a shadow ray from the intersection point to the light source. If here is an intersection before the light source, this point is in shadow from the object and the isShadow is true. This is carried out in the isShadow() method.

This method was extended later (post planes) on to include the “inShadow” parameter, which sets the function to only return the ambient value. 

—PLANES & TRIANGLES—
Once the sphere looked good against its red background, I added the plane objects. It was a far more straightforward implementation That the sphere. Given that I was only checking for the object being behind the camera and the ray being parallel to the plane. Three were then added to create the environment. 

The implementation for the triangles was similarly easy. I took in the three points and created the 3 edges using that to check for intersections. 

—REFLECTIONS—
I calculated the reflections by sending rays originating through an intersection point along the reflection vector. A ratio of the reflection rays colour, taken from the original intersection point is applied based on the supplied reflection parameter. This is done recursively due to the ray “bouncing” off other objects and redoing the process. I have applied the recursion limit “numBounces“ to 5 due to the relatively small size of the objects on screen. 

—REFRACTIONS—
A refraction amount and index are passed in as parameters and the refractive ray is calculated by bending the original ray around the angle of incidence and the indices of refraction between the two materials. The ray bounces inside the object only once before leaving. 

—RENDERS—
Two renders can be seen in the “/renders” folder. One shows the scene without reflective planes to show simply the items in the scene. The other shows the two side planes mirroring what is in the scene. I drew a Scotland flag with spheres because we are in Scotland.