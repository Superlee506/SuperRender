//
// Created by superqqli on 2021/12/12.
//
#include <nori/core/frame.h>

NORI_NAMESPACE_BEGIN
Frame::Frame() { }

Frame::Frame(const Normal3f & n, const Vector3f & dpdu)
{
    this->n = n;
    this->s = (dpdu - this->n * this->n.dot(dpdu)).normalized();
    this->t = this->n.cross(this->s);
}

Frame::Frame(const Vector3f &s, const Vector3f &t, const Normal3f &n)
        : s(s), t(t), n(n) { }

Frame::Frame(const Vector3f &x, const Vector3f &y, const Vector3f &z)
        : s(x), t(y), n(z) { }

Frame::Frame(const Vector3f &n) : n(n) {
    coordinateSystem(n, s, t);
}

/// Convert from world coordinates to local coordinates
Vector3f Frame::toLocal(const Vector3f &v) const {
    return Vector3f(
            v.dot(s), v.dot(t), v.dot(n)
    );
}

/// Convert from local coordinates to world coordinates
Vector3f Frame::toWorld(const Vector3f &v) const {
    return s * v.x() + t * v.y() + n * v.z();
}

float Frame::cosTheta(const Vector3f &v) {
    return v.z();
}

float Frame::sinTheta(const Vector3f &v) {
    float temp = sinTheta2(v);
    if (temp <= 0.0f)
        return 0.0f;
    return std::sqrt(temp);
}

float Frame::tanTheta(const Vector3f &v) {
    float temp = 1 - v.z()*v.z();
    if (temp <= 0.0f)
        return 0.0f;
    return std::sqrt(temp) / v.z();
}

float Frame::sinTheta2(const Vector3f &v) {
    return 1.0f - v.z() * v.z();
}

float Frame::sinPhi(const Vector3f &v) {
    float sinTheta = Frame::sinTheta(v);
    if (sinTheta == 0.0f)
        return 1.0f;
    return clamp(v.y() / sinTheta, -1.0f, 1.0f);
}

float Frame::cosPhi(const Vector3f &v) {
    float sinTheta = Frame::sinTheta(v);
    if (sinTheta == 0.0f)
        return 1.0f;
    return clamp(v.x() / sinTheta, -1.0f, 1.0f);
}

float Frame::sinPhi2(const Vector3f &v) {
    return clamp(v.y() * v.y() / sinTheta2(v), 0.0f, 1.0f);
}

float Frame::cosPhi2(const Vector3f &v) {
    return clamp(v.x() * v.x() / sinTheta2(v), 0.0f, 1.0f);
}

bool Frame::operator==(const Frame &frame) const {
    return frame.s == s && frame.t == t && frame.n == n;
}

bool Frame::operator!=(const Frame &frame) const {
    return !operator==(frame);
}

std::string Frame::toString() const {
    return tfm::format(
            "Frame[\n"
            "  s = %s,\n"
            "  t = %s,\n"
            "  n = %s\n"
            "]", s.toString(), t.toString(), n.toString());
}

NORI_NAMESPACE_END