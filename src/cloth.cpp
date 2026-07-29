#include "cloth.h"
#include "constants.h"
#include "shader.h"

#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>

// =============================================================================
// init
// =============================================================================
bool ClothSimulator::init(int gridW, int gridH, float spacing) {
    if (m_initialized) cleanup();

    m_gridW   = gridW;
    m_gridH   = gridH;
    m_spacing = spacing;
    m_simTime = 0.0f;

    m_stiffness    = STIFFNESS;
    m_damping      = DAMPING;
    m_particleMass = PARTICLE_MASS;
    m_trailIdx     = (gridH - 1 - TRAIL_PARTICLE_ROW) * gridW + (gridW / 2 + TRAIL_PARTICLE_COL);
    if (m_trailIdx < 0) m_trailIdx = 0;
    if (m_trailIdx >= gridW * gridH) m_trailIdx = gridW * gridH - 1;

    buildGrid(gridW, gridH, spacing);
    buildSprings();

    m_shader = createProgram(VERTEX_SHADER_SRC, FRAGMENT_SHADER_SRC);
    if (!m_shader) {
        std::cerr << "Fallo al crear el programa de shaders." << std::endl;
        return false;
    }

    buildFlatShaders();
    buildBuffers();
    buildTrailBuffers();
    buildArrowBuffers();

    m_initialized = true;
    return true;
}

// =============================================================================
// buildGrid
// =============================================================================
void ClothSimulator::buildGrid(int gridW, int gridH, float spacing) {
    m_particles.clear();
    m_particles.reserve(static_cast<std::size_t>(gridW) * static_cast<std::size_t>(gridH));

    float halfW = (gridW - 1) * spacing * 0.5f;
    float halfH = (gridH - 1) * spacing * 0.3f;

    for (int j = 0; j < gridH; ++j) {
        for (int i = 0; i < gridW; ++i) {
            Particle p;
            p.position.x = static_cast<float>(i) * spacing - halfW;
            p.position.y = 0.0f;
            p.position.z = static_cast<float>(j) * spacing * 0.6f - halfH;

            p.position.y = 0.2f * std::sin(i * 0.35f + j * 0.25f) +
                           0.1f * std::cos(i * 0.5f);

            p.velocity = glm::vec3(0.0f);
            p.inv_mass = (j == 0) ? 0.0f : (1.0f / m_particleMass);
            m_particles.push_back(p);
        }
    }
}

// =============================================================================
// buildSprings
// =============================================================================
void ClothSimulator::buildSprings() {
    m_springs.clear();

    auto addSpring = [&](int i1, int i2, float stiff) {
        Spring s;
        s.i1          = i1;
        s.i2          = i2;
        s.rest_length = glm::length(m_particles[i1].position -
                                    m_particles[i2].position);
        s.stiffness   = stiff;
        m_springs.push_back(s);
    };

    for (int j = 0; j < m_gridH; ++j) {
        for (int i = 0; i < m_gridW; ++i) {
            int idx = j * m_gridW + i;

            if (i < m_gridW - 1)
                addSpring(idx, j * m_gridW + (i + 1), m_stiffness);

            if (j < m_gridH - 1)
                addSpring(idx, (j + 1) * m_gridW + i, m_stiffness);

            if (i < m_gridW - 1 && j < m_gridH - 1)
                addSpring(idx, (j + 1) * m_gridW + (i + 1), m_stiffness * 0.5f);

            if (i > 0 && j < m_gridH - 1)
                addSpring(idx, (j + 1) * m_gridW + (i - 1), m_stiffness * 0.5f);
        }
    }
}

// =============================================================================
// buildBuffers
// =============================================================================
void ClothSimulator::buildBuffers() {
    std::size_t n = m_particles.size();

    std::vector<GLfloat> verts;
    verts.reserve(n * 3);
    for (const auto& p : m_particles) {
        verts.push_back(p.position.x);
        verts.push_back(p.position.y);
        verts.push_back(p.position.z);
    }

    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(n * 3 * sizeof(GLfloat)),
                 verts.data(),
                 GL_DYNAMIC_DRAW);

    std::vector<GLuint> indices;
    auto addLine = [&](int a, int b) {
        indices.push_back(static_cast<GLuint>(a));
        indices.push_back(static_cast<GLuint>(b));
    };

    for (int j = 0; j < m_gridH; ++j) {
        for (int i = 0; i < m_gridW; ++i) {
            int idx = j * m_gridW + i;
            if (i < m_gridW - 1)
                addLine(idx, j * m_gridW + (i + 1));
            if (j < m_gridH - 1)
                addLine(idx, (j + 1) * m_gridW + i);
        }
    }
    m_numIndices = static_cast<GLsizei>(indices.size());

    glGenBuffers(1, &m_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(indices.size() * sizeof(GLuint)),
                 indices.data(),
                 GL_STATIC_DRAW);

    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

// =============================================================================
// buildFlatShaders  —  shader simple para trail y flechas
// =============================================================================
void ClothSimulator::buildFlatShaders() {
    m_flatShader = createProgram(FLAT_VERTEX_SRC, FLAT_FRAGMENT_SRC);
    if (!m_flatShader)
        std::cerr << "Fallo al crear shader plano." << std::endl;
}

// =============================================================================
// buildTrailBuffers
// =============================================================================
void ClothSimulator::buildTrailBuffers() {
    glGenVertexArrays(1, &m_trailVAO);
    glGenBuffers(1, &m_trailVBO);
    glBindVertexArray(m_trailVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_trailVBO);
    // allocate max size
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(TRAIL_LENGTH * 3 * sizeof(GLfloat)),
                 nullptr,
                 GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

// =============================================================================
// buildArrowBuffers
// =============================================================================
void ClothSimulator::buildArrowBuffers() {
    glGenVertexArrays(1, &m_arrowVAO);
    glGenBuffers(1, &m_arrowVBO);
    glBindVertexArray(m_arrowVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_arrowVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(m_particles.size() * 6 * sizeof(GLfloat)),
                 nullptr,
                 GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

// =============================================================================
// computeForces
// =============================================================================
void ClothSimulator::computeForces(float dt) {
    std::size_t n = m_particles.size();
    std::vector<glm::vec3> forces(n, glm::vec3(0.0f));

    // 1. Gravedad
    if (m_gravOn) {
        glm::vec3 gravity(0.0f, GRAVITY_VAL, 0.0f);
        for (std::size_t i = 0; i < n; ++i)
            if (m_particles[i].inv_mass > 0.0f)
                forces[i] += gravity / m_particles[i].inv_mass;
    }

    // 2. Resortes
    if (m_springOn) {
        for (const auto& s : m_springs) {
            const glm::vec3& p1 = m_particles[s.i1].position;
            const glm::vec3& p2 = m_particles[s.i2].position;
            glm::vec3 delta = p2 - p1;
            float dist = glm::length(delta);
            if (dist < 1e-8f) continue;

            glm::vec3 dir = delta / dist;
            float displacement = dist - s.rest_length;
            glm::vec3 f = s.stiffness * displacement * dir;

            forces[s.i1] += f;
            forces[s.i2] -= f;
        }
    }

    // 3. Amortiguamiento
    if (m_dampOn) {
        for (std::size_t i = 0; i < n; ++i)
            if (m_particles[i].inv_mass > 0.0f)
                forces[i] -= m_damping * m_particles[i].velocity;
    }

    // 4. Viento
    if (m_windOn) {
        float windPhase = m_simTime * WIND_FREQ;
        glm::vec3 wind(
            WIND_STRENGTH * std::sin(windPhase),
            0.0f,
            WIND_STRENGTH * 0.5f * std::cos(windPhase * 0.7f + 1.2f)
        );
        for (std::size_t i = 0; i < n; ++i)
            if (m_particles[i].inv_mass > 0.0f)
                forces[i] += wind;
    }

    // 5. v ← v + (F/m)·Δt
    for (std::size_t i = 0; i < n; ++i)
        if (m_particles[i].inv_mass > 0.0f)
            m_particles[i].velocity += forces[i] * m_particles[i].inv_mass * dt;

    // Guardamos fuerzas para las flechas (solo si se muestran)
    if (m_showArrows)
        updateArrows();
}

// =============================================================================
// integrate
// =============================================================================
void ClothSimulator::integrate(float dt) {
    m_minY =  std::numeric_limits<float>::max();
    m_maxY = -std::numeric_limits<float>::max();

    for (auto& p : m_particles) {
        if (p.inv_mass > 0.0f) p.position += p.velocity * dt;
        if (p.position.y < m_minY) m_minY = p.position.y;
        if (p.position.y > m_maxY) m_maxY = p.position.y;
    }
    m_heightRange = m_maxY - m_minY;
    if (m_heightRange < 0.001f) m_heightRange = 0.001f;
}

// =============================================================================
// update
// =============================================================================
void ClothSimulator::update(float frameDt, int substeps) {
    if (m_paused || frameDt <= 0.0f) return;
    if (frameDt > MAX_FRAME_DT) frameDt = MAX_FRAME_DT;

    float subDt = frameDt / static_cast<float>(substeps);

    for (int step = 0; step < substeps; ++step) {
        m_simTime += subDt;
        computeForces(subDt);
        integrate(subDt);
    }

    updateVBO();
    if (m_showTrail) updateTrail();
}

// =============================================================================
// singleStep
// =============================================================================
void ClothSimulator::singleStep() {
    float dt = MAX_FRAME_DT / static_cast<float>(SUBSTEPS);
    m_simTime += dt;
    computeForces(dt);
    integrate(dt);
    updateVBO();
    if (m_showTrail) updateTrail();
}

// =============================================================================
// updateVBO
// =============================================================================
void ClothSimulator::updateVBO() {
    std::size_t n = m_particles.size();
    std::vector<GLfloat> verts;
    verts.reserve(n * 3);
    for (const auto& p : m_particles) {
        verts.push_back(p.position.x);
        verts.push_back(p.position.y);
        verts.push_back(p.position.z);
    }
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    static_cast<GLsizeiptr>(n * 3 * sizeof(GLfloat)),
                    verts.data());
}

// =============================================================================
// render
// =============================================================================
void ClothSimulator::render(const glm::mat4& view, const glm::mat4& proj) {
    if (!m_initialized) return;

    glUseProgram(m_shader);

    glm::mat4 mvp = proj * view;
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "uMVP"), 1,
                       GL_FALSE, glm::value_ptr(mvp));
    glUniform1f(glGetUniformLocation(m_shader, "uHeightRange"), m_heightRange);
    glUniform1f(glGetUniformLocation(m_shader, "uMinY"), m_minY);

    glBindVertexArray(m_VAO);
    glDrawElements(GL_LINES, m_numIndices, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

// =============================================================================
// renderTrail
// =============================================================================
void ClothSimulator::renderTrail(const glm::mat4& view, const glm::mat4& proj) {
    if (!m_initialized || !m_showTrail || m_trailCount < 2) return;

    glUseProgram(m_flatShader);
    glm::mat4 mvp = proj * view;
    glUniformMatrix4fv(glGetUniformLocation(m_flatShader, "uMVP"), 1,
                       GL_FALSE, glm::value_ptr(mvp));
    glUniform4f(glGetUniformLocation(m_flatShader, "uColor"), 1.0f, 0.3f, 0.1f, 1.0f);

    glLineWidth(2.0f);
    glBindVertexArray(m_trailVAO);
    glDrawArrays(GL_LINE_STRIP, 0, m_trailCount);
    glBindVertexArray(0);
    glLineWidth(1.0f);
}

// =============================================================================
// renderArrows
// =============================================================================
void ClothSimulator::renderArrows(const glm::mat4& view, const glm::mat4& proj) {
    if (!m_initialized || !m_showArrows || m_arrowCount < 2) return;

    glUseProgram(m_flatShader);
    glm::mat4 mvp = proj * view;
    glUniformMatrix4fv(glGetUniformLocation(m_flatShader, "uMVP"), 1,
                       GL_FALSE, glm::value_ptr(mvp));
    glUniform4f(glGetUniformLocation(m_flatShader, "uColor"), 0.1f, 0.9f, 0.2f, 1.0f);

    glBindVertexArray(m_arrowVAO);
    glDrawArrays(GL_LINES, 0, m_arrowCount);
    glBindVertexArray(0);
}

// =============================================================================
// updateTrail
// =============================================================================
void ClothSimulator::updateTrail() {
    if (m_trailIdx < 0 || m_trailIdx >= static_cast<int>(m_particles.size())) return;

    m_trailPoints.push_back(m_particles[m_trailIdx].position);
    if (static_cast<int>(m_trailPoints.size()) > TRAIL_LENGTH)
        m_trailPoints.erase(m_trailPoints.begin());

    m_trailCount = static_cast<GLsizei>(m_trailPoints.size());

    std::vector<GLfloat> data;
    data.reserve(static_cast<std::size_t>(m_trailCount) * 3);
    for (const auto& pt : m_trailPoints) {
        data.push_back(pt.x);
        data.push_back(pt.y);
        data.push_back(pt.z);
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_trailVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    static_cast<GLsizeiptr>(data.size() * sizeof(GLfloat)),
                    data.data());
}

// =============================================================================
// updateArrows  —  dibuja segmentos desde cada partícula en dirección de F
// =============================================================================
void ClothSimulator::updateArrows() {
    std::vector<GLfloat> data;
    float scale = 0.02f;

    for (const auto& p : m_particles) {
        if (p.inv_mass <= 0.0f) continue;
        // Usamos la velocidad como proxy de la fuerza neta (escala)
        glm::vec3 dir = p.velocity;
        float len = glm::length(dir);
        if (len < 0.1f) continue;

        glm::vec3 end = p.position + dir * scale;
        data.push_back(p.position.x);
        data.push_back(p.position.y);
        data.push_back(p.position.z);
        data.push_back(end.x);
        data.push_back(end.y);
        data.push_back(end.z);
    }

    m_arrowCount = static_cast<GLsizei>(data.size() / 3);

    if (!data.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, m_arrowVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
                        static_cast<GLsizeiptr>(data.size() * sizeof(GLfloat)),
                        data.data());
    }
}

// =============================================================================
// getCenter
// =============================================================================
glm::vec3 ClothSimulator::getCenter() const {
    if (m_particles.empty()) return glm::vec3(0.0f);
    glm::vec3 sum(0.0f);
    for (const auto& p : m_particles) sum += p.position;
    return sum / static_cast<float>(m_particles.size());
}

// =============================================================================
// setStiffness
// =============================================================================
void ClothSimulator::setStiffness(float k) {
    m_stiffness = std::max(STIFFNESS_MIN, std::min(STIFFNESS_MAX, k));
    // Reconstruir resortes con la nueva rigidez
    buildSprings();
}

// =============================================================================
// setDamping
// =============================================================================
void ClothSimulator::setDamping(float c) {
    m_damping = std::max(DAMPING_MIN, std::min(DAMPING_MAX, c));
}

// =============================================================================
// setParticleMass
// =============================================================================
void ClothSimulator::setParticleMass(float m) {
    m_particleMass = std::max(MASS_MIN, std::min(MASS_MAX, m));
    float inv = 1.0f / m_particleMass;
    for (auto& p : m_particles)
        if (p.inv_mass > 0.0f) p.inv_mass = inv;
}

// =============================================================================
// reset
// =============================================================================
void ClothSimulator::reset() {
    m_simTime = 0.0f;
    m_trailPoints.clear();
    m_trailCount = 0;
    buildGrid(m_gridW, m_gridH, m_spacing);
    buildSprings();
    updateVBO();
}

// =============================================================================
// cleanup
// =============================================================================
void ClothSimulator::cleanup() {
    if (m_shader)    glDeleteProgram(m_shader);
    if (m_flatShader) glDeleteProgram(m_flatShader);
    if (m_EBO)       glDeleteBuffers(1, &m_EBO);
    if (m_VBO)       glDeleteBuffers(1, &m_VBO);
    if (m_VAO)       glDeleteVertexArrays(1, &m_VAO);
    if (m_trailVBO)  glDeleteBuffers(1, &m_trailVBO);
    if (m_trailVAO)  glDeleteVertexArrays(1, &m_trailVAO);
    if (m_arrowVBO)  glDeleteBuffers(1, &m_arrowVBO);
    if (m_arrowVAO)  glDeleteVertexArrays(1, &m_arrowVAO);

    m_shader = m_flatShader = 0;
    m_EBO = m_VBO = m_VAO = 0;
    m_trailVAO = m_trailVBO = 0;
    m_arrowVAO = m_arrowVBO = 0;
    m_initialized = false;
    m_particles.clear();
    m_springs.clear();
    m_trailPoints.clear();
}
