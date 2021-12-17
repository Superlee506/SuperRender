cmake_minimum_required (VERSION 3.2)
project(nori)

# static link glog library
add_definitions(-DGOOGLE_GLOG_DLL_DECL=)
if(MSVC)
  add_definitions(-DGLOG_NO_ABBREVIATED_SEVERITIES -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
endif()

# static link glew library
add_definitions(-DGLEW_STATIC)

add_subdirectory(ext ${PROJECT_BINARY_DIR}/ext_build)
set(ROOT_NORI_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include/nori)
set(ROOT_NORI_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/nori)

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
        # Nori include files
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

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

add_definitions(${NANOGUI_EXTRA_DEFS})

add_subdirectory(src/nori)
add_subdirectory(src/main)
add_subdirectory(src/test)
