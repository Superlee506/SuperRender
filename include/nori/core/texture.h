//
// Created by superqqli on 2021/12/28.
//

#pragma once
#include <nori/core/common.h>
#include <nori/core/object.h>
#include <nori/core/memoryHelper.h>

NORI_NAMESPACE_BEGIN

std::unique_ptr<float[]> loadImageFromFileR(
        const std::string & filename,
        float gamma,
        int & width,
        int & height,
        float * pAverage = nullptr,
        float * pMaximum = nullptr,
        float * pMinimum = nullptr
);

std::unique_ptr<Color3f[]> loadImageFromFileRGB(
        const std::string & filename,
        float gamma,
        int & width,
        int & height,
        Color3f * pAverage = nullptr,
        Color3f * pMaximum = nullptr,
        Color3f * pMinimum = nullptr
);

/**
* Basic class of all textures
*/
class Texture : public NoriObject
{
public:
    /**
    * \brief Return the texture value at \c Isect
    */
    virtual Color3f eval(const Intersection & isect, bool bFilter = true) const;

    /**
    * \brief Return the texture gradient at isect. This function is usually
    * implemented pointwise without any kind of filtering. Length of the array
    * 'pGradients' should be at least 2 (U and V component respectively).
    */
    virtual void evalGradient(const Intersection & isect, Color3f * pGradients) const;

    /// Return the component-wise average value of the texture over its domain
    virtual Color3f getAverage() const;

    /// Return the component-wise minimum of the texture over its domain
    virtual Color3f getMinimum() const;

    /// Return the component-wise maximum of the texture over its domain
    virtual Color3f getMaximum() const;

    /// Return the dimension in pixels, if applicable
    virtual Vector3i getDimension() const;

    /// Return whether the texture takes on a constant value everywhere
    virtual bool isConstant() const;

    /// Return whether the texture is monochromatic / spectrally uniform
    virtual bool isMonochromatic() const;

    /// Some textures are only proxies for an actual implementation.This function returns the actual texture implementation to be used.
    virtual Texture * actualTexture();

    /**
    * \brief Return the type of object (i.e. Mesh/BSDF/etc.)
    * provided by this instance
    * */
    virtual EClassType getClassType() const override;

protected:
    Point2f m_uvOffset;
    Vector2f m_uvScale;
};

/**
*  Base class of all 2D textures
*/
class Texture2D : public Texture
{
public:
    /**
    * \brief Return the texture value at \c isect
    * \param bFilter
    *    Specifies whether a filtered texture lookup is desired. Note
    *    that this does not mean that filtering will actually be used.
    */
    virtual Color3f eval(const Intersection & isect, bool bFilter = true) const override;

    /// Filtered texture lookup -- Texture2D subclasses must provide this function
    virtual Color3f eval(const Point2f & UV, const Vector2f & D0, const Vector2f & D1) const = 0;

    /// Unfiltered texture lookup -- Texture2D subclasses must provide this function
    virtual Color3f eval(const Point2f & UV) const = 0;

    /**
    * \brief Return the texture gradient at isect. This function is usually
    * implemented pointwise without any kind of filtering. Length of the array
    * 'pGradients' should be at least 2 (U and V component respectively).
    */
    virtual void evalGradient(const Intersection & isect, Color3f * pGradients) const override;

    /// Unfiltered texture lookup
    virtual void evalGradient(const Point2f & uv, Color3f * pGradients) const;

};

/* ============================================================ */
/*                  Some very basic textures                    */
/* ============================================================ */

class ConstantColor3fTexture : public Texture
{
public:
    ConstantColor3fTexture(const Color3f & value);

    virtual Color3f eval(const Intersection & isect, bool bFilter = true) const override;

    virtual Color3f getAverage() const override;

    virtual Color3f getMinimum() const override;

    virtual Color3f getMaximum() const override;

    virtual Vector3i getDimension() const override;

    virtual bool isConstant() const override;

    virtual bool isMonochromatic() const override;

    virtual std::string toString() const override;

protected:
    Color3f m_value;
};

class ConstantFloatTexture : public Texture
{
public:
    ConstantFloatTexture(float Value);

    virtual Color3f eval(const Intersection & Isect, bool bFilter = true) const override;

    virtual Color3f getAverage() const override;

    virtual Color3f getMinimum() const override;

    virtual Color3f getMaximum() const override;

    virtual Vector3i getDimension() const override;

    virtual bool isConstant() const override;

    virtual bool isMonochromatic() const override;

    virtual std::string toString() const override;

protected:
    float m_value;
};

class Color3fAdditionTexture : public Texture
{
public:
    Color3fAdditionTexture(const Texture * pTextureA, const Texture * pTextureB);

    virtual Color3f eval(const Intersection & isect, bool bFilter = true) const override;

    virtual Color3f getAverage() const override;

    virtual Color3f getMinimum() const override;

    virtual Color3f getMaximum() const override;

    virtual Vector3i getDimension() const override;

    virtual bool isConstant() const override;

    virtual bool isMonochromatic() const override;

    virtual std::string toString() const override;

protected:
    const Texture * m_pTextureA;
    const Texture * m_pTextureB;
};

class Color3fSubtractionTexture : public Texture
{
public:
    Color3fSubtractionTexture(const Texture * pTextureA, const Texture * pTextureB);

    virtual Color3f eval(const Intersection & Isect, bool bFilter = true) const override;

    virtual Color3f getAverage() const override;

    virtual Color3f getMinimum() const override;

    virtual Color3f getMaximum() const override;

    virtual Vector3i getDimension() const override;

    virtual bool isConstant() const override;

    virtual bool isMonochromatic() const override;

    virtual std::string toString() const override;

protected:
    const Texture * m_pTextureA;
    const Texture * m_pTextureB;
};

class Color3fProductTexture : public Texture
{
public:
    Color3fProductTexture(const Texture * pTextureA, const Texture * pTextureB);

    virtual Color3f eval(const Intersection & isect, bool bFilter = true) const override;

    virtual Color3f getAverage() const override;

    virtual Color3f getMinimum() const override;

    virtual Color3f getMaximum() const override;

    virtual Vector3i getDimension() const override;

    virtual bool isConstant() const override;

    virtual bool isMonochromatic() const override;

    virtual std::string toString() const override;

protected:
    const Texture * m_pTextureA;
    const Texture * m_pTextureB;
};

NORI_NAMESPACE_END