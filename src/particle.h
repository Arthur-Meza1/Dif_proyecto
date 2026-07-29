#ifndef PARTICLE_H
#define PARTICLE_H

#include <glm/glm.hpp>

// =============================================================================
// Partícula material con masa, posición y velocidad.
//   inv_mass = 1/m  (0 para partículas fijas / masa infinita).
// =============================================================================
struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    float     inv_mass;
};

// =============================================================================
// Resorte que conecta dos partículas.
//   i1, i2      — índices dentro del arreglo de partículas
//   rest_length — longitud de reposo (sin deformar)
//   stiffness   — constante k de la Ley de Hooke
// =============================================================================
struct Spring {
    int   i1, i2;
    float rest_length;
    float stiffness;
};

#endif // PARTICLE_H
