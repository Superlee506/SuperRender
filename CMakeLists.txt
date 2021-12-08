cmake_minimum_required (VERSION 3.2)
project(nori)

add_subdirectory(ext ext_build)

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

add_subdirectory(src)
# The following lines build the main executable. If you add a source
# code file to Nori, be sure to include it in this list.
foreach(EXAMPLE ${CORE_HEADERS})
  message(${EXAMPLE})
endforeach(EXAMPLE)
add_executable(nori
        ${CORE_HEADERS}
  # Header files
        include/nori/filter/gaussianFilter.h
        include/nori/acceleration/bvh.h
        include/nori/acceleration/octree.h
        include/nori/filter/boxFilter.h
        include/nori/filter/gaussianFilter.h
        include/nori/filter/mitchellNetravaliFilter.h
        include/nori/filter/tentFilter.h

  # Source code files
        src/filter/gaussianFilter.cpp
        src/bsdf/diffuse.cpp
        src/main.cpp
        src/mesh/obj.cpp
        src/camera/perspective.cpp
        src/bsdf/microfacet.cpp
        src/bsdf/mirror.cpp
        src/bsdf/dielectric.cpp
        src/sampler/independent.cpp
        src/integrator/normal.cpp
  src/integrator/simple.cpp
  src/acceleration/bvh.cpp
  src/acceleration/octree.cpp
        src/filter/boxFilter.cpp
        src/filter/gaussianFilter.cpp
        src/filter/mitchellNetravaliFilter.cpp
        src/filter/tentFilter.cpp
  )

add_definitions(${NANOGUI_EXTRA_DEFS})
# The following lines build the warping test application
add_executable(warptest
        src/test/warptest.cpp
        src/bsdf/microfacet.cpp)

if (WIN32)
  target_link_libraries(nori core tbb_static pugixml IlmImf nanogui ${NANOGUI_EXTRA_LIBS} zlibstatic)
else()
  target_link_libraries(nori core tbb_static pugixml IlmImf nanogui ${NANOGUI_EXTRA_LIBS})
endif()

target_link_libraries(warptest core tbb_static nanogui ${NANOGUI_EXTRA_LIBS})
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
