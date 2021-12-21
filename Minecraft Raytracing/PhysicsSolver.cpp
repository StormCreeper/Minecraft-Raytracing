#include "PhysicsSolver.h"

bool collide(const AABB& a, const AABB& b) {
    glm::vec3 amin = a.corner;
    glm::vec3 amax = a.corner + a.dim;
    glm::vec3 bmin = b.corner;
    glm::vec3 bmax = b.corner + b.dim;
    return (amin.x <= bmax.x && amax.x >= bmin.x) &&
           (amin.y <= bmax.y && amax.y >= bmin.y) &&
           (amin.z <= bmax.z && amax.z >= bmin.z);
}

void intersection(const AABB& a, const AABB& b, AABB& c) {
    glm::vec3 amin = a.corner;
    glm::vec3 amax = a.corner + a.dim;
    glm::vec3 bmin = b.corner;
    glm::vec3 bmax = b.corner + b.dim;

    glm::vec3 cmin = max(amin, bmin);
    glm::vec3 cmax = min(amax, bmax);
    
    c.corner = cmin;
    c.dim = cmax - cmin;
}
