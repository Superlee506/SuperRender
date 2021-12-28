/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include <nori/core/object.h>
#include <iomanip>

#if defined(PLATFORM_LINUX)
#include <malloc.h>
#endif

#if defined(PLATFORM_WINDOWS)
#include <Windows.h>
#endif

#if defined(PLATFORM_MACOS)
#include <sys/sysctl.h>
#endif

#if defined(__clang__)
#  define NANOGUI_FALLTHROUGH [[clang::fallthrough]];
#elif defined(__GNUG__)
#  define NANOGUI_FALLTHROUGH __attribute__ ((fallthrough));
#else
#  define NANOGUI_FALLTHROUGH
#endif

NORI_NAMESPACE_BEGIN

const std::string getAssetPath()
{
#if defined(SUPER_RENDER_DATA)
    return SUPER_RENDER_DATA;
#else
    return "./";
#endif
}

std::string utf8(uint32_t c) {
    char seq[8];
    int n = 0;
    if (c < 0x80) n = 1;
    else if (c < 0x800) n = 2;
    else if (c < 0x10000) n = 3;
    else if (c < 0x200000) n = 4;
    else if (c < 0x4000000) n = 5;
    else if (c <= 0x7fffffff) n = 6;
    seq[n] = '\0';
    switch (n) {
        case 6: seq[5] = 0x80 | (c & 0x3f); c = c >> 6; c |= 0x4000000; NANOGUI_FALLTHROUGH
        case 5: seq[4] = 0x80 | (c & 0x3f); c = c >> 6; c |= 0x200000;  NANOGUI_FALLTHROUGH
        case 4: seq[3] = 0x80 | (c & 0x3f); c = c >> 6; c |= 0x10000;   NANOGUI_FALLTHROUGH
        case 3: seq[2] = 0x80 | (c & 0x3f); c = c >> 6; c |= 0x800;     NANOGUI_FALLTHROUGH
        case 2: seq[1] = 0x80 | (c & 0x3f); c = c >> 6; c |= 0xc0;      NANOGUI_FALLTHROUGH
        case 1: seq[0] = c;
    }
    return std::string(seq, seq + n);
}

std::string indent(const std::string &string, int amount) {
    /* This could probably be done faster (it's not
       really speed-critical though) */
    std::istringstream iss(string);
    std::ostringstream oss;
    std::string spacer(amount, ' ');
    bool firstLine = true;
    for (std::string line; std::getline(iss, line); ) {
        if (!firstLine)
            oss << spacer;
        oss << line;
        if (!iss.eof())
            oss << endl;
        firstLine = false;
    }
    return oss.str();
}

bool endsWith(const std::string &value, const std::string &ending) {
    if (ending.size() > value.size())
        return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

std::string toLower(const std::string &value) {
    std::string result;
    result.resize(value.size());
    std::transform(value.begin(), value.end(), result.begin(), ::tolower);
    return result;
}

bool toBool(const std::string &str) {
    std::string value = toLower(str);
    if (value == "false")
        return false;
    else if (value == "true")
        return true;
    else
        throw NoriException("Could not parse boolean value \"%s\"", str);
}

int toInt(const std::string &str) {
    char *end_ptr = nullptr;
    int result = (int) strtol(str.c_str(), &end_ptr, 10);
    if (*end_ptr != '\0')
        throw NoriException("Could not parse integer value \"%s\"", str);
    return result;
}

unsigned int toUInt(const std::string &str) {
    char *end_ptr = nullptr;
    unsigned int result = (int) strtoul(str.c_str(), &end_ptr, 10);
    if (*end_ptr != '\0')
        throw NoriException("Could not parse integer value \"%s\"", str);
    return result;
}

float toFloat(const std::string &str) {
    char *end_ptr = nullptr;
    float result = (float) strtof(str.c_str(), &end_ptr);
    if (*end_ptr != '\0')
        throw NoriException("Could not parse floating point value \"%s\"", str);
    return result;
}

Eigen::Vector3f toVector3f(const std::string &str) {
    std::vector<std::string> tokens = tokenize(str);
    if (tokens.size() != 3)
        throw NoriException("Expected 3 values");
    Eigen::Vector3f result;
    for (int i=0; i<3; ++i)
        result[i] = toFloat(tokens[i]);
    return result;
}

std::vector<std::string> tokenize(const std::string &string, const std::string &delim, bool includeEmpty) {
    std::string::size_type lastPos = 0, pos = string.find_first_of(delim, lastPos);
    std::vector<std::string> tokens;

    while (lastPos != std::string::npos) {
        if (pos != lastPos || includeEmpty)
            tokens.push_back(string.substr(lastPos, pos - lastPos));
        lastPos = pos;
        if (lastPos != std::string::npos) {
            lastPos += 1;
            pos = string.find_first_of(delim, lastPos);
        }
    }

    return tokens;
}

std::string timeString(double time, bool precise) {
    if (std::isnan(time) || std::isinf(time))
        return "inf";

    std::string suffix = "ms";
    if (time > 1000) {
        time /= 1000; suffix = "s";
        if (time > 60) {
            time /= 60; suffix = "m";
            if (time > 60) {
                time /= 60; suffix = "h";
                if (time > 12) {
                    time /= 12; suffix = "d";
                }
            }
        }
    }

    std::ostringstream os;
    os << std::setprecision(precise ? 4 : 1)
       << std::fixed << time << suffix;

    return os.str();
}

std::string memString(size_t size, bool precise) {
    double value = (double) size;
    const char *suffixes[] = {
        "B", "KiB", "MiB", "GiB", "TiB", "PiB"
    };
    int suffix = 0;
    while (suffix < 5 && value > 1024.0f) {
        value /= 1024.0f; ++suffix;
    }

    std::ostringstream os;
    os << std::setprecision(suffix == 0 ? 0 : (precise ? 4 : 1))
       << std::fixed << value << " " << suffixes[suffix];

    return os.str();
}

float gammaCorrect(float value, float invGamma)
{
    return invGamma == 1.0f ? value : std::pow(value, invGamma);
}

float fresnelDielectric(float cosThetaI, float eta, float invEta, float & cosThetaT)
{
    if (eta == 1.0f)
    {
        cosThetaT = -cosThetaI;
        return 0.0f;
    }

    /* Using Snell's law, calculate the squared sine of the
    angle between the normal and the transmitted ray */
    float scale = (cosThetaI > 0.0f) ? invEta : eta;
    float cosThetaTSqr = 1.0f - (1.0f - cosThetaI * cosThetaI) * (scale * scale);

    /* Check for total internal reflection */
    if (cosThetaTSqr <= 0.0f)
    {
        cosThetaT = 0.0f;
        return 1.0f;
    }

    /* Find the absolute cosines of the incident/transmitted rays */
    float cosThetaIi = std::abs(cosThetaI);
    float cosThetaTt = std::sqrt(cosThetaTSqr);

    float rs = (cosThetaIi - eta * cosThetaTt)
               / (cosThetaIi + eta * cosThetaTt);
    float rp = (eta * cosThetaIi - cosThetaTt)
               / (eta * cosThetaIi + cosThetaTt);

    cosThetaT = (cosThetaI > 0.0f) ? -cosThetaTt : cosThetaTt;

    /* No polarization -- return the unpolarized reflectance */
    return 0.5f * (rs * rs + rp * rp);
}

Color3f fresnelConductor(float cosThetaI, const Color3f & eta, const Color3f & etaK)
{
    float cosThetaI2 = cosThetaI * cosThetaI;
    float sinThetaI2 = 1.0f - cosThetaI2;
    Color3f eta2 = eta * eta;
    Color3f etaK2 = etaK * etaK;

    Color3f t0 = eta2 - etaK2 - Color3f(sinThetaI2);
    Color3f a2PlusB2 = (t0 * t0 + 4.0f * eta2 * etaK2).cwiseSqrt();
    Color3f t1 = a2PlusB2 + Color3f(cosThetaI2);
    Color3f A = (0.5f * (a2PlusB2 + t0)).cwiseSqrt();
    Color3f t2 = 2.0f * cosThetaI * A;
    Color3f rs = (t1 - t2).cwiseQuotient(t1 + t2);

    Color3f t3 = cosThetaI2 * a2PlusB2 + Color3f(sinThetaI2 * sinThetaI2);
    Color3f t4 = t2 * sinThetaI2;
    Color3f rp = rs * (t3 - t4).cwiseQuotient(t3 + t4);

    return 0.5f * (rp + rs);
}

float approxFresnelDiffuseReflectance(float eta)
{
    /**
    * An evalution of the accuracy led
    * to the following scheme, which cherry-picks
    * fits from two papers where they are best.
    */
    if (eta < 1.0f)
    {
        /* Fit by Egan and Hilgeman (1973). Works
        reasonably well for "normal" IOR values (<2).
        Max rel. error in 1.0 - 1.5 : 0.1%
        Max rel. error in 1.5 - 2   : 0.6%
        Max rel. error in 2.0 - 5   : 9.5%
        */
        return -1.4399f * (eta * eta) + 0.7099f * eta + 0.6681f + 0.0636f / eta;
    }
    else
    {
        /* Fit by d'Eon and Irving (2011)
        *
        * Maintains a good accuracy even for
        * unrealistic IOR values.
        *
        * Max rel. error in 1.0 - 2.0   : 0.1%
        * Max rel. error in 2.0 - 10.0  : 0.2%
        */
        float invEta = 1.0f / eta,
                invEta2 = invEta * invEta,
                invEta3 = invEta2 * invEta,
                invEta4 = invEta3 * invEta,
                invEta5 = invEta4 * invEta;

        return 0.919317f - 3.4793f * invEta
               + 6.75335f * invEta2
               - 7.80989f * invEta3
               + 4.98554f * invEta4
               - 1.36881f * invEta5;
    }
}

void coordinateSystem(const Vector3f & va, Vector3f & vb, Vector3f & vc)
{
    if (std::abs(va.x()) > std::abs(va.y()))
    {
        float invLen = 1.0f / std::sqrt(va.x() * va.x() + va.z() * va.z());
        vc = Vector3f(va.z() * invLen, 0.0f, -va.x() * invLen);
    }
    else
    {
        float invLen = 1.0f / std::sqrt(va.y() * va.y() + va.z() * va.z());
        vc = Vector3f(0.0f, va.z() * invLen, -va.y() * invLen);
    }
    vb = vc.cross(va);
}

Vector3f reflect(const Vector3f & wi)
{
    return Vector3f(-wi.x(), -wi.y(), wi.z());
}

Vector3f refract(const Vector3f & wi, float cosThetaT, float eta, float invEta)
{
    float scale = -(cosThetaT < 0.0f ? invEta : eta);
    return Vector3f(scale * wi.x(), scale * wi.y(), cosThetaT);
}

Vector3f reflect(const Vector3f & wi, const Vector3f & m)
{
    return 2.0f * wi.dot(m) * m - wi;
}

Vector3f refract(const Vector3f & wi, const Vector3f & m, float cosThetaT, float eta, float invEta)
{
    eta = (cosThetaT < 0.0f ? invEta : eta);
    return m * (wi.dot(m) * eta + cosThetaT) - wi * eta;
}

filesystem::resolver *getFileResolver() {
    static filesystem::resolver *resolver = new filesystem::resolver();
    return resolver;
}

Color3f Color3f::toSRGB() const {
    Color3f result;

    for (int i=0; i<3; ++i) {
        float value = coeff(i);

        if (value <= 0.0031308f)
            result[i] = 12.92f * value;
        else
            result[i] = (1.0f + 0.055f)
                * std::pow(value, 1.0f/2.4f) -  0.055f;
    }

    return result;
}

Color3f Color3f::toLinearRGB() const {
    Color3f result;

    for (int i=0; i<3; ++i) {
        float value = coeff(i);

        if (value <= 0.04045f)
            result[i] = value * (1.0f / 12.92f);
        else
            result[i] = std::pow((value + 0.055f)
                * (1.0f / 1.055f), 2.4f);
    }

    return result;
}

bool Color3f::isValid() const {
    for (int i=0; i<3; ++i) {
        float value = coeff(i);
        if (value < 0 || !std::isfinite(value))
            return false;
    }
    return true;
}

float Color3f::getLuminance() const {
    return coeff(0) * 0.212671f + coeff(1) * 0.715160f + coeff(2) * 0.072169f;
}

Transform::Transform(const Eigen::Matrix4f &trafo)
    : m_transform(trafo), m_inverse(trafo.inverse()) { }

std::string Transform::toString() const {
    std::ostringstream oss;
    oss << m_transform.format(Eigen::IOFormat(4, 0, ", ", ";\n", "", "", "[", "]"));
    return oss.str();
}

Transform Transform::operator*(const Transform &t) const {
    return Transform(m_transform * t.m_transform,
        t.m_inverse * m_inverse);
}

Vector3f sphericalDirection(float theta, float phi) {
    float sinTheta, cosTheta, sinPhi, cosPhi;

    sincosf(theta, &sinTheta, &cosTheta);
    sincosf(phi, &sinPhi, &cosPhi);

    return Vector3f(
        sinTheta * cosPhi,
        sinTheta * sinPhi,
        cosTheta
    );
}

Point2f sphericalCoordinates(const Vector3f &v) {
    Point2f result(
        std::acos(v.z()),
        std::atan2(v.y(), v.x())
    );
    if (result.y() < 0)
        result.y() += 2*M_PI;
    return result;
}

float fresnel(float cosThetaI, float extIOR, float intIOR) {
    float etaI = extIOR, etaT = intIOR;

    if (extIOR == intIOR)
        return 0.0f;

    /* Swap the indices of refraction if the interaction starts
       at the inside of the object */
    if (cosThetaI < 0.0f) {
        std::swap(etaI, etaT);
        cosThetaI = -cosThetaI;
    }

    /* Using Snell's law, calculate the squared sine of the
       angle between the normal and the transmitted ray */
    float eta = etaI / etaT,
          sinThetaTSqr = eta*eta * (1-cosThetaI*cosThetaI);

    if (sinThetaTSqr > 1.0f)
        return 1.0f;  /* Total internal reflection! */

    float cosThetaT = std::sqrt(1.0f - sinThetaTSqr);

    float Rs = (etaI * cosThetaI - etaT * cosThetaT)
             / (etaI * cosThetaI + etaT * cosThetaT);
    float Rp = (etaT * cosThetaI - etaI * cosThetaT)
             / (etaT * cosThetaI + etaI * cosThetaT);

    return (Rs * Rs + Rp * Rp) / 2.0f;
}

NORI_NAMESPACE_END
