//
// Created by superqqli on 2021/12/8.
//

#include <nori/core/common.h>
#include <nori/core/mesh.h>
#include <filesystem/resolver.h>
#include <unordered_map>
#include <fstream>

NORI_NAMESPACE_BEGIN

/**
 * \brief Loader for Wavefront OBJ triangle meshes
 */
class WavefrontOBJ : public Mesh {
public:
    WavefrontOBJ(const PropertyList &propList);

protected:
    /// Vertex indices used by the OBJ format
    struct OBJVertex {
        uint32_t p = (uint32_t) -1;
        uint32_t n = (uint32_t) -1;
        uint32_t uv = (uint32_t) -1;

        OBJVertex();

        OBJVertex(const std::string &string);

        bool operator==(const OBJVertex &v) const;
    };

    /// Hash function for OBJVertex
    struct OBJVertexHash {
        std::size_t operator()(const OBJVertex &v) const;
    };
};

NORI_NAMESPACE_END
