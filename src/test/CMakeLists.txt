# The following lines build the warping test application
add_executable(warptest
        warptest.cpp
        chi2test.cpp
        ${ROOT_NORI_SRC}/bsdf/microfacetBSDF.cpp
        ${GUI_SRC})

target_link_libraries(warptest core tbb_static glog nanogui ${NANOGUI_EXTRA_LIBS})

target_compile_features(warptest PRIVATE cxx_std_17)

