/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include <nori/warp.h>
#include <nori/vector.h>
#include <nori/frame.h>

NORI_NAMESPACE_BEGIN

Point2f Warp::squareToUniformSquare(const Point2f &sample) {
    return sample;
}

float Warp::squareToUniformSquarePdf(const Point2f &sample) {
    return ((sample.array() >= 0).all() && (sample.array() <= 1).all()) ? 1.0f : 0.0f;
}
float tent(float x) {
    return x < 0.5f ? sqrt(2.0f * x) - 1.0f : 1.0f - sqrt(2.0f - 2.0f * x);
}
Point2f Warp::squareToTent(const Point2f &sample) {
    return Point2f(tent(sample.x()), tent(sample.y()));
}
float tentPdf(float t) {
    return t >= -1 && t <= 1 ? 1 - abs(t) : 0;
}

float Warp::squareToTentPdf(const Point2f &p) {
    return tentPdf(p.x()) * tentPdf(p.y());
}

Point2f Warp::squareToUniformDisk(const Point2f &sample) {
    float radius = std::sqrt(sample.x());
    float angle = sample.y() * (float)M_PI * 2;
    return Point2f(radius * std::cos(angle), radius * std::sin(angle));
}

float Warp::squareToUniformDiskPdf(const Point2f &p) {
    return std::sqrt(p.x() * p.x() + p.y() * p.y()) <= 1.0f ? INV_PI : 0.0f;
}

Vector3f Warp::squareToUniformSphere(const Point2f &sample) {
    float z = 1 - 2 * sample.x();
    float r = std::sqrt(std::max(0.f, 1.f - z * z));
    float phi = 2 * M_PI * sample.y();
    return Vector3f (r * std::cos(phi), r * std::sin(phi), z);
}

float Warp::squareToUniformSpherePdf(const Vector3f &v) {
    return INV_FOURPI;
}

Vector3f Warp::squareToUniformHemisphere(const Point2f &sample) {
    // theta = acos(1 - x), x is uniform from [0, 1],
    // so (1 - x) is also uniform from [0, 1]
    float cosTheta = sample.x();
    float phi = sample.y() * M_PI * 2;
    float r = std::sqrt(std::max(0.f, 1.f - cosTheta * cosTheta));
    return Vector3f(r * std::cos(phi), r * std::sin(phi), cosTheta);
}

float Warp::squareToUniformHemispherePdf(const Vector3f &v) {
    return v.z() < 0 ? 0 : INV_TWOPI;
}

Vector3f Warp::squareToCosineHemisphere(const Point2f &sample) {
    // Project the uniform samples in the Disk onto sphere,
    // and we can get the cos weighted samples
    // https://www.pbr-book.org/3ed-2018/Monte_Carlo_Integration/2D_Sampling_with_Multidimensional_Transformations
    Point2f bottom = squareToUniformDisk(sample);
    float x = bottom.x();
    float y = bottom.y();
    return Vector3f(x, y, std::sqrt(std::max(0.f, 1.f - x * x - y * y)));
}

float Warp::squareToCosineHemispherePdf(const Vector3f &v) {
    return v.z() < 0 ? 0 : v.z() * INV_PI;
}

Vector3f Warp::squareToBeckmann(const Point2f &sample, float alpha) {
    float phi = M_PI * 2 * sample.x();
    float theta = std::atan(std::sqrt(-alpha * alpha * std::log(1 - sample.y())));
    float cosPhi = std::cos(phi);
    float sinPhi = std::sin(phi);
    float cosTheta = std::cos(theta);
    float sinTheta = std::sin(theta);
    float x = sinTheta * cosPhi;
    float y = sinTheta * sinPhi;
    float z = cosTheta;
    return Vector3f(x, y, z);
}

float Warp::squareToBeckmannPdf(const Vector3f &m, float alpha) {
    if (m.z() <= 0) {
        return 0;
    }
    float alpha2 = alpha * alpha;
    float cosTheta = m.z();
    float tanTheta2 = (m.x() * m.x() + m.y() * m.y()) / (cosTheta * cosTheta);
    float cosTheta3 = cosTheta * cosTheta * cosTheta;
    float azimuthal = INV_PI;
    float longitudinal = std::exp(-tanTheta2 / alpha2) / (alpha2 * cosTheta3);
    return azimuthal * longitudinal;
}

NORI_NAMESPACE_END
