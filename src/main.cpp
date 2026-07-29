// =============================================================================
// Simulador de Dinámica de Telas — Sistema Masa-Resorte
// =============================================================================
//
//  m·a = mg + Σ(-k(||Δx||-ℓ₀)û) - cv + F_viento
//
// Integración: Euler Semi-Implícito
//   v(t+Δt) = v(t) + F(t)/m · Δt
//   x(t+Δt) = x(t) + v(t+Δt) · Δt
// =============================================================================

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <algorithm>
#include <cmath>

#include "constants.h"
#include "camera.h"
#include "cloth.h"
#include "text.h"

static void printHelp() {
    std::cout << "\n"
"  TELA MASA-RESORTE — Controles:\n"
"  [G] Gravedad   [S] Resortes   [D] Amortiguamiento   [W] Viento\n"
"  [+/-] Rigidez  [K/L] Amort.   [P] Pausa  [Space] Paso  [R] Reset\n"
"  [T] Trail  [F] Flechas   [Mouse] Rotar   [Scroll] Zoom   [ESC] Salir\n"
"  ∂²x\n"
"  m·── = mg + Σ(-k(||Δx||-ℓ₀)û) - cv + F_viento\n"
"  ∂t²\n"
<< std::endl;
}

int main() {
    printHelp();

    if (!glfwInit()) {
        std::cerr << "Error al inicializar GLFW." << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        WIN_W, WIN_H,
        "Tela Masa-Resorte — Simulador Interactivo",
        nullptr, nullptr);

    if (!window) {
        std::cerr << "Error al crear la ventana GLFW." << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress))) {
        std::cerr << "Error al inicializar GLAD." << std::endl;
        glfwTerminate();
        return -1;
    }

    glViewport(0, 0, WIN_W, WIN_H);
    glEnable(GL_DEPTH_TEST);

    ClothSimulator cloth;
    if (!cloth.init(GRID_W, GRID_H, SPACING)) {
        std::cerr << "Error al inicializar el simulador." << std::endl;
        glfwTerminate();
        return -1;
    }

    TextRenderer text;
    text.init(WIN_W, WIN_H);

    OrbitalCamera cam;
    cam.radius = CAM_RADIUS;

    glfwSetScrollCallback(window, [](GLFWwindow* w, double, double yoff) {
        auto* c = static_cast<OrbitalCamera*>(glfwGetWindowUserPointer(w));
        if (c)
            c->radius = std::max(3.0f,
                c->radius - static_cast<float>(yoff) * ZOOM_SPEED * c->radius);
    });
    glfwSetWindowUserPointer(window, &cam);

    glm::vec3 center(0.0f, -4.0f, 3.0f);
    double lastTime = glfwGetTime();
    int fbW = WIN_W, fbH = WIN_H;

    while (!glfwWindowShouldClose(window)) {
        double now  = glfwGetTime();
        float frameDt = static_cast<float>(now - lastTime);
        lastTime = now;

        // ---- Input ----
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
            cloth.setGravityEnabled(!cloth.gravityEnabled());
            while (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) glfwPollEvents();
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            cloth.setSpringsEnabled(!cloth.springsEnabled());
            while (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) glfwPollEvents();
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            cloth.setDampingEnabled(!cloth.dampingEnabled());
            while (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) glfwPollEvents();
        }
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            cloth.setWindEnabled(!cloth.windEnabled());
            while (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) glfwPollEvents();
        }

        if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
            cloth.setStiffness(cloth.stiffness() * PARAM_STEP);
            while (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS ||
                   glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) glfwPollEvents();
        }
        if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
            cloth.setStiffness(cloth.stiffness() / PARAM_STEP);
            while (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS ||
                   glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) glfwPollEvents();
        }

        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
            cloth.setDamping(cloth.damping() * PARAM_STEP);
            while (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) glfwPollEvents();
        }
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
            cloth.setDamping(cloth.damping() / PARAM_STEP);
            while (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) glfwPollEvents();
        }

        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
            cloth.setPaused(!cloth.paused());
            while (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) glfwPollEvents();
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            cloth.singleStep();
            while (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) glfwPollEvents();
        }
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            cloth.reset();
            while (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) glfwPollEvents();
        }

        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
            cloth.setShowTrail(!cloth.showTrail());
            while (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) glfwPollEvents();
        }
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
            cloth.setShowArrows(!cloth.showArrows());
            while (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) glfwPollEvents();
        }

        // Mouse
        int mouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
        double mx, my;
        glfwGetCursorPos(window, &mx, &my);
        if (mouseState == GLFW_PRESS) {
            if (!cam.dragging) {
                cam.dragging = true;
            } else {
                double dx = mx - cam.lastMX;
                double dy = my - cam.lastMY;
                cam.theta += static_cast<float>(dx) * CAM_SPEED;
                cam.phi = std::max(0.05f,
                    std::min(1.4f, cam.phi + static_cast<float>(dy) * CAM_SPEED));
            }
        } else {
            cam.dragging = false;
        }
        cam.lastMX = mx;
        cam.lastMY = my;

        // ---- Resize ----
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        if (w != fbW || h != fbH) {
            fbW = w; fbH = h;
            glViewport(0, 0, fbW, fbH);
        }

        // ---- Física ----
        cloth.update(frameDt, SUBSTEPS);

        // ---- Render ----
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float aspect = static_cast<float>(fbW) / static_cast<float>(fbH);
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cam.eye(center), center, glm::vec3(0.0f, 1.0f, 0.0f));

        cloth.render(view, proj);
        cloth.renderTrail(view, proj);
        cloth.renderArrows(view, proj);

        // ---- HUD (renderizado 2D) ----
        text.renderHUD(fbW, fbH,
                       cloth.gravityEnabled(),
                       cloth.springsEnabled(),
                       cloth.dampingEnabled(),
                       cloth.windEnabled(),
                       cloth.showTrail(),
                       cloth.showArrows(),
                       cloth.paused(),
                       cloth.stiffness(),
                       cloth.damping(),
                       cloth.particleMass());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cloth.cleanup();
    text.cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
