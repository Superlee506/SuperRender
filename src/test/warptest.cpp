/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include <nori/test/warptest.h>
#include <imgui.h>


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
using nori::utf8;

WarpTest::WarpTest(WarpType warpType_, float parameterValue_, BSDF *bsdf_,
        BSDFQueryRecord bRec_, int xres_, int yres_)
: warpType(warpType_), parameterValue(parameterValue_), bsdf(bsdf_),
bRec(bRec_), xres(xres_), yres(yres_) {

    if (warpType != Square && warpType != Disk && warpType != Tent)
        xres *= 2;
    res = xres * yres;
}

std::pair<bool, std::string> WarpTest::run() {
    int sampleCount = 1000 * res;
    obsFrequencies.reset(new double[res]);
    expFrequencies.reset(new double[res]);
    memset(obsFrequencies.get(), 0, res*sizeof(double));
    memset(expFrequencies.get(), 0, res*sizeof(double));

    nori::MatrixXf points, values;
    generatePoints(sampleCount, Independent, points, values);

    for (int i=0; i<sampleCount; ++i) {
        if (values(0, i) == 0)
            continue;
        nori::Vector3f sample = points.col(i);
        float x, y;

        if (warpType == Square) {
            x = sample.x();
            y = sample.y();
        } else if (warpType == Disk || warpType == Tent) {
            x = sample.x() * 0.5f + 0.5f;
            y = sample.y() * 0.5f + 0.5f;
        } else {
            x = std::atan2(sample.y(), sample.x()) * INV_TWOPI;
            if (x < 0)
                x += 1;
            y = sample.z() * 0.5f + 0.5f;
        }

        int xbin = std::min(xres-1, std::max(0, (int) std::floor(x * xres)));
        int ybin = std::min(yres-1, std::max(0, (int) std::floor(y * yres)));
        obsFrequencies[ybin * xres + xbin] += 1;
    }

    auto integrand = [&](double y, double x) -> double {
        if (warpType == Square) {
            return Warp::squareToUniformSquarePdf(Point2f(x, y));
        } else if (warpType == Disk) {
            x = x * 2 - 1; y = y * 2 - 1;
            return Warp::squareToUniformDiskPdf(Point2f(x, y));
        } else if (warpType == Tent) {
            x = x * 2 - 1; y = y * 2 - 1;
            return Warp::squareToTentPdf(Point2f(x, y));
        } else {
            x *= 2 * M_PI;
            y = y * 2 - 1;

            double sinTheta = std::sqrt(1 - y * y);
            double sinPhi = std::sin(x),
                    cosPhi = std::cos(x);

            nori::Vector3f v((float) (sinTheta * cosPhi),
                             (float) (sinTheta * sinPhi),
                             (float) y);

            if (warpType == UniformSphere)
                return Warp::squareToUniformSpherePdf(v);
            else if (warpType == UniformHemisphere)
                return Warp::squareToUniformHemispherePdf(v);
            else if (warpType == CosineHemisphere)
                return Warp::squareToCosineHemispherePdf(v);
            else if (warpType == Beckmann)
                return Warp::squareToBeckmannPdf(v, parameterValue);
            else if (warpType == MicrofacetBRDF) {
                BSDFQueryRecord br(bRec);
                br.wo = v;
                br.measure = nori::ESolidAngle;
                return bsdf->pdf(br);
            } else {
                throw NoriException("Invalid warp type");
            }
        }
    };

    double scale = sampleCount;
    if (warpType == Square)
        scale *= 1;
    else if (warpType == Disk || warpType == Tent)
        scale *= 4;
    else
        scale *= 4*M_PI;

    double *ptr = expFrequencies.get();
    for (int y=0; y<yres; ++y) {
        double yStart =  y    / (double) yres;
        double yEnd   = (y+1) / (double) yres;
        for (int x=0; x<xres; ++x) {
            double xStart =  x    / (double) xres;
            double xEnd   = (x+1) / (double) xres;
            ptr[y * xres + x] = hypothesis::adaptiveSimpson2D(
                    integrand, yStart, xStart, yEnd, xEnd) * scale;
            if (ptr[y * xres + x] < 0)
                throw NoriException("The Pdf() function returned negative values!");
        }
    }

    /* Write the test input data to disk for debugging */
    hypothesis::chi2_dump(yres, xres, obsFrequencies.get(), expFrequencies.get(), "chitest.m");

    /* Perform the Chi^2 test */
    const int minExpFrequency = 5;
    const float significanceLevel = 0.01f;

    return hypothesis::chi2_test(yres*xres, obsFrequencies.get(),
                                 expFrequencies.get(), sampleCount,
                                 minExpFrequency, significanceLevel, 1);
}


std::pair<Point3f, float> WarpTest::warpPoint(const Point2f &sample) {
    Point3f result;

    switch (warpType) {
        case Square:
            result << Warp::squareToUniformSquare(sample), 0; break;
        case Tent:
            result << Warp::squareToTent(sample), 0; break;
        case Disk:
            result << Warp::squareToUniformDisk(sample), 0; break;
        case UniformSphere:
            result << Warp::squareToUniformSphere(sample); break;
        case UniformHemisphere:
            result << Warp::squareToUniformHemisphere(sample); break;
        case CosineHemisphere:
            result << Warp::squareToCosineHemisphere(sample); break;
        case Beckmann:
            result << Warp::squareToBeckmann(sample, parameterValue); break;
        case MicrofacetBRDF: {
            BSDFQueryRecord br(bRec);
            float value = bsdf->sample(br, sample).getLuminance();
            return std::make_pair(
                    br.wo,
                    value == 0 ? 0.f : bsdf->eval(br)[0]
            );
        }
        default:
            throw std::runtime_error("Unsupported warp type.");
    }

    return std::make_pair(result, 1.f);
}


void WarpTest::generatePoints(int &pointCount, PointType pointType,
                    nori::MatrixXf &positions, nori::MatrixXf &weights) {
    /* Determine the number of points that should be sampled */
    int sqrtVal = (int) (std::sqrt((float) pointCount) + 0.5f);
    float invSqrtVal = 1.f / sqrtVal;
    if (pointType == Grid || pointType == Stratified)
        pointCount = sqrtVal*sqrtVal;

    pcg32 rng;
    positions.resize(3, pointCount);
    weights.resize(1, pointCount);

    for (int i=0; i<pointCount; ++i) {
        int y = i / sqrtVal, x = i % sqrtVal;
        Point2f sample;

        switch (pointType) {
            case Independent:
                sample = Point2f(rng.nextFloat(), rng.nextFloat());
                break;

            case Grid:
                sample = Point2f((x + 0.5f) * invSqrtVal, (y + 0.5f) * invSqrtVal);
                break;

            case Stratified:
                sample = Point2f((x + rng.nextFloat()) * invSqrtVal,
                                 (y + rng.nextFloat()) * invSqrtVal);
                break;
        }

        auto result = warpPoint(sample);
        positions.col(i) = result.first;
        weights(0, i) = result.second;
    }
}

std::pair<BSDF *, BSDFQueryRecord>
WarpTest::create_microfacet_bsdf(float alpha, float kd, float bsdfAngle) {
    PropertyList list;
    list.setFloat("alpha", alpha);
    list.setColor("kd", Color3f(kd));
    auto * brdf = (BSDF *) NoriObjectFactory::createInstance("microfacet", list);

    nori::Vector3f wi(std::sin(bsdfAngle), 0.f,
                      std::max(std::cos(bsdfAngle), 1e-4f));
    wi = wi.normalized();
    BSDFQueryRecord bRec(wi);
    return { brdf, bRec };
}


Arcball::Arcball(float speedFactor)
        : m_active(false), m_lastPos(nori::Vector2i::Zero()), m_size(nori::Vector2i::Zero()),
          m_quat(Quaternionf::Identity()),
          m_incr(Quaternionf::Identity()),
          m_speedFactor(speedFactor) { }

void Arcball::setSize(nori::Vector2i size) { m_size = size; }

const nori::Vector2i &Arcball::size() const { return m_size; }

void Arcball::button(nori::Vector2i pos, bool pressed) {
    m_active = pressed;
    m_lastPos = pos;
    if (!m_active)
        m_quat = (m_incr * m_quat).normalized();
    m_incr = Quaternionf::Identity();
}

bool Arcball::motion(nori::Vector2i pos) {
    if (!m_active)
        return false;

    /* Based on the rotation controller from AntTweakBar */
    float invMinDim = 1.0f / m_size.minCoeff();
    float w = (float) m_size.x(), h = (float) m_size.y();

    float ox = (m_speedFactor * (2*m_lastPos.x() - w) + w) - w - 1.0f;
    float tx = (m_speedFactor * (2*pos.x()      - w) + w) - w - 1.0f;
    float oy = (m_speedFactor * (h - 2*m_lastPos.y()) + h) - h - 1.0f;
    float ty = (m_speedFactor * (h - 2*pos.y())      + h) - h - 1.0f;

    ox *= invMinDim; oy *= invMinDim;
    tx *= invMinDim; ty *= invMinDim;

    nori::Vector3f v0(ox, oy, 1.0f), v1(tx, ty, 1.0f);
    if (v0.squaredNorm() > 1e-4f && v1.squaredNorm() > 1e-4f) {
        v0.normalize(); v1.normalize();
        nori::Vector3f axis = v0.cross(v1);
        float sa = std::sqrt(axis.dot(axis)),
                ca = v0.dot(v1),
                angle = std::atan2(sa, ca);
        if (tx*tx + ty*ty > 1.0f)
            angle *= 1.0f + 0.2f * (std::sqrt(tx*tx + ty*ty) - 1.0f);
        m_incr = Eigen::AngleAxisf(angle, axis.normalized());
        if (!std::isfinite(m_incr.norm()))
            m_incr = Quaternionf::Identity();
    }
    return true;
}

Eigen::Matrix4f Arcball::matrix() const {
    Eigen::Matrix4f result2 = Eigen::Matrix4f::Identity();
    result2.block<3,3>(0, 0) = (m_incr * m_quat).toRotationMatrix();
    return result2;
}


WarpTestGui::WarpTestGui(): m_bRec(nori::Vector3f())
{
    m_width = 1280;
    m_height = 720;
    m_windowsName = "warptest: Sampling and Warping";
    m_drawHistogram = false;
    this->initGui();
    this->loadShaders();
    m_pointCountSlider = 7.0 / 15.0;
    m_pointTypeIndexCombo = 0;
    m_warpMethodIndexCombo = 0;
    m_warpParam1Slider = 0.5;
    m_warpParam2Slider = 0.0f;
    m_visualizeGridCheckBox = false;
    m_bsdfAngleSlider = 0.0f;
    m_visualizeBSDFCheckBox = false;

    m_pointCount = (int) std::pow(2.f, 15 * m_pointCountSlider + 5);
    if (m_pointCount > 1000000) {
        m_pointCountStr = tfm::format("%.2f", m_pointCount * 1e-6f) + " M";
    } else if (m_pointCount > 1000) {
        m_pointCountStr = tfm::format("%.2f", m_pointCount * 1e-3f) + " K";
    } else {
        m_pointCountStr = tfm::format("%i", m_pointCount) + " ";
    }
    m_bsdfAngleStr = tfm::format("%.1f", m_bsdfAngleSlider * 180 - 90);
    m_bsdfAngleStr += utf8(0x00B0);
}

void WarpTestGui::loadShaders()
{
    const GLchar * pointVertexShaderSource =
            /* Vertex shader */
            "#version 330\n"
            "uniform mat4 mvp;\n"
            "in vec3 position;\n"
            "in vec3 color;\n"
            "out vec3 frag_color;\n"
            "void main() {\n"
            "    gl_Position = mvp * vec4(position, 1.0);\n"
            "    if (isnan(position.r)) \n"
            "       frag_color = vec3(0.0);\n"
            "    else\n"
            "       frag_color = color;\n"
            "}";

    const GLchar * pointFragShaderSource =
            "#version 330\n"
            "in vec3 frag_color;\n"
            "out vec4 out_color;\n"
            "void main() {\n"
            "    if (frag_color == vec3(0.0))\n"
            "         discard;\n"
            "    out_color = vec4(frag_color, 1.0);\n"
            "}\n";
    m_pPointShader.reset(new Shader(pointVertexShaderSource, pointFragShaderSource)) ;

    const GLchar* gridVertexShader =
            "#version 330\n"
            "uniform mat4 mvp;\n"
            "in vec3 position;\n"
            "void main() {\n"
            "     gl_Position = mvp * vec4(position, 1.0);\n"
            " }\n";
    const GLchar* gridFragShader =
            "#version 330\n"
            "out vec4 out_color;\n"
            "void main() {\n"
            "    out_color = vec4(vec3(1.0), 0.4);\n"
            "}\n";

    //TODO Alpha blend
    m_pGridShader.reset(new Shader( gridVertexShader, gridFragShader)) ;

    const GLchar* arrowVertexShader =
            "#version 330\n"
            "uniform mat4 mvp;\n"
            "in vec3 position;\n"
            "void main() {\n"
            "    gl_Position = mvp * vec4(position, 1.0);\n"
            "}\n";

    const GLchar* arrowFragShader =
            "#version 330\n"
            "out vec4 out_color;\n"
            "void main() {\n"
            "    out_color = vec4(vec3(1.0), 0.4);\n"
            "}\n";

    m_pArrowShader.reset(new Shader(arrowVertexShader, arrowFragShader));

    const GLchar* hisVertexShader =
            "#version 330\n"
            "uniform mat4 mvp;\n"
            "in vec2 position;\n"
            "out vec2 uv;\n"
            "void main() {\n"
            "    gl_Position = mvp * vec4(position, 0.0, 1.0);\n"
            "     uv = position;\n"
            "}\n";
    const GLchar* hisFragShader =
            "#version 330\n"
            "out vec4 out_color;\n"
            "uniform sampler2D tex;\n"
            "in vec2 uv;\n"
            "vec3 colormap(float v, float vmin, float vmax) {\n"
            "    vec3 c = vec3(1.0);\n"
            "    if (v < vmin)\n"
            "        v = vmin;\n"
            "    if (v > vmax)\n"
            "        v = vmax;\n"
            "    float dv = vmax - vmin;\n"
            "\n"
            "    if (v < (vmin + 0.25 * dv)) {\n"
            "        c.r = 0.0;\n"
            "        c.g = 4.0 * (v - vmin) / dv;\n"
            "     } else if (v < (vmin + 0.5 * dv)) {\n"
            "        c.r = 0.0;\n"
            "        c.b = 1.0 + 4.0 * (vmin + 0.25 * dv - v) / dv;\n"
            "     } else if (v < (vmin + 0.75 * dv)) {\n"
            "         c.r = 4.0 * (v - vmin - 0.5 * dv) / dv;\n"
            "         c.b = 0.0;\n"
            "     } else {\n"
            "          c.g = 1.0 + 4.0 * (vmin + 0.75 * dv - v) / dv;\n"
            "          c.b = 0.0;\n"
            "      }\n"
            "      return c;\n"
            "}\n"
            "void main() {\n"
            "     float value = texture(tex, uv).r;\n"
            "      out_color = vec4(colormap(value, 0.0, 1.0), 1.0);\n"
            " }";
    m_pHistogramShader.reset(new Shader(hisVertexShader,hisFragShader));
}

void WarpTestGui::drawUI()
{
    if (ImGui::Begin("Warp tester", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {

        ImGui::Text("Input point set");
        if(ImGui::SliderFloat("##Point Count", &m_pointCountSlider, 0.0f, 1.0f, ""))
        {
            refresh();
        }
        ImGui::SameLine();
        ImGui::Text(m_pointCountStr.c_str());
        const char* pointTypeItems[] = { "Independent", "Grid", "Stratified" };
        if(ImGui::Combo("Warp Type", &m_pointTypeIndexCombo, pointTypeItems, IM_ARRAYSIZE(pointTypeItems)))
        {
            refresh();
        }

        ImGui::Text("Warping method");
        const char* warpMethodItems[] =  { "Square", "Tent", "Disk", "Sphere", "Hemisphere (unif.)",
                                         "Hemisphere (cos)", "Beckmann distr.", "Microfacet BRDF" };
        if(ImGui::Combo("Warp Method", &m_warpMethodIndexCombo, warpMethodItems, IM_ARRAYSIZE(warpMethodItems)))
        {
            refresh();
        }

        if(ImGui::SliderFloat("Warp Param1", &m_warpParam1Slider, 0.0f, 1.0f))
        {
            refresh();
        }

        if(ImGui::SliderFloat("Warp Param2", &m_warpParam2Slider, 0.0f, 1.0f))
        {
            refresh();
        }
        if(ImGui::Checkbox("Visuallize warped grid", &m_visualizeGridCheckBox))
        {
            refresh();
        }

        ImGui::Text("BSDF parameters");
        if(ImGui::SliderFloat("##BSDF Angle", &m_bsdfAngleSlider, 0.0f, 1.0f, ""))
        {
            refresh();
        }
        ImGui::SameLine();
        ImGui::Text(m_bsdfAngleStr.c_str());
        if(ImGui::Checkbox("Visuallize BSDF values", &m_visualizeBSDFCheckBox))
        {
            refresh();
        }
        std::string testRunLabel = "x" +
                                   std::string(utf8(0x00B2).data()) + " hypothesis test";
        ImGui::Text(testRunLabel.c_str());
        auto windowWidth = ImGui::GetWindowSize().x;
        auto bottonWidth   = ImGui::GetFontSize()*2;
        ImGui::SetCursorPosX((windowWidth - bottonWidth) * 0.5f);
        if(ImGui::Button("Run", ImVec2(bottonWidth, bottonWidth)))
        {
            try
            {
                runTest();

            } catch (const NoriException &e)
            {
                LOG(ERROR) << "An error occurred: " << e.what() << endl;
            }
        }

        ImGui::End();
        // ImGui::TreePop();
    }
}

void WarpTestGui::drawContent()
{

}

float WarpTestGui::mapParameter(WarpType warpType, float parameterValue) {
    if (warpType == Beckmann || warpType == MicrofacetBRDF)
        parameterValue = std::exp(std::log(0.01f) * (1 - parameterValue) +
                                  std::log(1.f)   *  parameterValue);
    return parameterValue;
}

void WarpTestGui::refresh()
{
    PointType pointType = (PointType) m_pointTypeIndexCombo;
    WarpType warpType = (WarpType) m_warpMethodIndexCombo;
    float parameterValue = mapParameter(warpType, m_warpParam1Slider);
    float parameter2Value = mapParameter(warpType, m_warpParam2Slider);
    m_pointCount = (int) std::pow(2.f, 15 * m_pointCountSlider + 5);

    if (warpType == MicrofacetBRDF) {
        BSDF *ptr;
        float bsdfAngle = M_PI * (m_bsdfAngleSlider- 0.5f);
        std::tie(ptr, m_bRec) = WarpTest::create_microfacet_bsdf(
                parameterValue, parameter2Value, bsdfAngle);
        m_brdf.reset(ptr);
    }

    /* Generate the point positions */
    nori::MatrixXf positions, values;
    try {
        WarpTest tester(warpType, parameterValue, m_brdf.get(), m_bRec);
        tester.generatePoints(m_pointCount, pointType, positions, values);
    } catch (const NoriException &e) {
        m_warpMethodIndexCombo = 0;
        refresh();
        LOG(ERROR) << "An error occurred: " + std::string(e.what());
        return;
    }

    float value_scale = 0.f;
    for (int i=0; i<m_pointCount; ++i)
        value_scale = std::max(value_scale, values(0, i));
    value_scale = 1.f / value_scale;

    if (!m_visualizeBSDFCheckBox || warpType != MicrofacetBRDF)
        value_scale = 0.f;

    if (warpType != Square) {
        for (int i=0; i < m_pointCount; ++i) {
            if (values(0, i) == 0.0f) {
                positions.col(i) = nori::Vector3f::Constant(std::numeric_limits<float>::quiet_NaN());
                continue;
            }
            positions.col(i) =
                    ((value_scale == 0 ? 1.0f : (value_scale * values(0, i))) *
                     positions.col(i)) * 0.5f + nori::Vector3f(0.5f, 0.5f, 0.0f);
        }
    }

    /* Generate a color gradient */
    nori::MatrixXf colors(3, m_pointCount);
    float colScale = 1.f / m_pointCount;
    for (int i=0; i<m_pointCount; ++i)
        colors.col(i) << i*colScale, 1-i*colScale, 0;

    /* Upload points to GPU */
    // m_pPointShader->set_buffer("position", VariableType::Float32, {(size_t) m_pointCount, 3}, positions.data());
    // m_pPointShader->set_buffer("color", VariableType::Float32, {(size_t) m_pointCount, 3}, colors.data());

    /* Upload lines to GPU */
    if (m_visualizeGridCheckBox) {
        int gridRes = (int) (std::sqrt((float) m_pointCount) + 0.5f);
        int fineGridRes = 16*gridRes, idx = 0;
        m_lineCount = 4 * (gridRes+1) * (fineGridRes+1);
        positions.resize(3, m_lineCount);
        float coarseScale = 1.f / gridRes, fineScale = 1.f / fineGridRes;

        WarpTest tester(warpType, parameterValue, m_brdf.get(), m_bRec);
        for (int i=0; i<=gridRes; ++i) {
            for (int j=0; j<=fineGridRes; ++j) {
                auto pt = tester.warpPoint(Point2f(j * fineScale, i * coarseScale));
                positions.col(idx++) = value_scale == 0.f ? pt.first : (pt.first * pt.second * value_scale);
                pt = tester.warpPoint(Point2f((j+1) * fineScale, i * coarseScale));
                positions.col(idx++) = value_scale == 0.f ? pt.first : (pt.first * pt.second * value_scale);
                pt = tester.warpPoint(Point2f(i*coarseScale, j     * fineScale));
                positions.col(idx++) = value_scale == 0.f ? pt.first : (pt.first * pt.second * value_scale);
                pt = tester.warpPoint(Point2f(i*coarseScale, (j+1) * fineScale));
                positions.col(idx++) = value_scale == 0.f ? pt.first : (pt.first * pt.second * value_scale);
            }
        }
        if (warpType != Square) {
            for (int i=0; i<m_lineCount; ++i)
                positions.col(i) = positions.col(i) * 0.5f + nori::Vector3f(0.5f, 0.5f, 0.0f);
        }
       //  m_pGridShader->set_buffer("position", VariableType::Float32, {(size_t) m_lineCount, 3}, positions.data());
    }

    int ctr = 0;
    positions.resize(3, 106);
    for (int i=0; i<=50; ++i) {
        float angle1 = i * 2 * M_PI / 50;
        float angle2 = (i+1) * 2 * M_PI / 50;
        positions.col(ctr++) << std::cos(angle1)*.5f + 0.5f, std::sin(angle1)*.5f + 0.5f, 0.f;
        positions.col(ctr++) << std::cos(angle2)*.5f + 0.5f, std::sin(angle2)*.5f + 0.5f, 0.f;
    }
    positions.col(ctr++) << 0.5f, 0.5f, 0.f;
    positions.col(ctr++) << -m_bRec.wi.x() * 0.5f + 0.5f, -m_bRec.wi.y() * 0.5f + 0.5f, m_bRec.wi.z() * 0.5f;
    positions.col(ctr++) << 0.5f, 0.5f, 0.f;
    positions.col(ctr++) << m_bRec.wi.x() * 0.5f + 0.5f, m_bRec.wi.y() * 0.5f + 0.5f, m_bRec.wi.z() * 0.5f;
    // m_pArrowShader->set_buffer("position", VariableType::Float32, {106, 3}, positions.data());

    /* Update user interface */
    if (m_pointCount > 1000000) {
        m_pointCountStr = tfm::format("%.2f", m_pointCount * 1e-6f) + " M";
    } else if (m_pointCount > 1000) {
        m_pointCountStr = tfm::format("%.2f", m_pointCount * 1e-3f) + " K";
    } else {
        m_pointCountStr = tfm::format("%i", m_pointCount) + " ";
    }

    m_bsdfAngleStr = tfm::format("%.1f", m_bsdfAngleSlider * 180-90);
    m_bsdfAngleStr += utf8(0x00B0);
}

void WarpTestGui::runTest()
{

}

std::tuple<WarpType, float, float> parse_arguments(int argc, char **argv) {
    WarpType tp = WarpTypeCount;
    for (int i = 0; i < WarpTypeCount; ++i) {
        if (strcmp(kWarpTypeNames[i].c_str(), argv[1]) == 0)
            tp = WarpType(i);
    }
    if (tp >= WarpTypeCount)
        throw std::runtime_error("Invalid warp type!");

    float value = 0.f, value2 = 0.f;
    if (argc > 2)
        value = std::stof(argv[2]);
    if (argc > 3)
        value2 = std::stof(argv[3]);

    return { tp, value, value2 };
}


int main(int argc, char **argv) {
    google::InitGoogleLogging("SuperNori");
    google::SetStderrLogging(google::GLOG_INFO);
    if (argc <= 1) {
        // GUI mode
        std::unique_ptr<GuiBase> pGui = nullptr;
        pGui.reset(new WarpTestGui());
        pGui->draw();
        return 0;
    }

    // CLI mode
    WarpType warpType;
    float paramValue, param2Value;
    std::unique_ptr<BSDF> bsdf;
    auto bRec = BSDFQueryRecord(nori::Vector3f());
    std::tie(warpType, paramValue, param2Value) = parse_arguments(argc, argv);
    if (warpType == MicrofacetBRDF) {
        float bsdfAngle = M_PI * 0.f;
        BSDF *ptr;
        std::tie(ptr, bRec) = WarpTest::create_microfacet_bsdf(
            paramValue, param2Value, bsdfAngle);
        bsdf.reset(ptr);
    }

    std::string extra = "";
    if (param2Value > 0)
        extra = tfm::format(", second parameter value = %f", param2Value);
    std::cout << tfm::format(
        "Testing warp %s, parameter value = %f%s",
         kWarpTypeNames[int(warpType)], paramValue, extra
    ) << std::endl;
    WarpTest tester(warpType, paramValue, bsdf.get(), bRec);
    auto res = tester.run();
    if (res.first)
        return 0;

    std::cout << tfm::format("warptest failed: %s", res.second) << std::endl;
    google::ShutdownGoogleLogging();
    return 1;
}
