//
// Created by superqqli on 2021/12/28.
//
#include <nori/core/texture.h>
#include <nori/core/timer.h>
#include <nori/core/intersection.h>

#include <stb_image.h>
NORI_NAMESPACE_BEGIN

std::unique_ptr<float[]> loadImageFromFileR(
        const std::string & filename,
        float gamma,
        int & width,
        int & height,
        float * pAverage,
        float * pMaximum,
        float * pMinimum
)
{
    // Top left - (0,0)

    LOG(INFO) << "Loading texture (R) \"" << filename << "\" ... ";
    cout.flush();
    Timer texLoadTimer;

    int channelsInFile = -1, requestChannels = 1;
    unsigned char * pData = stbi_load(filename.c_str(), &width, &height, &channelsInFile, requestChannels);

    if (pData == nullptr)
    {
        const char * pFailReason = stbi_failure_reason();
        throw NoriException("Load texture \"%s\" failed. Reason : [%s]", filename.c_str(), pFailReason);
    }

    std::unique_ptr<float[]> pixels(new float[width * height]);

    constexpr float inv255 = 1.0f / 255.0f;
    const float invGamma = 1.0f / gamma;

    float maximum = std::numeric_limits<float>::min();
    float minimum = std::numeric_limits<float>::max();
    float average = 0.0f;

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            int loadPixelIdx = y * width + x;
            int storePixelIdx = (height - 1 - y) * width + x;

            pixels[storePixelIdx] = gammaCorrect(float(pData[loadPixelIdx * requestChannels + 0]) * inv255, invGamma);

            if (pixels[storePixelIdx] > maximum) { maximum = pixels[storePixelIdx]; }
            if (pixels[storePixelIdx] < minimum) { minimum = pixels[storePixelIdx]; }
            average += pixels[storePixelIdx];
        }
    }

    average /= (width * height);

    if (pMaximum != nullptr) { *pMaximum = maximum; }
    if (pMinimum != nullptr) { *pMinimum = minimum; }
    if (pAverage != nullptr) { *pAverage = average; }

    stbi_image_free(pData);

    LOG(INFO) << "Done. (Took " << texLoadTimer.elapsedString() << ")";

    return pixels;
}

std::unique_ptr<Color3f[]> loadImageFromFileRGB(
        const std::string & filename,
        float gamma,
        int & width,
        int & height,
        Color3f * pAverage,
        Color3f * pMaximum,
        Color3f * pMinimum
)
{
    // Top left - (0,0)

    LOG(INFO) << "Loading texture (RGB) \"" << filename << "\" ... ";
    cout.flush();
    Timer texLoadTimer;

    int channelsInFile = -1, requestChannels = 3;
    unsigned char * pData = stbi_load(filename.c_str(), &width, &height, &channelsInFile, requestChannels);

    if (pData == nullptr)
    {
        const char * pFailReason = stbi_failure_reason();
        throw NoriException("Load texture \"%s\" failed. Reason : [%s]", filename.c_str(), pFailReason);
    }

    std::unique_ptr<Color3f[]> pixels(new Color3f[width * height]);

    constexpr float inv255 = 1.0f / 255.0f;
    const float invGamma = 1.0f / gamma;

    Color3f maximum = Color3f(std::numeric_limits<float>::min());
    Color3f minimum = Color3f(std::numeric_limits<float>::max());
    Color3f average = Color3f(0.0f);

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            int loadPixelIdx = y * width + x;
            int storePixelIdx = (height - 1 - y) * width + x;

            pixels[storePixelIdx] = Color3f(
                    gammaCorrect(float(pData[loadPixelIdx * requestChannels + 0]) * inv255, invGamma),
                    gammaCorrect(float(pData[loadPixelIdx * requestChannels + 1]) * inv255, invGamma),
                    gammaCorrect(float(pData[loadPixelIdx * requestChannels + 2]) * inv255, invGamma)
            );

            if (pixels[storePixelIdx][0] > maximum[0]) { maximum[0] = pixels[storePixelIdx][0]; }
            if (pixels[storePixelIdx][1] > maximum[1]) { maximum[1] = pixels[storePixelIdx][1]; }
            if (pixels[storePixelIdx][2] > maximum[2]) { maximum[2] = pixels[storePixelIdx][2]; }
            if (pixels[storePixelIdx][0] < minimum[0]) { minimum[0] = pixels[storePixelIdx][0]; }
            if (pixels[storePixelIdx][1] < minimum[1]) { minimum[1] = pixels[storePixelIdx][1]; }
            if (pixels[storePixelIdx][2] < minimum[2]) { minimum[2] = pixels[storePixelIdx][2]; }
            average[0] += pixels[storePixelIdx][0];
            average[1] += pixels[storePixelIdx][1];
            average[2] += pixels[storePixelIdx][2];
        }
    }

    average /= float(width * height);

    if (pMaximum != nullptr) { *pMaximum = maximum; }
    if (pMinimum != nullptr) { *pMinimum = minimum; }
    if (pAverage != nullptr) { *pAverage = average; }

    stbi_image_free(pData);

    LOG(INFO) << "Done. (Took " << texLoadTimer.elapsedString() << ")";

    return pixels;
}

Color3f Texture::eval(const Intersection & isect, bool bFilter) const
{
    throw NoriException(
            "Texture::eval(const Intersection & Isect, bool bFilter) is not implemented!"
    );
}

void Texture::evalGradient(const Intersection & isect, Color3f * pGradients) const
{
    Intersection isectCopy(isect);

    Color3f value = eval(isect, false);

    isectCopy.p = isect.p + isect.dPdU * float(Epsilon);
    isectCopy.uv = isect.uv + Vector2f(float(Epsilon), 0.0f);
    Color3f valueU = eval(isectCopy, false);

    isectCopy.p = isect.p + isect.dPdV * float(Epsilon);
    isectCopy.uv = isect.uv + Vector2f(0.0f, float(Epsilon));
    Color3f valueV = eval(isectCopy, false);

    pGradients[0] = (valueU - value) / float(Epsilon);
    pGradients[1] = (valueV - value) / float(Epsilon);
}

Color3f Texture::getAverage() const
{
    throw NoriException("Texture::getAverage() is not implemented!");
}

Color3f Texture::getMinimum() const
{
    throw NoriException("Texture::getMinimum() is not implemented!");
}

Color3f Texture::getMaximum() const
{
    throw NoriException("Texture::getMaximum() is not implemented!");
}

Vector3i Texture::getDimension() const
{
    throw NoriException("Texture::getDimension() is not implemented!");
}

bool Texture::isConstant() const
{
    throw NoriException("Texture::isConstant() is not implemented!");
}

bool Texture::isMonochromatic() const
{
    throw NoriException("Texture::isMonochromatic() is not implemented!");
}

Texture * Texture::actualTexture()
{
    return this;
}

NoriObject::EClassType Texture::getClassType() const
{
    return EClassType::ETexture;
}

Color3f Texture2D::eval(const Intersection & isect, bool bFilter) const
{
    Point2f uv = Point2f(isect.uv.x() * m_uvScale.x(), isect.uv.y() * m_uvScale.y()) + m_uvOffset;

    if (bFilter)
    {
        return eval(uv,
                    Vector2f(isect.dUdX * m_uvScale.x(), isect.dVdX * m_uvScale.y()),
                    Vector2f(isect.dUdY * m_uvScale.x(), isect.dVdY * m_uvScale.y()));
    }
    else
    {
        return eval(uv);
    }
}

void Texture2D::evalGradient(const Intersection & isect, Color3f * pGradients) const
{
    evalGradient(isect.uv, pGradients);

    pGradients[0] *= m_uvScale.x();
    pGradients[1] *= m_uvScale.y();
}

void Texture2D::evalGradient(const Point2f & uv, Color3f * pGradients) const
{
    Color3f value = eval(uv);
    Color3f valueU = eval(uv + Vector2f(float(Epsilon), 0.0f));
    Color3f valueV = eval(uv + Vector2f(0.0f, float(Epsilon)));

    pGradients[0] = (valueU - value) / float(Epsilon);
    pGradients[1] = (valueV - value) / float(Epsilon);
}

ConstantColor3fTexture::ConstantColor3fTexture(const Color3f & value) : m_value(value) { }

Color3f ConstantColor3fTexture::eval(const Intersection & isect, bool bFilter) const
{
    return m_value;
}

Color3f ConstantColor3fTexture::getAverage() const
{
    return m_value;
}

Color3f ConstantColor3fTexture::getMinimum() const
{
    return m_value;
}

Color3f ConstantColor3fTexture::getMaximum() const
{
    return m_value;
}

Vector3i ConstantColor3fTexture::getDimension() const
{
    return Vector3i(1, 1, 1);
}

bool ConstantColor3fTexture::isConstant() const
{
    return true;
}

bool ConstantColor3fTexture::isMonochromatic() const
{
    return m_value[0] == m_value[1] && m_value[1] == m_value[2];
}

std::string ConstantColor3fTexture::toString() const
{
    return tfm::format(
            "ConstantColor3fTexture[value = %s]", m_value.toString()
    );
}

ConstantFloatTexture::ConstantFloatTexture(float Value) : m_value(Value) { }

Color3f ConstantFloatTexture::eval(const Intersection & Isect, bool bFilter) const
{
    return Color3f(m_value);
}

Color3f ConstantFloatTexture::getAverage() const
{
    return Color3f(m_value);
}

Color3f ConstantFloatTexture::getMinimum() const
{
    return Color3f(m_value);
}

Color3f ConstantFloatTexture::getMaximum() const
{
    return Color3f(m_value);
}

Vector3i ConstantFloatTexture::getDimension() const
{
    return Vector3i(1, 1, 1);
}

bool ConstantFloatTexture::isConstant() const
{
    return true;
}

bool ConstantFloatTexture::isMonochromatic() const
{
    return true;
}

std::string ConstantFloatTexture::toString() const
{
    return tfm::format(
            "ConstantFloatTexture[value = %f]", m_value
    );
}

Color3fAdditionTexture::Color3fAdditionTexture(const Texture * pTextureA, const Texture * pTextureB) :
        m_pTextureA(pTextureA), m_pTextureB(pTextureB)
{
    CHECK_NOTNULL(m_pTextureA);
    CHECK_NOTNULL(m_pTextureB);
}

Color3f Color3fAdditionTexture::eval(const Intersection & isect, bool bFilter) const
{
    return m_pTextureA->eval(isect, bFilter) + m_pTextureB->eval(isect, bFilter);
}

Color3f Color3fAdditionTexture::getAverage() const
{
    return m_pTextureA->getAverage() + m_pTextureB->getAverage();
}

Color3f Color3fAdditionTexture::getMinimum() const
{
    // This is a conservative estimate
    return m_pTextureA->getMinimum() + m_pTextureB->getMinimum();
}

Color3f Color3fAdditionTexture::getMaximum() const
{
    // This is a conservative estimate
    return m_pTextureA->getMaximum() + m_pTextureB->getMaximum();
}

Vector3i Color3fAdditionTexture::getDimension() const
{
    LOG(WARNING) << "Color3fAdditionTexture::getDimension() - information meaningless!";
    return Vector3i(0, 0, 0);
}

bool Color3fAdditionTexture::isConstant() const
{
    return m_pTextureA->isConstant() && m_pTextureB->isConstant();
}

bool Color3fAdditionTexture::isMonochromatic() const
{
    return m_pTextureA->isMonochromatic() && m_pTextureB->isMonochromatic();
}

std::string Color3fAdditionTexture::toString() const
{
    return tfm::format(
            "Color3fAdditionTexture[\n"
            " textureA = %s,\n"
            " textureB = %s,\n"
            "]",
            indent(m_pTextureA->toString()),
            indent(m_pTextureB->toString())
    );
}

Color3fSubtractionTexture::Color3fSubtractionTexture(const Texture * pTextureA, const Texture * pTextureB) :
        m_pTextureA(pTextureA), m_pTextureB(pTextureB)
{
    CHECK_NOTNULL(m_pTextureA);
    CHECK_NOTNULL(m_pTextureB);
}

Color3f Color3fSubtractionTexture::eval(const Intersection & Isect, bool bFilter) const
{
    return m_pTextureA->eval(Isect, bFilter) - m_pTextureB->eval(Isect, bFilter);
}

Color3f Color3fSubtractionTexture::getAverage() const
{
    return m_pTextureA->getAverage() - m_pTextureB->getAverage();
}

Color3f Color3fSubtractionTexture::getMinimum() const
{
    // This is a conservative estimate
    return m_pTextureA->getMinimum() - m_pTextureB->getMinimum();
}

Color3f Color3fSubtractionTexture::getMaximum() const
{
    // This is a conservative estimate
    return m_pTextureA->getMaximum() - m_pTextureB->getMaximum();
}

Vector3i Color3fSubtractionTexture::getDimension() const
{
    LOG(WARNING) << "Color3fAdditionTexture::getDimension() - information meaningless!";
    return Vector3i(0, 0, 0);
}

bool Color3fSubtractionTexture::isConstant() const
{
    return m_pTextureA->isConstant() && m_pTextureB->isConstant();
}

bool Color3fSubtractionTexture::isMonochromatic() const
{
    return m_pTextureA->isMonochromatic() && m_pTextureB->isMonochromatic();
}

std::string Color3fSubtractionTexture::toString() const
{
    return tfm::format(
            "Color3fSubtractionTexture[\n"
            " textureA = %s,\n"
            " textureB = %s,\n"
            "]",
            indent(m_pTextureA->toString()),
            indent(m_pTextureB->toString())
    );
}

Color3fProductTexture::Color3fProductTexture(const Texture * pTextureA, const Texture * pTextureB) :
        m_pTextureA(pTextureA), m_pTextureB(pTextureB)
{
    CHECK_NOTNULL(m_pTextureA);
    CHECK_NOTNULL(m_pTextureB);
}

Color3f Color3fProductTexture::eval(const Intersection & isect, bool bFilter) const
{
    return m_pTextureA->eval(isect, bFilter) * m_pTextureB->eval(isect, bFilter);
}

Color3f Color3fProductTexture::getAverage() const
{
    LOG(ERROR) << "Color3fProductTexture::getAverage() - information unavailable!";
    return Color3f(0.0f);
}

Color3f Color3fProductTexture::getMinimum() const
{
    // This is a conservative estimate
    return m_pTextureA->getMinimum() * m_pTextureB->getMinimum();
}

Color3f Color3fProductTexture::getMaximum() const
{
    // This is a conservative estimate
    return m_pTextureA->getMaximum() * m_pTextureB->getMaximum();
}

Vector3i Color3fProductTexture::getDimension() const
{
    LOG(WARNING) << "Color3fAdditionTexture::getDimension() - information meaningless!";
    return Vector3i(0, 0, 0);
}

bool Color3fProductTexture::isConstant() const
{
    return m_pTextureA->isConstant() && m_pTextureB->isConstant();
}

bool Color3fProductTexture::isMonochromatic() const
{
    return m_pTextureA->isMonochromatic() && m_pTextureB->isMonochromatic();
}

std::string Color3fProductTexture::toString() const
{
    return tfm::format(
            "Color3fProductTexture[\n"
            " textureA = %s,\n"
            " textureB = %s,\n"
            "]",
            indent(m_pTextureA->toString()),
            indent(m_pTextureB->toString())
    );
}

NORI_NAMESPACE_END

