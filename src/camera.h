#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

// =============================================================================
// Cámara orbital esférica.
//   theta  — ángulo horizontal (rad)
//   phi    — ángulo vertical   (rad)
//   radius — distancia al centro de interés
// =============================================================================
struct OrbitalCamera {
    float theta   = 0.6f;
    float phi     = 0.5f;
    float radius  = 14.0f;

    double lastMX = 0.0, lastMY = 0.0;
    bool   dragging = false;

    // Retorna la posición del ojo en coordenadas cartesianas.
    glm::vec3 eye(const glm::vec3& center) const;
};

#endif // CAMERA_H
