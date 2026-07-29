#include "camera.h"

#include <cmath>

glm::vec3 OrbitalCamera::eye(const glm::vec3& center) const {
    return glm::vec3(
        center.x + radius * std::sin(theta) * std::cos(phi),
        center.y + radius * std::sin(phi),
        center.z + radius * std::cos(theta) * std::cos(phi)
    );
}
