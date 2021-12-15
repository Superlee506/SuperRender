cmake_minimum_required (VERSION 3.2)
project(nori)

# static link glog library
add_definitions(-DGOOGLE_GLOG_DLL_DECL=)
if(MSVC)
  add_definitions(-DGLOG_NO_ABBREVIATED_SEVERITIES -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
endif()

add_subdirectory(ext ${PROJECT_BINARY_DIR}/ext_build)
set(ROOT_NORI_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include/nori)
set(ROOT_NORI_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)

include_directories(
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
  ${GLOG_INCLUDE_DIR}
  # Helper functions for statistical hypothesis tests
  ${HYPOTHESIS_INCLUDE_DIR}
  # Portable filesystem API
  SYSTEM ${FILESYSTEM_INCLUDE_DIR}
  # STB Image Write
  SYSTEM ${STB_IMAGE_INCLUDE_DIR}
  # glfw
  SYSTEM ${GLFW_INCLUDE_DIR}
  # Nori include files
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_subdirectory(src)
# The following lines build the main executable. If you add a source
# code file to Nori, be sure to include it in this list.
add_executable(nori
  # Source code files
        ${ACCEL_SRC}
        ${BSDF_SRC}
        ${CAMERA_SRC}
        ${EMITTER_SRC}
        ${FILTER_SRC}
        ${INTEGRATION_SRC}
        ${MESH_SRC}
        ${SAMPLER_SRC}
        ${TEXTURE_SRC}
        src/main.cpp
  )

# The following lines build the warping test application
add_executable(warptest
        ${TEST_SRC})

# The following lines build the warping test application
add_executable(superTest src/superTest.cpp)

if (WIN32)
  target_link_libraries(nori core tbb_static pugixml glog IlmImf zlibstatic)
else()
  target_link_libraries(nori core tbb_static pugixml glog IlmImf)
endif()

target_link_libraries(warptest core tbb_static glog)
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
