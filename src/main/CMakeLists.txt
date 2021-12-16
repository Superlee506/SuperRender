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
        ${STB_IMAGE_SRC}
        ${GUI_SRC}
        main.cpp
        )

if (WIN32)
  target_link_libraries(nori core tbb_static pugixml glog IlmImf zlibstatic glew_s glfw imgui_static)
else()
  target_link_libraries(nori core tbb_static pugixml glog IlmImf glew_s glfw imgui_static)
endif()

target_compile_features(nori PRIVATE cxx_std_17)