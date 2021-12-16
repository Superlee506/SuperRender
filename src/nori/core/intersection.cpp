//
// Created by superqqli on 2021/12/12.
//
#include <nori/core/intersection.h>
#include <nori/core/primitiveShape.h>

NORI_NAMESPACE_BEGIN

Intersection::Intersection() : mesh(nullptr) { }

Vector3f Intersection::toLocal(const Vector3f &d) const {
    return shFrame.toLocal(d);
}

Vector3f Intersection::toWorld(const Vector3f &d) const {
    return shFrame.toWorld(d);
}

Ray3f Intersection::generateShadowRay(const Point3f & destPoint) const
{
    Ray3f shadowRay;
    shadowRay.o = p;
    shadowRay.d = destPoint - shadowRay.o;
    shadowRay.maxt = 1.0f - float(Epsilon);
    shadowRay.mint = float(Epsilon);
    shadowRay.update();
    return shadowRay;
}

void Intersection::computeScreenSpacePartial(const Ray3f & Ray)
{
    /* Compute the texture coordinates partials wrt.
	changes in the screen-space position. Based on PBRT 10.1.1 */

    if (!Ray.bHasDifferentials || !bHasUVPartial)
    {
        return;
    }

    if (dPdU.isZero() && dPdV.isZero())
    {
        dUdX = 0.0f;
        dUdY = 0.0f;
        dVdX = 0.0f;
        dVdY = 0.0f;
        return;
    }

    /* Compute a few projections onto the surface normal */
    float pP = geoFrame.n.dot(Vector3f(p));
    float pRxOrigin = geoFrame.n.dot(Vector3f(Ray.rxOrigin));
    float pRyOrigin = geoFrame.n.dot(Vector3f(Ray.ryOrigin));
    float pRxDirection = geoFrame.n.dot(Ray.rxDirection);
    float pRyDirection = geoFrame.n.dot(Ray.ryDirection);

    if (pRxDirection == 0.0 || pRyDirection == 0.0)
    {
        dUdX = 0.0f;
        dUdY = 0.0f;
        dVdX = 0.0f;
        dVdY = 0.0f;
        return;
    }

    /* Compute ray-plane intersections against the offset rays */
    float Tx = (pP - pRxOrigin) / pRxDirection;
    float Ty = (pP - pRyOrigin) / pRyDirection;

    /* Calculate the U and V partials by solving two out
    of a set of 3 equations in an overconstrained system */
    float AbsX = std::abs(geoFrame.n.x());
    float AbsY = std::abs(geoFrame.n.y());
    float AbsZ = std::abs(geoFrame.n.z());

    float A[2][2], Bx[2], By[2], X[2];
    int Axes[2];

    if (AbsX > AbsY && AbsX > AbsZ)
    {
        Axes[0] = 1; Axes[1] = 2;
    }
    else if (AbsY > AbsZ)
    {
        Axes[0] = 0; Axes[1] = 2;
    }
    else
    {
        Axes[0] = 0; Axes[1] = 1;
    }

    A[0][0] = dPdU[Axes[0]];
    A[0][1] = dPdV[Axes[0]];
    A[1][0] = dPdU[Axes[1]];
    A[1][1] = dPdV[Axes[1]];

    /* Auxilary intersection point of the adjacent rays */
    Point3f Px = Ray.rxOrigin + Ray.rxDirection * Tx;
    Point3f	Py = Ray.ryOrigin + Ray.ryDirection * Ty;

    Bx[0] = Px[Axes[0]] - p[Axes[0]];
    Bx[1] = Px[Axes[1]] - p[Axes[1]];
    By[0] = Py[Axes[0]] - p[Axes[0]];
    By[1] = Py[Axes[1]] - p[Axes[1]];

    if (solveLinearSystem2x2(A, Bx, X))
    {
        dUdX = X[0];
        dVdX = X[1];
    }
    else
    {
        dUdX = 1.0f;
        dVdX = 0.0f;
    }

    if (solveLinearSystem2x2(A, By, X))
    {
        dUdY = X[0];
        dVdY = X[1];
    }
    else
    {
        dUdY = 0.0f;
        dVdY = 1.0f;
    }
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
