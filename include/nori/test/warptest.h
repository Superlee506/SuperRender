/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#pragma once

#include <nori/core/common.h>
#include <nori/core/object.h>
#include <nori/core/warp.h>
#include <nori/core/bsdf.h>
#include <nori/core/bsdfQueryRecord.h>
#include <nori/core/guiBase.h>
#include <nori/gui/shader.h>
#include <nori/core/vector.h>

#include <pcg32.h>
#include <hypothesis.h>
#include <tinyformat.h>

#include <Eigen/Geometry>

/* =======================================================================
 *   WARNING    WARNING    WARNING    WARNING    WARNING    WARNING
 * =======================================================================
 *   Remember to put on SAFETY GOGGLES before looking at this file. You
 *   are most certainly not expected to read or understand any of it.
 * ======================================================================= */

#if defined(_MSC_VER)
#  pragma warning (disable: 4305 4244)
#endif

using namespace std;

using nori::NoriException;
using nori::NoriObjectFactory;
using nori::Point2f;
using nori::Point2i;
using nori::Point3f;
using nori::Warp;
using nori::PropertyList;
using nori::BSDF;
using nori::BSDFQueryRecord;
using nori::Color3f;
using nori::GuiBase;
using nori::Vector2i;
using nori::Shader;



enum PointType : int {
    Independent = 0,
    Grid,
    Stratified
};

enum WarpType : int {
    Square = 0,
    Tent,
    Disk,
    UniformSphere,
    UniformHemisphere,
    CosineHemisphere,
    Beckmann,
    MicrofacetBRDF,
    WarpTypeCount
};

static const std::string kWarpTypeNames[WarpTypeCount] = {
    "square", "tent", "disk", "uniform_sphere", "uniform_hemisphere",
    "cosine_hemisphere", "beckmann", "microfacet_brdf"
};


struct WarpTest {
    static const int kDefaultXres = 51;
    static const int kDefaultYres = 51;

    WarpType warpType;
    float parameterValue;
    BSDF *bsdf;
    BSDFQueryRecord bRec;
    int xres, yres, res;

    // Observed and expected frequencies, initialized after calling run().
    std::unique_ptr<double[]> obsFrequencies, expFrequencies;

    WarpTest(WarpType warpType_, float parameterValue_, BSDF *bsdf_ = nullptr,
             BSDFQueryRecord bRec_ = BSDFQueryRecord(nori::Vector3f()),
             int xres_ = kDefaultXres, int yres_ = kDefaultYres);

    std::pair<bool, std::string> run();


    std::pair<Point3f, float> warpPoint(const Point2f &sample);


    void generatePoints(int &pointCount, PointType pointType,
                        nori::MatrixXf &positions, nori::MatrixXf &weights);

    static std::pair<BSDF *, BSDFQueryRecord>
    create_microfacet_bsdf(float alpha, float kd, float bsdfAngle);
};


struct Arcball {
    using Quaternionf = Eigen::Quaternion<float, Eigen::DontAlign>;

    Arcball(float speedFactor = 2.0f);

    void setSize(nori::Vector2i size);

    const nori::Vector2i &size() const;

    void button(nori::Vector2i pos, bool pressed);

    bool motion(nori::Vector2i pos);

    Eigen::Matrix4f matrix() const;


private:
    /// Whether or not this Arcball is currently active.
    bool m_active;

    /// The last click position (which triggered the Arcball to be active / non-active).
    nori::Vector2i m_lastPos;

    /// The size of this Arcball.
    nori::Vector2i m_size;

    /**
     * The current stable state.  When this Arcball is active, represents the
     * state of this Arcball when \ref Arcball::button was called with
     * ``down = true``.
     */
    Quaternionf m_quat;

    /// When active, tracks the overall update to the state.  Identity when non-active.
    Quaternionf m_incr;

    /**
     * The speed at which this Arcball rotates.  Smaller values mean it rotates
     * more slowly, higher values mean it rotates more quickly.
     */
    float m_speedFactor;

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};


class WarpTestGui : public GuiBase {
public:

    WarpTestGui();

protected:
    virtual void drawUI() override;
    virtual void drawContent() override;
    void  refresh();
    void runTest();
    static float mapParameter(WarpType warpType, float parameterValue);
    void loadShaders();

private:
    std::unique_ptr<Shader> m_pPointShader, m_pGridShader,
    m_pHistogramShader, m_pArrowShader;

    Arcball m_arcball;
    bool m_drawHistogram;
    std::unique_ptr<BSDF> m_brdf;
    BSDFQueryRecord m_bRec;
    std::pair<bool, std::string> m_testResult;

    int m_lineCount;
    int m_pointCount = 0;

    std::string m_pointCountStr;
    std::string m_bsdfAngleStr;
    float m_pointCountSlider;
    int m_pointTypeIndexCombo;
    int m_warpMethodIndexCombo;
    float m_warpParam1Slider;
    float m_warpParam2Slider;
    bool m_visualizeGridCheckBox;
    float m_bsdfAngleSlider;
    bool m_visualizeBSDFCheckBox;

};


std::tuple<WarpType, float, float> parse_arguments(int argc, char **argv);


int main(int argc, char **argv);
