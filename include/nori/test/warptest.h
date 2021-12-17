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

#include <nanogui/screen.h>
#include <nanogui/label.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/icons.h>
#include <nanogui/combobox.h>
#include <nanogui/slider.h>
#include <nanogui/textbox.h>
#include <nanogui/checkbox.h>
#include <nanogui/messagedialog.h>
#include <nanogui/renderpass.h>
#include <nanogui/shader.h>
#include <nanogui/texture.h>
#include <nanogui/screen.h>
#include <nanogui/opengl.h>
#include <nanogui/window.h>

#include <nanovg_gl.h>

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

using namespace nanogui;
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


class WarpTestScreen : public Screen {
public:

    WarpTestScreen();

    static float mapParameter(WarpType warpType, float parameterValue);

    void refresh();

    bool mouse_motion_event(const Vector2i &p, const Vector2i &rel,
                            int button, int modifiers);

    bool mouse_button_event(const Vector2i &p, int button, bool down, int modifiers);

    void draw_contents();

    void drawHistogram(const nanogui::Vector2i &pos_, const nanogui::Vector2i &size_, Texture *texture);

    void runTest();

    void initializeGUI();

private:
    nanogui::ref<Shader> m_pointShader, m_gridShader, m_histogramShader, m_arrowShader;
    Window *m_window;
    Slider *m_pointCountSlider, *m_parameterSlider, *m_parameter2Slider, *m_angleSlider;
    TextBox *m_pointCountBox, *m_parameterBox, *m_parameter2Box, *m_angleBox;
    nanogui::ref<nanogui::Texture> m_textures[2];
    ComboBox *m_pointTypeBox;
    ComboBox *m_warpTypeBox;
    CheckBox *m_gridCheckBox;
    CheckBox *m_brdfValueCheckBox;
    Arcball m_arcball;
    int m_pointCount, m_lineCount;
    bool m_drawHistogram;
    std::unique_ptr<BSDF> m_brdf;
    BSDFQueryRecord m_bRec;
    std::pair<bool, std::string> m_testResult;
    nanogui::ref<RenderPass> m_renderPass;
};


std::tuple<WarpType, float, float> parse_arguments(int argc, char **argv);


int main(int argc, char **argv);
