# glp

Graphics Layer Portable? I don't know what does it abbreviate.

3D rendering scene-focused framework implementing OpenGL (and Vulkan in the future) for portability.
Checkout [archived branch](https://github.com/damemay/glp/tree/archive-vitagl-compat) for old version that's compatible with vitaGL.

## current features
- SDL2 windowing and event handling
- OpenGL 4.1 rendering backend
- GLTF mesh and material loader 
- Material-based dynamic shader generation

## features to reimplement from archived branch
- Skeletons, bones and animations implementation
- 2D text rendering interface
- Customizing materials and saving data to custom format with zstd compression
- Bullet3 integration

## planned new features
- Change shader generation to be scene-based, not material-based
- UBOs and framebuffers
- Deferred rendering
- IBL
- Instancing
- Vulkan implementation and option to choose rendering backend on instantiation and building

## example usage
Checkout [main.cc](main.cc) for a sample that builds with the library. It's used internally for testing new features.

```c++
int main(int argc, char* argv[]) {
    glp::GLP sample {"sample", 1280, 720};

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

## building

```
mkdir build && cd build
cmake .. && make
```

### cmake options
| option       | description                                          | default |
|--------------|------------------------------------------------------|---------|
| `GLP_SAMPLE` | Build sample                                         | ON      |
| `GLP_DEBUG`  | Build with debug options (ASAN, OpenGL, GLP logging) | OFF     |
