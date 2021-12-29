/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#pragma once

#include <nori/core/object.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Superclass of all emitters
 */
class Emitter : public NoriObject {
public:
    /**
	* \brief Sample the emitter and return the importance weight (i.e. the
	* value of the Emitter divided by the probability density
	* of the sample with respect to solid angles).
	*
	* \param record    An emitter query record (only Ref is needed except for
	*                  infinity emitter such as EnvironmentLight. In such cases
	*                  Distance is needed as the radius of bounding sphere of the
	*                  entire scene.)
	* \param sample2D  A uniformly distributed sample on \f$[0,1]^2\f$
	* \param sample1D  Another optional sample that might be used in some scenarios.
	*
	* \return The emitter value divided by the probability density of the sample.
	*         A zero value means that sampling failed.
	*/
    virtual Color3f sample(EmitterQueryRecord & record, const Point2f & sample2D, float sample1D) const = 0;

    /**
	* \brief Compute the probability of sampling \c Record.P.
	*
	* This method provides access to the probability density that
	* is realized by the \ref Sample() method.
	*
	* \param record
	*     A record with detailed information on the emitter query.
	*     In most cases, Wi is needed. N and Distance are need for
	*     the AreaLight.
	*
	* \return
	*     A probability/density value
	*/
    virtual float pdf(const EmitterQueryRecord & record) const = 0;

    /**
	* \brief Evaluate the emitter
	*
	* \param record
	*     A record with detailed information on the emitter query.
	*     In most cases, wi is needed. n is needed for AreaLight.
	* \return
	*     The emitter value, evaluated for each color channel
	*/
    virtual Color3f eval(const EmitterQueryRecord & record) const = 0;

    /**
     * \brief Return the type of object (i.e. Mesh/Emitter/etc.) 
     * provided by this instance
     * */
    EClassType getClassType() const;

    /**
	* \brief Set the mesh if the emitter is attached to a mesh
	* */
    void setMesh(Mesh * pMesh);

    EEmitterType getEmitterType() const;

    bool isDelta() const;

protected:
    Mesh * m_pMesh = nullptr;
    EEmitterType m_type = EEmitterType::EUnknown;
};

NORI_NAMESPACE_END
