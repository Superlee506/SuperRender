//
// Created by superqqli on 2021/12/12.
//
#include <nori/core/intersection.h>
#include <nori/core/shape.h>

NORI_NAMESPACE_BEGIN

Intersection::Intersection() : mesh(nullptr) { }

Vector3f Intersection::toLocal(const Vector3f &d) const {
    return shFrame.toLocal(d);
}

Vector3f Intersection::toWorld(const Vector3f &d) const {
    return shFrame.toWorld(d);
}

std::string Intersection::toString() const
{
    if (pShape == nullptr)
    {
        return "Intersection[invalid]";
    }


    if (!bHasUVPartial)
    {
        return tfm::format(
                "Intersection[\n"
                "  p = %s,\n"
                "  t = %f,\n"
                "  uv = %s,\n"
                "  shadingFrame = %s,\n"
                "  geometricFrame = %s,\n"
                "  shape = %s\n"
                "]",
                p.toString(),
                t,
                uv.toString(),
                indent(shFrame.toString()),
                indent(geoFrame.toString()),
                pShape->toString()
        );
    }
    else
    {
        std::string Str1 = tfm::format(
                "Intersection[\n"
                "  p = %s,\n"
                "  t = %f,\n"
                "  uv = %s,\n"
                "  shadingFrame = %s,\n"
                "  geometricFrame = %s,\n"
                "  shape = %s,\n",
                p.toString(),
                t,
                uv.toString(),
                indent(shFrame.toString()),
                indent(geoFrame.toString()),
                pShape->toString()
        );

        std::string Str2 = tfm::format(
                "  dPdU = %s,\n",
                "  dPdV = %s,\n",
                "  dNdU = %s,\n",
                "  dNdV = %s,\n",
                "  dUdX = %f,\n",
                "  dUdY = %f,\n",
                "  dVdX = %f,\n",
                "  dVdY = %f\n",
                "]",
                dPdU.toString(),
                dPdV.toString(),
                dNdU.toString(),
                dNdV.toString(),
                dUdX,
                dUdY,
                dVdX,
                dVdY
        );

        return Str1 + Str2;
    }
}
NORI_NAMESPACE_END
