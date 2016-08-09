#include "Object.h"

Material::Material():
    ambient(1.0f),
    diffuse(1.0f),
    specular(1.0f),
    specularIntensity(10.0f),
    reflection(0.0f)
  {}

Object::Object(const glm::mat4 &transform, const Material &material):
    transform(transform),
    material(material)
  {}


/* TODO: Implement */
bool Sphere::Intersect(const Ray &ray, IntersectInfo &info) const {

    float a = glm::dot(ray.direction,ray.direction);
    float b = glm::dot(2.0f * ray.direction, (ray.origin - origin));
    float c = glm::dot(ray.origin - origin, ray.origin - origin) - pow(radius, 2);

    float discriminant = pow(b, 2) - (4.0f * a * c);

    if (discriminant <= 0) {
        // case for when ray doesn't intersect
        return false;

    } else {
        // case for when ray intersects
        // we only require the "-" as we want the smallest, closest value
        float time = (-b - sqrt(discriminant)) / (2.0f * a);

        if (time < 0) {
            // case for when object is behind camera
            return false;

        } else {

            info.hitPoint = ray.origin + time * ray.direction;
            info.time = glm::length(ray.origin - info.hitPoint);
            info.normal = glm::normalize(info.hitPoint - origin);
            info.material = MaterialPtr();

            return true;
        }

    }

}

/* TODO: Implement */
bool Plane::Intersect(const Ray &ray, IntersectInfo &info) const {

    float a = glm::dot(ray.direction, normal);
    float time = (glm::dot((point-ray.origin), normal)) / a;

    if (a == 0 || time < 0) {
        // case for ray parallel to plane and object behind camera
        return false;
    }

    info.hitPoint = ray.origin + time * ray.direction;
    info.time = glm::length(ray.origin - info.hitPoint);
    info.normal = normal;
    info.material = MaterialPtr();

    return true;

}

/* TODO: Implement */
bool Triangle::Intersect(const Ray &ray, IntersectInfo &info) const {

    glm::vec3 edgeA = pointB - pointA;
    glm::vec3 edgeB = pointC - pointB;
    glm::vec3 edgeC = pointA - pointC;

    glm::vec3 normal = glm::normalize(glm::cross(edgeA, edgeB));

    float a = glm::dot(ray.direction, normal);
    float time = glm::dot((pointA - ray.origin), normal) / a;

    if (a == 0 || time <= 0) {
        // case for ray parallel to plane and object behind camera
        return false;
    }

    glm::vec3 hitPoint = ray.origin + (time * ray.direction);
    glm::vec3 hitA =  hitPoint - pointA;
    glm::vec3 hitB =  hitPoint - pointB;
    glm::vec3 hitC =  hitPoint - pointC;

    float determinantA = glm::dot(normal, glm::cross(edgeA, hitA));
    float determinantB = glm::dot(normal, glm::cross(edgeB, hitB));
    float determinantC = glm::dot(normal, glm::cross(edgeC, hitC));

    if (determinantA <= 0 || determinantB <= 0 || determinantC <= 0) {
        return false;
    }

    info.hitPoint = hitPoint;
    info.time = glm::length(ray.origin - info.hitPoint);
    info.normal = normal;
    info.material = MaterialPtr();

    return true;


}
