/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include <nori/mesh/wavefrontOBJMesh.h>
#include <nori/core/timer.h>
#include <unordered_map>
#include <fstream>

NORI_NAMESPACE_BEGIN

WavefrontOBJ::WavefrontOBJ(const PropertyList &propList)
{
    typedef std::unordered_map<OBJVertex, uint32_t, OBJVertexHash> VertexMap;

    filesystem::path filename =
            getFileResolver()->resolve(propList.getString("filename"));

    std::ifstream is(filename.str());
    if (is.fail())
        throw NoriException("Unable to open OBJ file \"%s\"!", filename);
    Transform trafo = propList.getTransform("toWorld", Transform());

    LOG(INFO) << "Loading \"" << filename << "\" .. ";
    Timer timer;
    std::vector<Vector3f>   positions;
    std::vector<Vector2f>   texcoords;
    std::vector<Vector3f>   normals;
    std::vector<uint32_t>   indices;
    std::vector<OBJVertex>  vertices;
    VertexMap vertexMap;

    std::string line_str;
    while (std::getline(is, line_str)) {
        std::istringstream line(line_str);

        std::string prefix;
        line >> prefix;

        if (prefix == "v") {
            Point3f p;
            line >> p.x() >> p.y() >> p.z();
            p = trafo * p;
            m_bbox.expandBy(p);
            positions.push_back(p);
        } else if (prefix == "vt") {
            Point2f tc;
            line >> tc.x() >> tc.y();
            texcoords.push_back(tc);
        } else if (prefix == "vn") {
            Normal3f n;
            line >> n.x() >> n.y() >> n.z();
            normals.push_back((trafo * n).normalized());
        } else if (prefix == "f") {
            std::string v1, v2, v3, v4;
            line >> v1 >> v2 >> v3 >> v4;
            OBJVertex verts[6];
            int nVertices = 3;

            verts[0] = OBJVertex(v1);
            verts[1] = OBJVertex(v2);
            verts[2] = OBJVertex(v3);

            if (!v4.empty()) {
                /* This is a quad, split into two triangles */
                verts[3] = OBJVertex(v4);
                verts[4] = verts[0];
                verts[5] = verts[2];
                nVertices = 6;
            }
            /* Convert to an indexed vertex list */
            for (int i=0; i<nVertices; ++i) {
                const OBJVertex &v = verts[i];
                VertexMap::const_iterator it = vertexMap.find(v);
                if (it == vertexMap.end()) {
                    vertexMap[v] = (uint32_t) vertices.size();
                    indices.push_back((uint32_t) vertices.size());
                    vertices.push_back(v);
                } else {
                    indices.push_back(it->second);
                }
            }
        }
    }

    m_F.resize(3, indices.size()/3);
    memcpy(m_F.data(), indices.data(), sizeof(uint32_t)*indices.size());

    m_V.resize(3, vertices.size());
    for (uint32_t i=0; i<vertices.size(); ++i)
        m_V.col(i) = positions.at(vertices[i].p-1);

    if (!normals.empty()) {
        m_N.resize(3, vertices.size());
        for (uint32_t i=0; i<vertices.size(); ++i)
            m_N.col(i) = normals.at(vertices[i].n-1);
    }

    if (!texcoords.empty()) {
        m_UV.resize(2, vertices.size());
        for (uint32_t i=0; i<vertices.size(); ++i)
            m_UV.col(i) = texcoords.at(vertices[i].uv-1);
    }

    m_name = filename.str();
    LOG(INFO) << "done. (V=" << m_V.cols() << ", F=" << m_F.cols() << ", took "
         << timer.elapsedString() << " and "
         << memString(m_F.size() * sizeof(uint32_t) +
                      sizeof(float) * (m_V.size() + m_N.size() + m_UV.size()))
         << ")" << endl;
}


WavefrontOBJ::OBJVertex::OBJVertex() { }

WavefrontOBJ::OBJVertex::OBJVertex(const std::string &string)
{
    std::vector<std::string> tokens = tokenize(string, "/", true);

    if (tokens.size() < 1 || tokens.size() > 3)
        throw NoriException("Invalid vertex data: \"%s\"", string);

    p = toUInt(tokens[0]);

    if (tokens.size() >= 2 && !tokens[1].empty())
        uv = toUInt(tokens[1]);

    if (tokens.size() >= 3 && !tokens[2].empty())
        n = toUInt(tokens[2]);
}

bool WavefrontOBJ::OBJVertex::operator==(const OBJVertex &v) const
{
    return v.p == p && v.n == n && v.uv == uv;
}

std::size_t WavefrontOBJ::OBJVertexHash::operator()(const OBJVertex &v) const
{
    size_t hash = std::hash<uint32_t>()(v.p);
    hash = hash * 37 + std::hash<uint32_t>()(v.uv);
    hash = hash * 37 + std::hash<uint32_t>()(v.n);
    return hash;
}


NORI_REGISTER_CLASS(WavefrontOBJ, "obj");
NORI_NAMESPACE_END
