# ImGui Helper for Panda3D C++

This project is to use [ImGui](https://github.com/ocornut/imgui) for [Panda3D](https://github.com/panda3d/panda3d) C++.

![panda3d-imgui-screenshot](https://user-images.githubusercontent.com/937305/52536220-0e9f1c00-2d9b-11e9-8f93-544e44e5a70f.png)

**NOTE**: There is some flickering bug (https://github.com/bluekyu/rpcpp_plugins/issues/18).


## Usage
Just copy source files in "panda3d_imgui" directory and write setup codes in your game or engine.


## Building Sample

1. Run cmake commands. With [vcpkg](https://github.com/Microsoft/vcpkg),
```cmake
cmake
  -G "Visual Studio 15 2017 Win64"
  -DCMAKE_INSTALL_PREFIX="[PATH_TO_INSTALL]"
  -DCMAKE_TOOLCHAIN_FILE="[PATH_TO_VCPKG]/vcpkg/scripts/buildsystems/vcpkg.cmake"
  -Dpanda3d_ROOT="[PATH_TO_PANDA3D]/panda3d"
```
   Without vcpkg, set `imgui_DIR` cache variable:
   ```cmake
   cmake
     -G "Visual Studio 15 2017 Win64"
     -DCMAKE_INSTALL_PREFIX="[PATH_TO_INSTALL]"
     -Dpanda3d_ROOT="[PATH_TO_PANDA3D]/panda3d"
     -Dimgui_DIR="[PATH_TO_IMGUI_CMAKE]
   ```

2. Build and install
```cmake
cmake --build . --config release --target install
```

3. Run sample file (`panda3d_imgui_sample`).

   (You may need to set up Panda3D and ImGui runtime path)



## License
See `LICENSE.md` file.
