Thin OpenGL wrapper for [Assimp](https://github.com/assimp/assimp)
==============================

Use Assimp-GL-Wrapper to easily load whole scene graphs with animations, bones, rigged meshes, lights and cameras.
High level functions like drawObjectBegin() and drawAllObjects handles the VAO, VBO and vertex array setup for you. All available data in meshes like vertices, vertex indices, normals, multiple texture coord sets, tangents, bitangents and bone matrices are set up in the shader for you, when available. With all the boilerplate out of the way, programmers are able to focus on what matters; creating the actual shaders and effects.

In addition, Assimp-inspector (a gigant hack) is a tool that spits out graphviz dot graphs given 3D model files as input. The tree represents Assimp's scene/data graph. If you have issues/bugs with importing, use this tool to confirm that the file has all the required data, and that the scene graph makes sense.

Building
===============================
Assimp-GL-Wrapper requires [libassimp](https://github.com/assimp/assimp), libGL, libglew and libglfw to build. Additionally, libglfw has some extra dependencies on its own. The cmake setup finds the libraries via pkg-config. Maybe this will be made into a single library or header include later, but the code isn't mature enough yet.
Note, that animation_test.cpp is the only file depending on GLFW. Feel free to change the file to use whatever toolkit you need. Secondly, animation_test.cpp, png_loader.h and png_loader.cpp are the only files that depends on libpng. If you don't want LoadImagePNG() in animation_test.cpp, you can also edit out the libpng dependency.

LICENCE
==============================
3-clause BSD licence, same as Assimp.

