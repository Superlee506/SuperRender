set(CORE_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../include/nori/core)
file(GLOB CORE_SRC "*.cpp" "*.hpp" "*.h" "${CORE_INCLUDE_PATH}/*.h" "${CORE_INCLUDE_PATH}/*.cpp")

file(GLOB CORE_HEADERS "${CORE_INCLUDE_PATH}/*.hpp" "${CORE_INCLUDE_PATH}/*.h")


add_library(core STATIC ${CORE_SRC})
if(WIN32)
  target_link_libraries(core tbb_static pugixml IlmImf nanogui ${NANOGUI_EXTRA_LIBS} zlibstatic)
else(WIN32)
  target_link_libraries(core tbb_static pugixml IlmImf nanogui ${NANOGUI_EXTRA_LIBS})
endif(WIN32)
set(CompilerFlags CORE_INCLUDE_PATH CORE_HEADERS CORE_SRC )
foreach(CompilerFlag ${CompilerFlags})
  set(${CompilerFlag} "${${CompilerFlag}}" PARENT_SCOPE)
endforeach()