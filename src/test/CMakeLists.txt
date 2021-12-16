# The following lines build the warping test application

# The following lines build the warping test application
add_executable(superTest
        superGuiTest.cpp
        ${FILTER_SRC})

add_executable(warptest
        warptest.cpp
        chi2test.cpp
        ttest.cpp
        ${GUI_SRC})

target_link_libraries(superTest core tbb_static glog IlmImf glew_s glfw imgui_static)
target_link_libraries(warptest core tbb_static glog glew_s glfw imgui_static)

target_compile_features(superTest PRIVATE cxx_std_17)
target_compile_features(warptest PRIVATE cxx_std_17)