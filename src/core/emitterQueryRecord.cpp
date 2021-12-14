//
// Created by superqqli on 2021/12/13.
//
#include <nori/core/emitterQueryRecord.h>
#include <nori/core/emitter.h>

NORI_NAMESPACE_BEGIN

EmitterQueryRecord::EmitterQueryRecord()
{

}

EmitterQueryRecord::EmitterQueryRecord(const Point3f & ref) :
        ref(ref)
{

}

EmitterQueryRecord::EmitterQueryRecord(const Emitter * pEmitter, const Point3f & ref, const Point3f & p, const Normal3f & n) :
        pEmitter(pEmitter), ref(ref), p(p), n(n)
{
    wi = p - ref;
    distance = wi.norm();
    wi /= distance;
}

std::string EmitterQueryRecord::toString() const
{
    return tfm::format(
            "EmitterQueryRecord[\n"
            "  emitter = \"%s\",\n"
            "  ref = %s,\n"
            "  p = %s,\n"
            "  n = %s,\n"
            "  pdf = %f,\n"
            "  wi = %s,\n"
            "  distance = %f\n"
            "]",
            indent(pEmitter->toString()),
            ref.toString(),
            p.toString(),
            n.toString(),
            pdf,
            wi.toString(),
            distance
    );
}

NORI_NAMESPACE_END