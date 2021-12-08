set(CORE_INCLUDE_PATH ../include/nori/core)
set(CORE_SRC_PATH core)

#[[set(CORE_INCLUDE_PATH ../include/nori/core)
set(CORE_SRC_PATH core)]]
file(GLOB CORE_SRC "${CORE_INCLUDE_PATH}/*.h" "${CORE_INCLUDE_PATH}/*.hpp" "${CORE_SRC_PATH}/*.cpp" "${CORE_SRC_PATH}/*.hpp")

file(GLOB CORE_HEADERS "${CORE_INCLUDE_PATH}/*.hpp" "${CORE_INCLUDE_PATH}/*.h")


add_library(core STATIC ${CORE_SRC})

set(CompilerFlags CORE_INCLUDE_PATH CORE_HEADERS CORE_SRC )
foreach(CompilerFlag ${CompilerFlags})
  set(${CompilerFlag} "${${CompilerFlag}}" PARENT_SCOPE)
endforeach()