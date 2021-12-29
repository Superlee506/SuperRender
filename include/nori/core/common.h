/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#pragma once

#if defined(_MSC_VER)
/* Disable some warnings on MSVC++ */
#pragma warning(disable : 4127 4702 4100 4515 4800 4146 4512)
#define WIN32_LEAN_AND_MEAN     /* Don't ever include MFC on Windows */
#define NOMINMAX                /* Don't override min/max */
#endif

/* Include the basics needed by any Nori file */
// System include
#include <string>
#include <iostream>
#include <algorithm>
#include <vector>
#include <memory>
#include <map>
#include <cstdint>
#include <thread>
#include <stdexcept>
#include <limits>
#include <iomanip>
#include <time.h>

// ext include
#include <Eigen\Core>
#include <Eigen\Geometry>
#include <Eigen\Lu>
#include <glog\logging.h>
#include <tinyformat.h>
#include <ImathPlatform.h>
#include <filesystem\resolver.h>


/* Convenience definitions */
#define NORI_NAMESPACE_BEGIN namespace nori {
#define NORI_NAMESPACE_END }

#if defined(__NORI_APPLE__NORI_)
#define PLATFORM_MACOS
#elif defined(__NORI_linux__NORI_)
#define PLATFORM_LINUX
#elif defined(WIN32)
#define PLATFORM_WINDOWS
#endif

/* "Ray epsilon": relative error threshold for ray intersection computations */
#define Epsilon 1e-4f

/* Used for BSDF::eval() or BSDF::pdf() for the delta distribution. */
#define DeltaEpsilon 1e-3

/* To avoid the numerical error in computation */
#define MIN_ALPHA 5e-4
#define MAX_ALPHA 1.0

/* A few useful constants */
#undef M_PI

#define M_PI         3.14159265358979323846f
#define INV_PI       0.31830988618379067154f
#define INV_TWOPI    0.15915494309189533577f
#define INV_FOURPI   0.07957747154594766788f
#define SQRT_TWO     1.41421356237309504880f
#define INV_SQRT_TWO 0.70710678118654752440f

/* XML fields name */
#define XML_TYPE(Field)     Field##_XmlType
#define XML_VALUE(Field)    Field##_XmlValue

#define XML_TRANSFORM_TRANSLATE                  "translate"
#define XML_TRANSFORM_MATRIX                     "matrix"
#define XML_TRANSFORM_ROTATE                     "rotate"
#define XML_TRANSFORM_ANGLE                      "angle"
#define XML_TRANSFORM_AXIS                       "axis"
#define XML_TRANSFORM_SCALE                      "scale"
#define XML_TRANSFORM_LOOKAT                     "lookat"
#define XML_TRANSFORM_ORIGIN                     "origin"
#define XML_TRANSFORM_TARGET                     "target"
#define XML_TRANSFORM_UP                         "up"

#define XML_INTEGRATOR                           "integrator"
#define XML_INTEGRATOR_NORMAL                    "normals"
#define XML_INTEGRATOR_SIMPLE                    "simple"
#define XML_INTEGRATOR_SIMPLE_POSITION           "position"
#define XML_INTEGRATOR_SIMPLE_POWER              "power"
#define XML_INTEGRATOR_AO                        "ao"
#define XML_INTEGRATOR_AO_ALPHA                  "alpha"
#define XML_INTEGRATOR_AO_SAMPLE_COUNT           "sampleCount"
#define XML_INTEGRATOR_WHITTED                   "whitted"
#define XML_INTEGRATOR_WHITTED_DEPTH             "depth"
#define XML_INTEGRATOR_PATH_EMS                  "path_ems"
#define XML_INTEGRATOR_PATH_EMS_DEPTH            "depth"
#define XML_INTEGRATOR_PATH_MATS                 "path_mats"
#define XML_INTEGRATOR_PATH_MATS_DEPTH           "depth"
#define XML_INTEGRATOR_PATH_MIS                  "path_mis"
#define XML_INTEGRATOR_PATH_MIS_DEPTH            "depth"

#define XML_EMITTER                              "emitter"
#define XML_EMITTER_AREA_LIGHT                   "area"
#define XML_EMITTER_AREA_LIGHT_RADIANCE          "radiance"
#define XML_EMITTER_POINT_LIGHT                  "point"
#define XML_EMITTER_POINT_LIGHT_POSITION         "position"
#define XML_EMITTER_POINT_LIGHT_POWER            "power"
#define XML_EMITTER_ENVIRONMENT_LIGHT            "env"
#define XML_EMITTER_ENVIRONMENT_LIGHT_FILENAME   "filename"
#define XML_EMITTER_ENVIRONMENT_LIGHT_SCALE      "scale"
#define XML_EMITTER_ENVIRONMENT_LIGHT_TO_WORLD   "toWorld"
#define XML_EMITTER_DIRECTIONAL_LIGHT            "directional"
#define XML_EMITTER_DIRECTIONAL_LIGHT_IRRADIANCE "irradiance"
#define XML_EMITTER_DIRECTIONAL_LIGHT_DIRECTION  "direction"
#define XML_EMITTER_CONSTANT_LIGHT               "constant"
#define XML_EMITTER_CONSTANT_LIGHT_RADIANCE      "radiance"

#define XML_TEXTURE                              "texture"
#define XML_TEXTURE_BITMAP                       "bitmap"
#define XML_TEXTURE_BITMAP_FILENAME              "filename"
#define XML_TEXTURE_BITMAP_GAMMA                 "gamma"
#define XML_TEXTURE_BITMAP_WRAP_MODE             "wrapMode"
#define XML_TEXTURE_BITMAP_WRAP_MODE_U           "uWrapMode"
#define XML_TEXTURE_BITMAP_WRAP_MODE_V           "vWrapMode"
#define XML_TEXTURE_BITMAP_WRAP_MODE_REPEAT      "repeat"
#define XML_TEXTURE_BITMAP_WRAP_MODE_CLAMP       "clamp"
#define XML_TEXTURE_BITMAP_WRAP_MODE_BLACK       "black"
#define XML_TEXTURE_BITMAP_FILTER_TYPE           "filterType"
#define XML_TEXTURE_BITMAP_FILTER_TYPE_NEAREST   "nearest"
#define XML_TEXTURE_BITMAP_FILTER_TYPE_BILINEAR  "bilinear"
#define XML_TEXTURE_BITMAP_FILTER_TYPE_TRILINEAR "trilinear"
#define XML_TEXTURE_BITMAP_FILTER_TYPE_EWA       "ewa"
#define XML_TEXTURE_BITMAP_MAX_ANISOTROPY        "maxAnisotropy"
#define XML_TEXTURE_BITMAP_OFFSE_U               "uOffset"
#define XML_TEXTURE_BITMAP_OFFSE_V               "vOffset"
#define XML_TEXTURE_BITMAP_SCALE_U               "uScale"
#define XML_TEXTURE_BITMAP_SCALE_V               "vScale"
#define XML_TEXTURE_BITMAP_CHANNEL               "channel"
#define XML_TEXTURE_BITMAP_CHANNEL_R             "r"
#define XML_TEXTURE_BITMAP_CHANNEL_RGB           "rgb"
#define XML_TEXTURE_CHECKERBOARD                 "checkerboard"
#define XML_TEXTURE_CHECKERBOARD_BLOCKS          "blocks"
#define XML_TEXTURE_CHECKERBOARD_COLOR_A         "colorA"
#define XML_TEXTURE_CHECKERBOARD_COLOR_B         "colorB"
#define XML_TEXTURE_CHECKERBOARD_OFFSE_U         "uOffset"
#define XML_TEXTURE_CHECKERBOARD_OFFSE_V         "vOffset"
#define XML_TEXTURE_CHECKERBOARD_SCALE_U         "uScale"
#define XML_TEXTURE_CHECKERBOARD_SCALE_V         "vScale"
#define XML_TEXTURE_WIREFRAME                    "wireframe"
#define XML_TEXTURE_WIREFRAME_INTERIOR_COLOR     "interiorColor"
#define XML_TEXTURE_WIREFRAME_EDGE_COLOR         "edgeColor"
#define XML_TEXTURE_WIREFRAME_EDGE_WIDTH         "edgeWidth"
#define XML_TEXTURE_WIREFRAME_TRANSITION_WIDTH   "transitionWidth"
#define XML_TEXTURE_WIREFRAME_OFFSE_U            "uOffset"
#define XML_TEXTURE_WIREFRAME_OFFSE_V            "vOffset"
#define XML_TEXTURE_WIREFRAME_SCALE_U            "uScale"
#define XML_TEXTURE_WIREFRAME_SCALE_V            "vScale"
#define XML_TEXTURE_GRID                         "grid"
#define XML_TEXTURE_GRID_COLOR_BACKGROUND        "backgroundColor"
#define XML_TEXTURE_GRID_COLOR_LINE              "lineColor"
#define XML_TEXTURE_GRID_LINE_WIDTH              "lineWidth"
#define XML_TEXTURE_GRID_LINES                   "lines"
#define XML_TEXTURE_GRID_OFFSE_U                 "uOffset"
#define XML_TEXTURE_GRID_OFFSE_V                 "vOffset"
#define XML_TEXTURE_GRID_SCALE_U                 "uScale"
#define XML_TEXTURE_GRID_SCALE_V                 "vScale"
#define XML_TEXTURE_CURVATURE                    "curvature"
#define XML_TEXTURE_CURVATURE_POSITIVE_COLOR     "positiveColor"
#define XML_TEXTURE_CURVATURE_NEGATIVE_COLOR     "negativeColor"
#define XML_TEXTURE_CURVATURE_SCALE              "scale"
#define XML_TEXTURE_CURVATURE_TYPE               "curvatureType"
#define XML_TEXTURE_CURVATURE_OFFSE_U            "uOffset"
#define XML_TEXTURE_CURVATURE_OFFSE_V            "vOffset"
#define XML_TEXTURE_CURVATURE_SCALE_U            "uScale"
#define XML_TEXTURE_CURVATURE_SCALE_V            "vScale"
#define XML_TEXTURE_SCALE                        "scale"
#define XML_TEXTURE_SCALE_SCALE                  "scale"

#define XML_ACCELERATION                         "acceleration"
#define XML_ACCELERATION_BRUTO_LOOP              "bruto"
#define XML_ACCELERATION_BVH                     "bvh"
#define XML_ACCELERATION_BVH_LEAF_SIZE           "leafSize"
#define XML_ACCELERATION_BVH_SPLIT_METHOD        "splitMethod"
#define XML_ACCELERATION_BVH_SPLIT_METHOD_CENTER "center"
#define XML_ACCELERATION_BVH_SPLIT_METHOD_SAH    "sah"
#define XML_ACCELERATION_HLBVH                   "hlbvh"
#define XML_ACCELERATION_HLBVH_LEAF_SIZE         "leafSize"

#define XML_SCENE                                "scene"
#define XML_SCENE_BACKGROUND                     "background"
#define XML_SCENE_FORCE_BACKGROUND               "forceBackground"

#define XML_MESH                                 "mesh"
#define XML_MESH_WAVEFRONG_OBJ                   "obj"
#define XML_MESH_WAVEFRONG_OBJ_FILENAME          "filename"
#define XML_MESH_WAVEFRONG_OBJ_TO_WORLD          "toWorld"

#define XML_BSDF                                 "bsdf"
#define XML_BSDF_GGX                             "ggx"
#define XML_BSDF_BECKMANN                        "beckmann"
#define XML_BSDF_DIELECTRIC                      "dielectric"
#define XML_BSDF_DIELECTRIC_INT_IOR              "intIOR"
#define XML_BSDF_DIELECTRIC_EXT_IOR              "extIOR"
#define XML_BSDF_DIELECTRIC_KS_REFLECT           "ksReflect"
#define XML_BSDF_DIELECTRIC_KS_REFRACT           "ksRefract"
#define XML_BSDF_DIFFUSE                         "diffuse"
#define XML_BSDF_DIFFUSE_ALBEDO                  "albedo"
#define XML_BSDF_MIRROR                          "mirror"
#define XML_BSDF_MICROFACET                      "microfacet"
#define XML_BSDF_MICROFACET_ALPHA                "alpha"
#define XML_BSDF_MICROFACET_INT_IOR              "intIOR"
#define XML_BSDF_MICROFACET_EXT_IOR              "extIOR"
#define XML_BSDF_MICROFACET_KD                   "kd"
#define XML_BSDF_CONDUCTOR                       "conductor"
#define XML_BSDF_CONDUCTOR_INT_IOR               "intIOR"
#define XML_BSDF_CONDUCTOR_EXT_IOR               "extIOR"
#define XML_BSDF_CONDUCTOR_K                     "k"
#define XML_BSDF_CONDUCTOR_KS                    "ks"
#define XML_BSDF_PLASTIC                         "plastic"
#define XML_BSDF_PLASTIC_INT_IOR                 "intIOR"
#define XML_BSDF_PLASTIC_EXT_IOR                 "extIOR"
#define XML_BSDF_PLASTIC_KS                      "ks"
#define XML_BSDF_PLASTIC_KD                      "kd"
#define XML_BSDF_PLASTIC_NONLINEAR               "nonlinear"
#define XML_BSDF_ROUGH_CONDUCTOR                 "roughConductor"
#define XML_BSDF_ROUGH_CONDUCTOR_INT_IOR         "intIOR"
#define XML_BSDF_ROUGH_CONDUCTOR_EXT_IOR         "extIOR"
#define XML_BSDF_ROUGH_CONDUCTOR_K               "k"
#define XML_BSDF_ROUGH_CONDUCTOR_KS              "ks"
#define XML_BSDF_ROUGH_CONDUCTOR_ALPHA           "alpha"
#define XML_BSDF_ROUGH_CONDUCTOR_ALPHA_U         "alphaU"
#define XML_BSDF_ROUGH_CONDUCTOR_ALPHA_V         "alphaV"
#define XML_BSDF_ROUGH_CONDUCTOR_TYPE            "type"
#define XML_BSDF_ROUGH_CONDUCTOR_AS              "as"
#define XML_BSDF_ROUGH_DIELECTRIC                "roughDielectric"
#define XML_BSDF_ROUGH_DIELECTRIC_INT_IOR        "intIOR"
#define XML_BSDF_ROUGH_DIELECTRIC_EXT_IOR        "extIOR"
#define XML_BSDF_ROUGH_DIELECTRIC_KS_REFLECT     "ksReflect"
#define XML_BSDF_ROUGH_DIELECTRIC_KS_REFRACT     "ksRefract"
#define XML_BSDF_ROUGH_DIELECTRIC_ALPHA          "alpha"
#define XML_BSDF_ROUGH_DIELECTRIC_ALPHA_U        "alphaU"
#define XML_BSDF_ROUGH_DIELECTRIC_ALPHA_V        "alphaV"
#define XML_BSDF_ROUGH_DIELECTRIC_TYPE           "type"
#define XML_BSDF_ROUGH_DIELECTRIC_AS             "as"
#define XML_BSDF_ROUGH_DIFFUSE                   "roughDiffuse"
#define XML_BSDF_ROUGH_DIFFUSE_ALBEDO            "albedo"
#define XML_BSDF_ROUGH_DIFFUSE_ALPHA             "alpha"
#define XML_BSDF_ROUGH_DIFFUSE_FAST_APPROX       "fastApprox"
#define XML_BSDF_ROUGH_PLASTIC                   "roughPlastic"
#define XML_BSDF_ROUGH_PLASTIC_INT_IOR           "intIOR"
#define XML_BSDF_ROUGH_PLASTIC_EXT_IOR           "extIOR"
#define XML_BSDF_ROUGH_PLASTIC_KS                "ks"
#define XML_BSDF_ROUGH_PLASTIC_KD                "kd"
#define XML_BSDF_ROUGH_PLASTIC_NONLINEAR         "nonlinear"
#define XML_BSDF_ROUGH_PLASTIC_TYPE              "type"
#define XML_BSDF_ROUGH_PLASTIC_ALPHA             "alpha"
#define XML_BSDF_ROUGH_PLASTIC_BECKMANN_RFT_DATA "data\\BeckmannRFTData.bin"
#define XML_BSDF_ROUGH_PLASTIC_GGX_RFT_DATA      "data\\GGXRFTData.bin"
#define XML_BSDF_COATING                         "coating"
#define XML_BSDF_COATING_INT_IOR                 "intIOR"
#define XML_BSDF_COATING_EXT_IOR                 "extIOR"
#define XML_BSDF_COATING_THICKNESS               "thickness"
#define XML_BSDF_COATING_SIGMA_A                 "sigmaA"
#define XML_BSDF_COATING_KS                      "ks"
#define XML_BSDF_ROUGH_COATING                   "roughCoating"
#define XML_BSDF_ROUGH_COATING_INT_IOR           "intIOR"
#define XML_BSDF_ROUGH_COATING_EXT_IOR           "extIOR"
#define XML_BSDF_ROUGH_COATING_THICKNESS         "thickness"
#define XML_BSDF_ROUGH_COATING_SIGMA_A           "sigmaA"
#define XML_BSDF_ROUGH_COATING_KS                "ks"
#define XML_BSDF_ROUGH_COATING_ALPHA             "alpha"
#define XML_BSDF_ROUGH_COATING_TYPE              "type"
#define XML_BSDF_ROUGH_COATING_BECKMANN_RFT_DATA "data\\BeckmannRFTData.bin"
#define XML_BSDF_ROUGH_COATING_GGX_RFT_DATA      "data\\GGXRFTData.bin"
#define XML_BSDF_TWO_SIDED                       "twoSided"
#define XML_BSDF_BUMP_MAP                        "bumpMap"
#define XML_BSDF_NORMAL_MAP                      "normalMap"

#define XML_MEDIUM                               "medium"

#define XML_PHASE                                "phase"

#define XML_CAMERA                               "camera"
#define XML_CAMERA_PERSPECTIVE                   "perspective"
#define XML_CAMERA_PERSPECTIVE_WIDTH             "width"
#define XML_CAMERA_PERSPECTIVE_HEIGHT            "height"
#define XML_CAMERA_PERSPECTIVE_TO_WORLD          "toWorld"
#define XML_CAMERA_PERSPECTIVE_FOV               "fov"
#define XML_CAMERA_PERSPECTIVE_NEAR_CLIP         "nearClip"
#define XML_CAMERA_PERSPECTIVE_FAR_CLIP          "farClip"

#define XML_TEST                                 "test"
#define XML_TEST_STUDENT_T                       "ttest"
#define XML_TEST_STUDENT_T_SIGNIFICANCE_LEVEL    "significanceLevel"
#define XML_TEST_STUDENT_T_ANGLES                "angles"
#define XML_TEST_STUDENT_T_REFERENCES            "references"
#define XML_TEST_STUDENT_T_SAMPLE_COUNT          "sampleCount"
#define XML_TEST_CHI2                            "chi2test"
#define XML_TEST_CHI2_SIGNIFICANCE_LEVEL         "significanceLevel"
#define XML_TEST_CHI2_RESOLUTION                 "resolution"
#define XML_TEST_CHI2_MIN_EXP_FREQUENCY          "minExpFrequency"
#define XML_TEST_CHI2_SAMPLE_COUNT               "sampleCount"
#define XML_TEST_CHI2_TEST_COUNT                 "testCount"

#define XML_FILTER                               "rfilter"
#define XML_FILTER_BOX                           "box"
#define XML_FILTER_GAUSSION                      "gaussian"
#define XML_FILTER_GAUSSION_RADIUS               "radius"
#define XML_FILTER_GAUSSION_STDDEV               "stddev"
#define XML_FILTER_MITCHELL_NETRAVALI            "mitchell"
#define XML_FILTER_MITCHELL_NETRAVALI_RADIUS     "radius"
#define XML_FILTER_MITCHELL_NETRAVALI_B          "B"
#define XML_FILTER_MITCHELL_NETRAVALI_C          "C"
#define XML_FILTER_TENT                          "tent"

#define XML_SAMPLER                              "sampler"
#define XML_SAMPLER_INDEPENDENT                  "independent"
#define XML_SAMPLER_INDEPENDENT_SAMPLE_COUNT     "sampleCount"

#define XML_SHAPE                                "shape"


/* Default setting */
#define DEFAULT_ACCELERATION_BVH_LEAF_SIZE         10
#define DEFAULT_ACCELERATION_BVH_SPLIT_METHOD      XML_ACCELERATION_BVH_SPLIT_METHOD_SAH

#define DEFAULT_ACCELERATION_HLBVH_LEAF_SIZE       10

#define DEFAULT_SCENE_ACCELERATION                 XML_ACCELERATION_BRUTO_LOOP

#define DEFAULT_SCENE_SAMPLER                      XML_SAMPLER_INDEPENDENT

#define DEFAULT_CAMERA_OUTPUTSIZE_X                1280
#define DEFAULT_CAMERA_OUTPUTSIZE_Y                720
#define DEFAULT_CAMERA_CAMERA_TO_WORLD             Transform()
#define DEFAULT_CAMERA_FOV                         30.0f
#define DEFAULT_CAMERA_NEAR_CLIP                   1e-4f
#define DEFAULT_CAMERA_FAR_CLIP                    1e4f
#define DEFAULT_CAMERA_FAR_CLIP                    1e4f
#define DEFAULT_CAMERA_RFILTER                     XML_FILTER_GAUSSION

#define DEFAULT_BSDF_DIELECTRIC_INT_IOR            1.5046f
#define DEFAULT_BSDF_DIELECTRIC_EXT_IOR            1.000277f /* Air */
#define DEFAULT_BSDF_DIELECTRIC_KS_REFLECT         Color3f(1.0f)
#define DEFAULT_BSDF_DIELECTRIC_KS_REFRACT         Color3f(1.0f)
#define DEFAULT_BSDF_DIFFUSE_ALBEDO                Color3f(0.5f)
#define DEFAULT_BSDF_MICROFACET_ALPHA              0.1f
#define DEFAULT_BSDF_MICROFACET_INT_IOR            1.5046f
#define DEFAULT_BSDF_MICROFACET_EXT_IOR            1.000277f /* Air */
#define DEFAULT_BSDF_MICROFACET_ALBEDO             Color3f(0.5f)
#define DEFAULT_BSDF_CONDUCTOR_INT_IOR             1.5046f
#define DEFAULT_BSDF_CONDUCTOR_EXT_IOR             1.000277f /* Air */
#define DEFAULT_BSDF_CONDUCTOR_K                   Color3f(1.0f)
#define DEFAULT_BSDF_CONDUCTOR_KS                  Color3f(1.0f)
#define DEFAULT_BSDF_PLASTIC_INT_IOR               1.5046f
#define DEFAULT_BSDF_PLASTIC_EXT_IOR               1.000277f /* Air */
#define DEFAULT_BSDF_PLASTIC_KS                    Color3f(1.0f)
#define DEFAULT_BSDF_PLASTIC_KD                    Color3f(0.5f)
#define DEFAULT_BSDF_PLASTIC_NONLINEAR             false
#define DEFAULT_BSDF_ROUGH_CONDUCTOR_INT_IOR       1.5046f
#define DEFAULT_BSDF_ROUGH_CONDUCTOR_EXT_IOR       1.000277f /* Air */
#define DEFAULT_BSDF_ROUGH_CONDUCTOR_K             Color3f(1.0f)
#define DEFAULT_BSDF_ROUGH_CONDUCTOR_KS            Color3f(1.0f)
#define DEFAULT_BSDF_ROUGH_CONDUCTOR_ALPHA         0.1f
#define DEFAULT_BSDF_ROUGH_CONDUCTOR_ALPHA_U       0.1f
#define DEFAULT_BSDF_ROUGH_CONDUCTOR_ALPHA_V       0.1f
#define DEFAULT_BSDF_ROUGH_CONDUCTOR_TYPE          XML_BSDF_BECKMANN
#define DEFAULT_BSDF_ROUGH_CONDUCTOR_AS            false
#define DEFAULT_BSDF_ROUGH_DIELECTRIC_INT_IOR      1.5046f
#define DEFAULT_BSDF_ROUGH_DIELECTRIC_EXT_IOR      1.000277f /* Air */
#define DEFAULT_BSDF_ROUGH_DIELECTRIC_KS_REFLECT   Color3f(1.0f)
#define DEFAULT_BSDF_ROUGH_DIELECTRIC_KS_REFRACT   Color3f(1.0f)
#define DEFAULT_BSDF_ROUGH_DIELECTRIC_ALPHA        0.1f
#define DEFAULT_BSDF_ROUGH_DIELECTRIC_ALPHA_U      0.1f
#define DEFAULT_BSDF_ROUGH_DIELECTRIC_ALPHA_V      0.1f
#define DEFAULT_BSDF_ROUGH_DIELECTRIC_TYPE         XML_BSDF_BECKMANN
#define DEFAULT_BSDF_ROUGH_DIELECTRIC_AS           false
#define DEFAULT_BSDF_ROUGH_DIFFUSE_ALBEDO          Color3f(0.5f)
#define DEFAULT_BSDF_ROUGH_DIFFUSE_ALPHA           0.2f
#define DEFAULT_BSDF_ROUGH_DIFFUSE_FAST_APPROX     false
#define DEFAULT_BSDF_ROUGH_PLASTIC_INT_IOR         1.49f
#define DEFAULT_BSDF_ROUGH_PLASTIC_EXT_IOR         1.000277f /* Air */
#define DEFAULT_BSDF_ROUGH_PLASTIC_KS              Color3f(1.0f)
#define DEFAULT_BSDF_ROUGH_PLASTIC_KD              Color3f(0.5f)
#define DEFAULT_BSDF_ROUGH_PLASTIC_NONLINEAR       false
#define DEFAULT_BSDF_ROUGH_PLASTIC_TYPE            XML_BSDF_BECKMANN
#define DEFAULT_BSDF_ROUGH_PLASTIC_ALPHA           0.1f
#define DEFAULT_BSDF_COATING_INT_IOR               1.49f
#define DEFAULT_BSDF_COATING_EXT_IOR               1.000277f /* Air */
#define DEFAULT_BSDF_COATING_THICKNESS             1.0f
#define DEFAULT_BSDF_COATING_SIGMA_A               Color3f(0.0f)
#define DEFAULT_BSDF_COATING_KS                    Color3f(1.0f)
#define DEFAULT_BSDF_ROUGH_COATING_INT_IOR         1.49f
#define DEFAULT_BSDF_ROUGH_COATING_EXT_IOR         1.000277f /* Air */
#define DEFAULT_BSDF_ROUGH_COATING_THICKNESS       1.0f
#define DEFAULT_BSDF_ROUGH_COATING_SIGMA_A         Color3f(0.0f)
#define DEFAULT_BSDF_ROUGH_COATING_KS              Color3f(1.0f)
#define DEFAULT_BSDF_ROUGH_COATING_ALPHA           0.1f
#define DEFAULT_BSDF_ROUGH_COATING_TYPE            XML_BSDF_BECKMANN

#define DEFAULT_PATH_TRACING_DEPTH                5

#define DEFAULT_FILTER_GAUSSIAN_RADIUS             2.0f
#define DEFAULT_FILTER_GAUSSIAN_STDDEV             0.5f
#define DEFAULT_FILTER_MITCHELL_RADIUS             2.0f
#define DEFAULT_FILTER_MITCHELL_B                  (1.0f / 3.0f)
#define DEFAULT_FILTER_MITCHELL_C                  (1.0f / 3.0f)

#define DEFAULT_MESH_TO_WORLD                      Transform()

#define DEFAULT_INTEGRATOR_AO_ALPHA                1e6f
#define DEFAULT_INTEGRATOR_AO_SAMPLE_COUNT         16
#define DEFAULT_INTEGRATOR_WHITTED_DEPTH           -1

#define DEFAULT_SAMPLER_INDEPENDENT_SAMPLE_COUNT   1

#define DEFAULT_MESH_BSDF                          XML_BSDF_DIFFUSE

#define DEFAULT_EMITTER_ENVIRONMENT_SCALE          1.0f
#define DEFAULT_EMITTER_ENVIRONMENT_TO_WORLD       Transform()

#define DEFAULT_TEST_STUDENT_T_SIGNIFICANCE_LEVEL  0.01f
#define DEFAULT_TEST_STUDENT_T_ANGLES              ""
#define DEFAULT_TEST_STUDENT_T_REFERENCES          ""
#define DEFAULT_TEST_STUDENT_T_SAMPLE_COUNT        100000

#define DEFAULT_TEST_CHI2_SIGNIFICANCE_LEVEL       0.01f
#define DEFAULT_TEST_CHI2_RESOLUTION               10
#define DEFAULT_TEST_CHI2_MIN_EXP_FREQUENCY        5
#define DEFAULT_TEST_CHI2_SAMPLE_COUNT             -1
#define DEFAULT_TEST_CHI2_TEST_COUNT               5

#define DEFAULT_SCENE_BACKGROUND                   Color3f(0.0f)
#define DEFAULT_SCENE_FORCE_BACKGROUND             false

#define DEFAULT_TEXTURE_BITMAP_GAMMA               1.0f
#define DEFAULT_TEXTURE_BITMAP_WRAP_MODE           XML_TEXTURE_BITMAP_WRAP_MODE_REPEAT
#define DEFAULT_TEXTURE_BITMAP_WRAP_MODE_U         XML_TEXTURE_BITMAP_WRAP_MODE_REPEAT
#define DEFAULT_TEXTURE_BITMAP_WRAP_MODE_V         XML_TEXTURE_BITMAP_WRAP_MODE_REPEAT
#define DEFAULT_TEXTURE_BITMAP_FILTER_TYPE         XML_TEXTURE_BITMAP_FILTER_TYPE_EWA
#define DEFAULT_TEXTURE_BITMAP_MAX_ANISOTROPY      20.0f
#define DEFAULT_TEXTURE_BITMAP_OFFSET_U            0.0f
#define DEFAULT_TEXTURE_BITMAP_OFFSET_V            0.0f
#define DEFAULT_TEXTURE_BITMAP_SCALE_U             1.0f
#define DEFAULT_TEXTURE_BITMAP_SCALE_V             1.0f
#define DEFAULT_TEXTURE_BITMAP_CHANNEL             XML_TEXTURE_BITMAP_CHANNEL_RGB
#define DEFAULT_TEXTURE_CHECKERBOARD_BLOCKS        10
#define DEFAULT_TEXTURE_CHECKERBOARD_COLOR_A       Color3f(0.4f)
#define DEFAULT_TEXTURE_CHECKERBOARD_COLOR_B       Color3f(0.2f)
#define DEFAULT_TEXTURE_CHECKERBOARD_OFFSET_U      0.0f
#define DEFAULT_TEXTURE_CHECKERBOARD_OFFSET_V      0.0f
#define DEFAULT_TEXTURE_CHECKERBOARD_SCALE_U       1.0f
#define DEFAULT_TEXTURE_CHECKERBOARD_SCALE_V       1.0f
#define DEFAULT_TEXTURE_WIREFRAME_INTERIOR_COLOR   Color3f(0.5f)
#define DEFAULT_TEXTURE_WIREFRAME_EDGE_COLOR       Color3f(0.1f)
#define DEFAULT_TEXTURE_WIREFRAME_EDGE_WIDTH       0.0f
#define DEFAULT_TEXTURE_WIREFRAME_TRANSITION_WIDTH 0.5f
#define DEFAULT_TEXTURE_WIREFRAME_OFFSET_U         0.0f
#define DEFAULT_TEXTURE_WIREFRAME_OFFSET_V         0.0f
#define DEFAULT_TEXTURE_WIREFRAME_SCALE_U          1.0f
#define DEFAULT_TEXTURE_WIREFRAME_SCALE_V          1.0f
#define DEFAULT_TEXTURE_GRID_COLOR_BACKGROUND      Color3f(0.2f)
#define DEFAULT_TEXTURE_GRID_COLOR_LINE            Color3f(0.4f)
#define DEFAULT_TEXTURE_GRID_LINE_WIDTH            0.01f
#define DEFAULT_TEXTURE_GRID_LINES                 10
#define DEFAULT_TEXTURE_GRID_OFFSET_U              0.0f
#define DEFAULT_TEXTURE_GRID_OFFSET_V              0.0f
#define DEFAULT_TEXTURE_GRID_SCALE_U               1.0f
#define DEFAULT_TEXTURE_GRID_SCALE_V               1.0f
#define DEFAULT_TEXTURE_CURVATURE_SCALE            1.0f
#define DEFAULT_TEXTURE_CURVATURE_POSITIVE_COLOR   Color3f(1.0f, 0.0f, 0.0f)
#define DEFAULT_TEXTURE_CURVATURE_NEGATIVE_COLOR   Color3f(0.0f, 0.0f, 1.0f)
#define DEFAULT_TEXTURE_CURVATURE_OFFSET_U         0.0f
#define DEFAULT_TEXTURE_CURVATURE_OFFSET_V         0.0f
#define DEFAULT_TEXTURE_CURVATURE_SCALE_U          1.0f
#define DEFAULT_TEXTURE_CURVATURE_SCALE_V          1.0f

/* Forward declarations */
namespace filesystem {
    class path;
    class resolver;
};

NORI_NAMESPACE_BEGIN

/* Forward declarations */
template <typename Scalar, int Dimension>  struct TVector;
template <typename Scalar, int Dimension>  struct TPoint;
template <typename Point, typename Vector> struct TRay;
template <typename Point>                  struct TBoundingBox;

/* Basic Nori data structures (vectors, points, rays, bounding boxes,
   kd-trees) are oblivious to the underlying data type and dimension.
   The following list of typedefs establishes some convenient aliases
   for specific types. */
typedef TVector<float, 1>       Vector1f;
typedef TVector<float, 2>       Vector2f;
typedef TVector<float, 3>       Vector3f;
typedef TVector<float, 4>       Vector4f;
typedef TVector<double, 1>      Vector1d;
typedef TVector<double, 2>      Vector2d;
typedef TVector<double, 3>      Vector3d;
typedef TVector<double, 4>      Vector4d;
typedef TVector<int, 1>         Vector1i;
typedef TVector<int, 2>         Vector2i;
typedef TVector<int, 3>         Vector3i;
typedef TVector<int, 4>         Vector4i;
typedef TPoint<float, 1>        Point1f;
typedef TPoint<float, 2>        Point2f;
typedef TPoint<float, 3>        Point3f;
typedef TPoint<float, 4>        Point4f;
typedef TPoint<double, 1>       Point1d;
typedef TPoint<double, 2>       Point2d;
typedef TPoint<double, 3>       Point3d;
typedef TPoint<double, 4>       Point4d;
typedef TPoint<int, 1>          Point1i;
typedef TPoint<int, 2>          Point2i;
typedef TPoint<int, 3>          Point3i;
typedef TPoint<int, 4>          Point4i;
typedef TBoundingBox<Point1f>   BoundingBox1f;
typedef TBoundingBox<Point2f>   BoundingBox2f;
typedef TBoundingBox<Point3f>   BoundingBox3f;
typedef TBoundingBox<Point4f>   BoundingBox4f;
typedef TBoundingBox<Point1d>   BoundingBox1d;
typedef TBoundingBox<Point2d>   BoundingBox2d;
typedef TBoundingBox<Point3d>   BoundingBox3d;
typedef TBoundingBox<Point4d>   BoundingBox4d;
typedef TBoundingBox<Point1i>   BoundingBox1i;
typedef TBoundingBox<Point2i>   BoundingBox2i;
typedef TBoundingBox<Point3i>   BoundingBox3i;
typedef TBoundingBox<Point4i>   BoundingBox4i;
typedef TRay<Point2f, Vector2f> Ray2f;
typedef TRay<Point3f, Vector3f> Ray3f;

/// Some more forward declarations
class Accel;
class BSDF;
class Bitmap;
class BlockGenerator;
class Camera;
class ImageBlock;
class Integrator;
class KDTree;
class Emitter;
struct EmitterQueryRecord;
class Mesh;
class PrimitiveShape;
struct Intersection;
class NoriObject;
class NoriObjectFactory;
class NoriScreen;
class PhaseFunction;
class ReconstructionFilter;
class Sampler;
class Scene;
class Timer;
struct BSDFQueryRecord;
class DiscretePDF1D;
class DiscretePDF2D;
class Texture;
struct Color3f;
struct Color4f;

/// Import cout, cerr, endl for debugging purposes
using std::cout;
using std::cerr;
using std::endl;

typedef Eigen::Matrix<float,    Eigen::Dynamic, Eigen::Dynamic> MatrixXf;
typedef Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic> MatrixXu;

/// Simple exception class, which stores a human-readable error description
class NoriException : public std::runtime_error {
public:
    /// Variadic template constructor to support printf-style arguments
    template <typename... Args> NoriException(const char *fmt, const Args &... args) 
     : std::runtime_error(tfm::format(fmt, args...)) { }
};

/// Return the number of cores (real and virtual)
extern int getCoreCount();

extern const std::string getAssetPath();

/// utf8 encoder
extern std::string utf8(uint32_t c);

/// Indent a string by the specified number of spaces
extern std::string indent(const std::string &string, int amount = 2);

/// Convert a string to lower case
extern std::string toLower(const std::string &value);

/// Convert a string into an boolean value
extern bool toBool(const std::string &str);

/// Convert a string into a signed integer value
extern int toInt(const std::string &str);

/// Convert a string into an unsigned integer value
extern unsigned int toUInt(const std::string &str);

/// Convert a string into a floating point value
extern float toFloat(const std::string &str);

/// Convert a string into a 3D vector
extern Eigen::Vector3f toVector3f(const std::string &str);

/// Tokenize a string into a list by splitting at 'delim'
extern std::vector<std::string> tokenize(const std::string &s, const std::string &delim = ", ", bool includeEmpty = false);

/// Check if a string ends with another string
extern bool endsWith(const std::string &value, const std::string &ending);

/// Convert a time value in milliseconds into a human-readable string
extern std::string timeString(double time, bool precise = false);

/// Convert a memory amount in bytes into a human-readable string
extern std::string memString(size_t size, bool precise = false);

/// gamma correction of the value
extern float gammaCorrect(float value, float invGamma);

/// Fresnel coefficient for dielectric material. eta = intIOR / extIOR
extern float fresnelDielectric(float cosThetaI, float eta, float invEta, float & cosThetaT);

/// Fresnel coefficient for conductor material. eta = intIOR / extIOR, etaK = K / extIOR
extern Color3f fresnelConductor(float cosThetaI, const Color3f & eta, const Color3f & etaK);

/**
* Approximating the diffuse Frensel reflectance
* for the eta < 1.0 and eta > 1.0 cases.
*/
extern float approxFresnelDiffuseReflectance(float eta);

/// Complete the set {a} to an orthonormal base
extern void coordinateSystem(const Vector3f & va, Vector3f & vb, Vector3f & vc);

/// Reflection in local coordinates
extern Vector3f reflect(const Vector3f & wi);

/// Refraction in local coordinates
extern Vector3f refract(const Vector3f & wi, float cosThetaT, float eta, float invEta);

/// Reflection in global coordinates
extern Vector3f reflect(const Vector3f & wi, const Vector3f & m);

/// Refraction in global coordinates
extern Vector3f refract(const Vector3f & wi, const Vector3f & m, float cosThetaT, float eta, float invEta);

/// Measures associated with probability distributions
enum EMeasure {
    EUnknownMeasure = 0,
    ESolidAngle,
    EDiscrete
};

/// Type of the emitter
enum class EEmitterType
{
    EUnknown = 0,
    EPoint = 1,
    EArea = 2,
    EEnvironment = 3,
    EDirectional = 4
};

/// Type of the light transport. Currently not used, so set it to ERadiance now.
enum class ETransportMode
{
    EUnknown = 0,
    ERadiance = 1,
    EImportance = 2
};

/// Lobe type of the BSDF
enum EBSDFType
{
    ENull                = 0x001,
    EDiffuseReflection   = 0x002,
    EDiffuseTransmission = 0x004,
    EGlossyReflection    = 0x008,
    EGlossyTransmission  = 0x010,
    EDeltaReflection     = 0x020,
    EDeltaTransmission   = 0x040,

    EExtraSampling       = 0x080,
    EUVDependent         = 0x100,
};

//// Convert radians to degrees
inline float radToDeg(float value) { return value * (180.0f / M_PI); }

/// Convert degrees to radians
inline float degToRad(float value) { return value * (M_PI / 180.0f); }

#if !defined(_GNU_SOURCE)
    /// Emulate sincosf using sinf() and cosf()
    inline void sincosf(float theta, float *_sin, float *_cos) {
        *_sin = sinf(theta);
        *_cos = cosf(theta);
    }
#endif

/// Simple floating point clamping function
inline float clamp(float value, float min, float max) {
    if (value < min)
        return min;
    else if (value > max)
        return max;
    else return value;
}

/// Simple integer clamping function
inline int clamp(int value, int min, int max) {
    if (value < min)
        return min;
    else if (value > max)
        return max;
    else return value;
}

/// Linearly interpolate between two values
inline float lerp(float t, float v1, float v2) {
    return ((float) 1 - t) * v1 + t * v2;
}

/// Always-positive modulo operation
inline int mod(int a, int b) {
    int r = a % b;
    return (r < 0) ? r+b : r;
}

inline bool solveLinearSystem2x2(const float A[2][2], const float B[2], float X[2])
{
    float Det = A[0][0] * A[1][1] - A[0][1] * A[1][0];

    constexpr float InvOverflow = 1.0f / std::numeric_limits<float>::max();

    if (std::abs(Det) <= InvOverflow)
    {
        return false;
    }

    float InvDet = 1.0f / Det;

    X[0] = (A[1][1] * B[0] - A[0][1] * B[1]) * InvDet;
    X[1] = (A[0][0] * B[1] - A[1][0] * B[0]) * InvDet;

    return true;
}

/// Compute a direction for the given coordinates in spherical coordinates
extern Vector3f sphericalDirection(float theta, float phi);

/// Compute a direction for the given coordinates in spherical coordinates
extern Point2f sphericalCoordinates(const Vector3f &dir);

/**
 * \brief Calculates the unpolarized fresnel reflection coefficient for a 
 * dielectric material. Handles incidence from either side (i.e.
 * \code cosThetaI<0 is allowed).
 *
 * \param cosThetaI
 *      Cosine of the angle between the normal and the incident ray
 * \param extIOR
 *      Refractive index of the side that contains the surface normal
 * \param intIOR
 *      Refractive index of the interior
 */
extern float fresnel(float cosThetaI, float extIOR, float intIOR);

/**
 * \brief Return the global file resolver instance
 *
 * This class is used to locate resource files (e.g. mesh or
 * texture files) referenced by a scene being loaded
 */
extern filesystem::resolver *getFileResolver();

NORI_NAMESPACE_END
