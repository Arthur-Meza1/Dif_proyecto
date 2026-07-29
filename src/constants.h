#ifndef CONSTANTS_H
#define CONSTANTS_H

// =============================================================================
// Constantes globales de la simulación
// =============================================================================

// Malla
inline constexpr int    GRID_W       = 20;
inline constexpr int    GRID_H       = 20;
inline constexpr float  SPACING      = 0.45f;

// Física
inline constexpr float  PARTICLE_MASS = 0.08f;
inline constexpr float  STIFFNESS     = 180.0f;
inline constexpr float  DAMPING       = 2.0f;
inline constexpr float  GRAVITY_VAL   = -9.81f;

// Integración
inline constexpr int    SUBSTEPS      = 10;
inline constexpr float  MAX_FRAME_DT  = 0.033f;

// Viento
inline constexpr float  WIND_STRENGTH = 2.5f;
inline constexpr float  WIND_FREQ     = 0.6f;

// Ventana / cámara
inline constexpr int    WIN_W        = 1280;
inline constexpr int    WIN_H        = 720;
inline constexpr float  CAM_RADIUS   = 14.0f;
inline constexpr float  CAM_SPEED    = 0.005f;
inline constexpr float  ZOOM_SPEED   = 0.1f;

// Trayectoria (trail)
inline constexpr int    TRAIL_LENGTH = 300;
inline constexpr int    TRAIL_PARTICLE_ROW = 3;   // fila desde abajo para la trayectoria
inline constexpr int    TRAIL_PARTICLE_COL = 2;   // columna desde el centro

// Parámetros ajustables
inline constexpr float  STIFFNESS_MIN = 10.0f;
inline constexpr float  STIFFNESS_MAX = 500.0f;
inline constexpr float  DAMPING_MIN   = 0.0f;
inline constexpr float  DAMPING_MAX   = 20.0f;
inline constexpr float  MASS_MIN      = 0.01f;
inline constexpr float  MASS_MAX      = 1.0f;
inline constexpr float  PARAM_STEP    = 1.05f;

// =============================================================================
// Shaders (cadenas GLSL incrustadas)
// =============================================================================

// Shader principal de la malla
inline const char* const VERTEX_SHADER_SRC = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 uMVP;
uniform float uHeightRange;
uniform float uMinY;

out float vHeight;

void main() {
    gl_Position = uMVP * vec4(aPos, 1.0);
    vHeight = (aPos.y - uMinY) / max(uHeightRange, 0.001);
}
)";

inline const char* const FRAGMENT_SHADER_SRC = R"(
#version 330 core
in float vHeight;
out vec4 FragColor;

void main() {
    vec3 col1 = vec3(0.12, 0.34, 0.76);
    vec3 col2 = vec3(0.20, 0.70, 0.35);
    vec3 col3 = vec3(0.95, 0.75, 0.15);
    vec3 color = mix(col1, col2, smoothstep(0.0, 0.5, vHeight));
    color      = mix(color, col3, smoothstep(0.5, 1.0, vHeight));
    FragColor = vec4(color, 1.0);
}
)";

// Shader para la trayectoria (trail) y flechas — un solo color
inline const char* const FLAT_VERTEX_SRC = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 uMVP;
void main() {
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)";

inline const char* const FLAT_FRAGMENT_SRC = R"(
#version 330 core
uniform vec4 uColor;
out vec4 FragColor;
void main() {
    FragColor = uColor;
}
)";

// Shader para texto (texture atlas)
inline const char* const TEXT_VERTEX_SRC = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;
uniform mat4 uOrtho;
out vec2 vUV;
void main() {
    gl_Position = uOrtho * vec4(aPos, 0.0, 1.0);
    vUV = aUV;
}
)";

inline const char* const TEXT_FRAGMENT_SRC = R"(
#version 330 core
in vec2 vUV;
uniform sampler2D uTex;
uniform vec4 uColor;
out vec4 FragColor;
void main() {
    float a = texture(uTex, vUV).r;
    if (a < 0.01) discard;
    FragColor = vec4(uColor.rgb, uColor.a * a);
}
)";

#endif // CONSTANTS_H
