file(GLOB CORE_SRC "${ROOT_NORI_SRC}/core/*.hpp" "${ROOT_NORI_SRC}/core/*.cpp")

file(GLOB CORE_HEADERS "${ROOT_NORI_INCLUDE}/core/*.hpp" "${ROOT_NORI_INCLUDE}/core/*.h")

file(GLOB ACCEL_SRC "${ROOT_NORI_SRC}/acceleration/*.hpp" "${ROOT_NORI_SRC}/acceleration/*.cpp")

file(GLOB BSDF_SRC "${ROOT_NORI_SRC}/bsdf/*.hpp" "${ROOT_NORI_SRC}/bsdf/*.cpp")

file(GLOB CAMERA_SRC "${ROOT_NORI_SRC}/camera/*.hpp" "${ROOT_NORI_SRC}/camera/*.cpp")

file(GLOB EMITTER_SRC "${ROOT_NORI_SRC}/emitter/*.hpp" "${ROOT_NORI_SRC}/emitter/*.cpp")

file(GLOB FILTER_SRC "${ROOT_NORI_SRC}/filter/*.hpp" "${ROOT_NORI_SRC}/filter/*.cpp")

file(GLOB INTEGRATION_SRC "${ROOT_NORI_SRC}/integrator/*.hpp" "${ROOT_NORI_SRC}/integrator/*.cpp")

file(GLOB MESH_SRC "${ROOT_NORI_SRC}/mesh/*.hpp" "${ROOT_NORI_SRC}/mesh/*.cpp")

file(GLOB SAMPLER_SRC "${ROOT_NORI_SRC}/sampler/*.hpp" "${ROOT_NORI_SRC}/sampler/*.cpp")


file(GLOB TEXTURE_SRC "${ROOT_NORI_SRC}/texture/*.hpp" "${ROOT_NORI_SRC}/texture/*.cpp")

file(GLOB GUI_SRC "${ROOT_NORI_SRC}/gui/*.hpp" "${ROOT_NORI_SRC}/gui/*.cpp")

add_library(core STATIC ${CORE_SRC})

set(CompilerFlags CORE_HEADERS CORE_SRC ACCEL_SRC
        BSDF_SRC CAMERA_SRC EMITTER_SRC FILTER_SRC
        INTEGRATION_SRC MESH_SRC SAMPLER_SRC
        TEXTURE_SRC GUI_SRC)

foreach(CompilerFlag ${CompilerFlags})
  set(${CompilerFlag} "${${CompilerFlag}}" PARENT_SCOPE)
endforeach()