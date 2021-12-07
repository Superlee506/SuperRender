cmake_minimum_required (VERSION 3.2)
project(nori)

add_subdirectory(ext ext_build)
option(ENABLE_OCTREE "Enable Octree" OFF) # disabled by default
option(ENABLE_BVH "Enable BVH" ON) # disenable by default
if (ENABLE_BVH)
  message("-- Using BVH for raytracing")
  add_definitions("-DUSE_BVH")
elseif(ENABLE_OCTREE)
  message("-- Using Octree for raytracing")
  add_definitions("-DUSE_OCTREE")
else()
  message("-- Using brust force search for raytracing")
endif()

include_directories(
  # Nori include files
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  # tinyformat string formatting library
  ${TFM_INCLUDE_DIR}
  # Eigen linear algebra library
  SYSTEM ${EIGEN_INCLUDE_DIR}
  # OpenEXR high dynamic range bitmap library
  SYSTEM ${OPENEXR_INCLUDE_DIRS}
  # Intel Thread Building Blocks
  SYSTEM ${TBB_INCLUDE_DIR}
  # Pseudorandom number generator
  ${PCG32_INCLUDE_DIR}
  # PugiXML parser
  ${PUGIXML_INCLUDE_DIR}
  # Helper functions for statistical hypothesis tests
  ${HYPOTHESIS_INCLUDE_DIR}
  # GLFW library for OpenGL context creation
  SYSTEM ${GLFW_INCLUDE_DIR}
  # GLEW library for accessing OpenGL functions
  SYSTEM ${GLEW_INCLUDE_DIR}
  # NanoVG drawing library
  SYSTEM ${NANOVG_INCLUDE_DIR}
  # NanoGUI user interface library
  SYSTEM ${NANOGUI_INCLUDE_DIR}
  SYSTEM ${NANOGUI_EXTRA_INCS}
  # Portable filesystem API
  SYSTEM ${FILESYSTEM_INCLUDE_DIR}
  # STB Image Write
  SYSTEM ${STB_IMAGE_WRITE_INCLUDE_DIR}
)

# The following lines build the main executable. If you add a source
# code file to Nori, be sure to include it in this list.
add_executable(nori

  # Header files
        include/nori/core/bbox.h
        include/nori/texture/bitmap.h
        include/nori/core/block.h
        include/nori/bsdf/bsdf.h
        include/nori/acceleration/accel.h
        include/nori/camera/camera.h
        include/nori/core/color.h
        include/nori/core/common.h
        include/nori/core/dpdf.h
        include/nori/core/frame.h
        include/nori/integrator/integrator.h
        include/nori/emitter/emitter.h
        include/nori/core/mesh.h
        include/nori/core/object.h
        include/nori/core/parser.h
        include/nori/core/proplist.h
        include/nori/core/ray.h
        include/nori/filter/rfilter.h
        include/nori/sampler/sampler.h
        include/nori/core/scene.h
        include/nori/core/timer.h
        include/nori/core/transform.h
        include/nori/core/vector.h
        include/nori/core/warp.h
        include/nori/acceleration/bvh.h
        include/nori/acceleration/octree.h

  # Source code files
        src/texture/bitmap.cpp
        src/core/block.cpp
        src/core/chi2test.cpp
        src/core/common.cpp
        src/bsdf/diffuse.cpp
        src/core/gui.cpp
  src/main.cpp
        src/core/mesh.cpp
        src/mesh/obj.cpp
        src/core/object.cpp
        src/core/parser.cpp
        src/camera/perspective.cpp
        src/core/proplist.cpp
        src/filter/rfilter.cpp
        src/core/scene.cpp
        src/core/ttest.cpp
        src/core/warp.cpp
        src/bsdf/microfacet.cpp
        src/bsdf/mirror.cpp
        src/bsdf/dielectric.cpp
        src/sampler/independent.cpp
  src/integrator/normals.cpp
  src/integrator/simple.cpp
  src/acceleration/bvh.cpp
  src/acceleration/octree.cpp
  src/acceleration/accel.cpp
  )

add_definitions(${NANOGUI_EXTRA_DEFS})

# The following lines build the warping test application
add_executable(warptest
        include/nori/core/warp.h
        src/core/warp.cpp
        src/core/warptest.cpp
        src/bsdf/microfacet.cpp
        src/core/object.cpp
        src/core/proplist.cpp
        src/core/common.cpp
)

if (WIN32)
  target_link_libraries(nori tbb_static pugixml IlmImf nanogui ${NANOGUI_EXTRA_LIBS} zlibstatic)
else()
  target_link_libraries(nori tbb_static pugixml IlmImf nanogui ${NANOGUI_EXTRA_LIBS})
endif()

target_link_libraries(warptest tbb_static nanogui ${NANOGUI_EXTRA_LIBS})

# Force colored output for the ninja generator
if (CMAKE_GENERATOR STREQUAL "Ninja")
  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fcolor-diagnostics")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")
  elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdiagnostics-color=always")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
  endif()
endif()

target_compile_features(warptest PRIVATE cxx_std_17)
target_compile_features(nori PRIVATE cxx_std_17)

# vim: set et ts=2 sw=2 ft=cmake nospell:
