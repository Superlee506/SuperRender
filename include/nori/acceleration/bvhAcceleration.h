/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob

    Nori is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3
    as published by the Free Software Foundation.

    Nori is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include <nori/core/accel.h>
#include <nori/core/mesh.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Acceleration data structure for ray intersection queries
 *
 * The current implementation is a default BVH implementation
 */
class BvhAccel: public Accel{
public:
    BvhAccel(const PropertyList& list);
    virtual ~BvhAccel();

    /// Build the acceleration data structure (currently a no-op)
    virtual void build() override;

    /**
     * \brief Intersect a ray against all triangles stored in the scene and
     * return detailed intersection information
     *
     * \param ray
     *    A 3-dimensional ray data structure with minimum/maximum extent
     *    information
     *
     * \param its
     *    A detailed intersection record, which will be filled by the
     *    intersection query
     *
     * \param shadowRay
     *    \c true if this is a shadow ray query, i.e. a query that only aims to
     *    find out whether the ray is blocked or not without returning detailed
     *    intersection information.
     *
     * \return \c true if an intersection was found
     */
    virtual bool rayIntersect(const Ray3f &ray, Intersection &its, bool shadowRay) const override;

    virtual std::string toString() const override;

private:
    /* Accel node in 32 bytes */
    struct BVHNode {
        union {
            struct {
                unsigned flag : 1;
                uint32_t size : 31;
                uint32_t start;
            } leaf;

            struct {
                unsigned flag : 1;
                uint32_t axis : 31;
                uint32_t rightChild;
            } inner;

            uint64_t data;
        };
        BoundingBox3f bbox;

        bool isLeaf() const {
            return leaf.flag == 1;
        }

        bool isInner() const {
            return leaf.flag == 0;
        }

        bool isUnused() const {
            return data == 0;
        }

        uint32_t start() const {
            return leaf.start;
        }

        uint32_t end() const {
            return leaf.start + leaf.size;
        }
    };
    struct Internals;
    std::vector<BVHNode> m_nodes;       ///< BVH nodes
    std::vector<uint32_t> m_indices;    ///< Index references by BVH nodes
};

NORI_NAMESPACE_END
