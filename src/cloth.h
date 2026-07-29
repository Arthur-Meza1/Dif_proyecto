#ifndef CLOTH_H
#define CLOTH_H

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <vector>

#include "particle.h"

// =============================================================================
// Simulador de tela basado en sistema masa-resorte.
// Modelo:  m·a = F_g + F_s + F_d + F_wind
// Integración: Euler Semi-Implícito con sub-pasos.
// =============================================================================
class ClothSimulator {
public:
    ClothSimulator()  = default;
    ~ClothSimulator() { cleanup(); }

    // --- Ciclo de vida -------------------------------------------------------
    bool init(int gridW, int gridH, float spacing);
    void update(float frameDt, int substeps);
    void render(const glm::mat4& view, const glm::mat4& proj);
    void cleanup();
    void reset();

    // --- Control de fuerzas --------------------------------------------------
    void setGravityEnabled(bool v)  { m_gravOn = v; }
    void setSpringsEnabled(bool v)  { m_springOn = v; }
    void setDampingEnabled(bool v)  { m_dampOn = v; }
    void setWindEnabled(bool v)     { m_windOn = v; }

    bool gravityEnabled()  const { return m_gravOn; }
    bool springsEnabled()  const { return m_springOn; }
    bool dampingEnabled()  const { return m_dampOn; }
    bool windEnabled()     const { return m_windOn; }

    // --- Parámetros ajustables -----------------------------------------------
    void   setStiffness(float k);
    void   setDamping(float c);
    void   setParticleMass(float m);
    float  stiffness()     const { return m_stiffness; }
    float  damping()       const { return m_damping; }
    float  particleMass()  const { return m_particleMass; }

    // --- Visualización adicional ---------------------------------------------
    void   setShowTrail(bool v)   { m_showTrail = v; }
    bool   showTrail()     const { return m_showTrail; }
    void   setShowArrows(bool v)  { m_showArrows = v; }
    bool   showArrows()    const { return m_showArrows; }
    void   setPaused(bool v)      { m_paused = v; }
    bool   paused()        const { return m_paused; }
    void   singleStep();

    void   renderTrail(const glm::mat4& view, const glm::mat4& proj);
    void   renderArrows(const glm::mat4& view, const glm::mat4& proj);

    // --- Consultas -----------------------------------------------------------
    glm::vec3 getCenter()       const;
    float     getMinY()         const { return m_minY; }
    float     getHeightRange()  const { return m_heightRange; }
    int       gridW()           const { return m_gridW; }
    int       gridH()           const { return m_gridH; }

private:
    // Partículas y resortes
    std::vector<Particle> m_particles;
    std::vector<Spring>   m_springs;
    int   m_gridW    = 0;
    int   m_gridH    = 0;
    float m_spacing  = 0.0f;
    float m_simTime  = 0.0f;

    // Estadísticas
    float m_minY        = 0.0f;
    float m_maxY        = 0.0f;
    float m_heightRange = 1.0f;

    // Banderas de control
    bool m_gravOn   = true;
    bool m_springOn = true;
    bool m_dampOn   = true;
    bool m_windOn   = true;
    bool m_paused   = false;

    // Parámetros modificables
    float m_stiffness     = 180.0f;
    float m_damping       = 2.0f;
    float m_particleMass  = 0.08f;

    // Visualización
    bool  m_showTrail  = true;
    bool  m_showArrows = false;
    int   m_trailIdx   = 0;              // índice de la partícula trazada

    // Recursos OpenGL — malla
    GLuint  m_VAO = 0, m_VBO = 0, m_EBO = 0, m_shader = 0;
    GLsizei m_numIndices = 0;

    // Recursos OpenGL — trail / flechas (shader plano)
    GLuint  m_flatShader    = 0;
    GLuint  m_trailVAO      = 0;
    GLuint  m_trailVBO      = 0;
    GLsizei m_trailCount    = 0;
    std::vector<glm::vec3> m_trailPoints;

    GLuint  m_arrowVAO      = 0;
    GLuint  m_arrowVBO      = 0;
    GLsizei m_arrowCount    = 0;

    bool    m_initialized = false;

    // Construcción
    void buildGrid(int w, int h, float spacing);
    void buildSprings();
    void buildBuffers();

    // Física
    void computeForces(float dt);
    void integrate(float dt);
    void updateVBO();

    // Trail / flechas
    void updateTrail();
    void updateArrows();
    void buildFlatShaders();
    void buildTrailBuffers();
    void buildArrowBuffers();
};

#endif // CLOTH_H
