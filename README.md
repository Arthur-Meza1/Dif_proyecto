# Simulador de Dinámica de Telas — Sistema Masa-Resorte

Simulador físico interactivo en tiempo real de una tela modelada como un sistema masa-resorte, implementado en C++17 con OpenGL 3.3. Desarrollado como demostración de aplicación de Ecuaciones Diferenciales Ordinarias (EDO) en gráficos por computadora.

## Características

- **Modelo masa-resorte** con malla de 20×20 partículas conectadas por resortes estructurales y de cortante.
- **Tres fuerzas concurrentes** modeladas en la EDO: gravedad, restitución elástica (Ley de Hooke) y amortiguamiento viscoso.
- **Integración numérica** con Euler Semi-Implícito (simpéctico) y sub-pasos por frame.
- **Interactividad completa** con teclado y mouse: toggle de fuerzas, ajuste de parámetros en vivo, pausa/paso, cámara orbital.
- **Visualizaciones adicionales**: trayectoria 3D de una partícula, flechas de velocidad, gradiente de color por altura.
- **HUD en pantalla** con indicadores de estado y comandos, renderizado con fuente bitmap 5×7.
- **Arquitectura modular** separada en componentes con responsabilidades únicas.

## Requisitos

| Dependencia | Versión | Propósito |
|-------------|---------|-----------|
| GLFW   | ≥ 3.3  | Ventana y contexto OpenGL |
| GLM    | ≥ 0.9  | Matemáticas de vectores/matrices |
| OpenGL | ≥ 3.3  | Renderizado (Core Profile) |
| GLAD   | 2.x    | Cargador de punteros OpenGL (incluido) |

### Instalación de dependencias

**Arch Linux / CachyOS:**
```bash
sudo pacman -S glfw glm
```

**Debian / Ubuntu:**
```bash
sudo apt install libglfw3-dev libglm-dev
```

**Fedora:**
```bash
sudo dnf install glfw-devel glm-devel
```

GLAD se incluye como `glad.c` en el proyecto. Si se prefiere usar el sistema o descargarlo vía FetchContent, el `CMakeLists.txt` lo maneja automáticamente.

## Compilación

```bash
git clone <repo> tela-masa-resorte
cd tela-masa-resorte
cmake -B build
cmake --build build
./build/cloth_sim
```

## Controles

### Teclado

| Tecla | Acción |
|-------|--------|
| `G` | Alterna gravedad |
| `S` | Alterna resortes (Ley de Hooke) |
| `D` | Alterna amortiguamiento |
| `W` | Alterna viento |
| `+` / `-` | Aumenta / reduce rigidez $k$ |
| `K` / `L` | Aumenta / reduce amortiguamiento $c$ |
| `P` | Pausa / reanuda |
| `Espacio` | Avanza un paso (en pausa) |
| `R` | Reinicia la simulación |
| `T` | Muestra/oculta trayectoria |
| `F` | Muestra/oculta flechas de fuerza |
| `Esc` | Salir |

### Mouse

| Acción | Efecto |
|--------|--------|
| Arrastrar con botón izquierdo | Rotar cámara orbital |
| Scroll | Acercar / alejar |

## Modelo físico

Cada partícula de masa $m$ sigue la EDO de segundo orden:

$$
m \frac{d^2\mathbf{x}}{dt^2} = m\mathbf{g} + \sum_{j \in \mathcal{N}(i)} \left[ -k \bigl( \lVert\Delta\mathbf{x}_{ij}\rVert - \ell_{ij} \bigr) \frac{\Delta\mathbf{x}_{ij}}{\lVert\Delta\mathbf{x}_{ij}\rVert} \right] - c \mathbf{v}_i + \mathbf{F}_{\text{viento}}
$$

| Término | Fuerza | Descripción |
|---------|--------|-------------|
| $m\mathbf{g}$ | Gravedad | Constante hacia abajo |
| $-k(\lVert\Delta\mathbf{x}\rVert - \ell_0)\hat{\mathbf{u}}$ | Restitución elástica | Ley de Hooke entre partículas adyacentes |
| $-c\mathbf{v}$ | Amortiguamiento viscoso | Disipa energía, evita oscilaciones infinitas |
| $\mathbf{F}_{\text{viento}}$ | Viento | Fuerza externa senoidal |

### Integración numérica

Se emplea el método de **Euler Semi-Implícito** (simpéctico):

$$
\begin{aligned}
\mathbf{v}(t + \Delta t) &= \mathbf{v}(t) + \frac{\mathbf{F}(t)}{m} \Delta t \\
\mathbf{x}(t + \Delta t) &= \mathbf{x}(t) + \mathbf{v}(t + \Delta t) \Delta t
\end{aligned}
$$

Cada frame se divide en 10 sub-pasos para mantener la estabilidad numérica. La fila superior de la malla permanece fija (masa infinita) para que la tela cuelgue.

## Arquitectura del proyecto

```
.
├── CMakeLists.txt          ← Sistema de compilación
├── glad.c                  ← Cargador GLAD (generado)
├── include/                ← Headers de GLAD
│   ├── glad/gl.h
│   └── KHR/khrplatform.h
├── src/
│   ├── main.cpp            ← Punto de entrada, bucle principal e input
│   ├── constants.h         ← Constantes globales y shaders GLSL
│   ├── particle.h          ← Estructuras Particle y Spring
│   ├── shader.h / .cpp     ← Compilación y enlace de shaders
│   ├── camera.h / .cpp     ← Cámara orbital (coordenadas esféricas)
│   ├── cloth.h / .cpp      ← Simulador: física, malla y renderizado
│   └── text.h / .cpp       ← HUD: renderizado de texto con bitmap font
└── README.md
```

## Formulación matemática detallada

Para una partícula $i$ conectada a sus vecinos $\mathcal{N}(i)$:

$$ m_i \ddot{\mathbf{x}}_i = m_i \mathbf{g} + \sum_{j \in \mathcal{N}(i)} \left[ -k_{ij} \bigl( \lVert \mathbf{x}_j - \mathbf{x}_i \rVert - \ell_{ij} \bigr) \frac{\mathbf{x}_j - \mathbf{x}_i}{\lVert \mathbf{x}_j - \mathbf{x}_i \rVert} \right] - c_i \dot{\mathbf{x}}_i + \mathbf{f}_{\text{viento}}(t) $$

Donde $\ell_{ij}$ es la longitud de reposo del resorte que conecta las partículas $i$ y $j$, calculada a partir de sus posiciones iniciales en la malla. Los resortes de cortante (diagonales) usan la mitad de rigidez que los estructurales.

## Rendimiento

La simulación corre a 60 FPS en hardware moderno con una malla de 20×20 partículas (400 partículas, ~1500 resortes, 10 sub-pasos por frame). El cuello de botella principal es la transferencia de datos VBO a la GPU.

## Licencia

Distribuido bajo licencia MIT. El código de la fuente bitmap 5×7 es de dominio público (derivado de la biblioteca Adafruit GFX).
