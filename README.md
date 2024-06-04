# GAL

GAL - Graphics/Game Abstraction Layer

3D rendering scene-focused framework implementing OpenGL for portability.

Check out [archived branch](https://github.com/damemay/glp/tree/archive-vitagl-compat) for old version (glp) that's compatible with vitaGL.

## Current features
- C++20 based
- Scene-focused API
- Accepting lambdas to extend internal loops
- SDL2 windowing and event handling
- OpenGL 4.1 rendering backend
- GLTF mesh and material loader 
- Dynamic PBR shader generation based on materials used within scene

## Features from glp not reimplemented yet
- Skeletons, skinning and animations interface
- 2D text rendering interface
- Customizing materials and saving data to custom format with zstd compression
- Bullet3 integration

## Planned new features
- UBOs and framebuffers
- Deferred rendering
- IBL
- Instancing
- Refactor shader, primitives and material storage for memory optimization
- Vulkan implementation and option to choose rendering backend on instantiation and building

## Example usage
See [main.cc](main.cc) for a sample that builds with the library. It's used internally for testing new features.

```c++
int main(int argc, char* argv[]) {
    gal::GAL sample {"sample", 1280, 720};

    auto scene = sample.add_scene("Sample");

    scene->add_camera("Main Camera", std::make_unique<glp::Basic_Camera>());
    scene->add_controller("Player", std::make_unique<glp::Basic_FPP_Controller>(25.0f, 0.1f));
    scene->add_renderable("Model", std::make_unique<glp::GLTF_Render_Model>("sample_model.gltf"));

    scene->set_camera("Main Camera");
    scene->set_controller("Player");
    sample.set_scene("Sample");

    sample.loop(nullptr);

    return EXIT_SUCCESS;
}
```

## Building

```
mkdir build && cd build
cmake .. && make
```

### CMake options

| option       | description                                          | default |
|--------------|------------------------------------------------------|---------|
| `GAL_SAMPLE` | Build sample                                         | ON      |
| `GAL_DEBUG`  | Build with debug options (ASAN, OpenGL, GAL logging) | OFF     |

### Dependecies

#### Provided in repo

- [glad](https://github.com/Dav1dde/glad) loader with OpenGL 4.1 and GL_KHR_debug extension configuration
- several [stb](https://github.com/nothings/stb.git) headers (notably [stb_image.h](https://github.com/nothings/stb/blob/master/stb_image.h))
- [tinygltf](https://github.com/syoyo/tinygltf/) - as a git submodule

#### Not provided

These are looked for by CMake
- [SDL2](https://github.com/libsdl-org/SDL.git)
- [glm](https://github.com/g-truc/glm.git)
- [Bullet3](https://github.com/bulletphysics/bullet3.git)
