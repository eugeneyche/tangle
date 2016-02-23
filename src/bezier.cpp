#include "bezier.hpp"
#include <iostream>

#define ABS(x) ((x >= 0.f) ? x : -x)

static const float DIST_THRESHOLD = 1e-3f;
static const float RISK_THRESHOLD = 1e-4f;

static float FloatAbs(float x)
{
    return (x >= 0.f) ? x : -x;
}

static void ImplGenBezierCurve(
        const std::vector<glm::vec2>& control_points, 
        std::vector<glm::vec2>& r_path,
        std::vector<GLfloat>& r_alpha,
        float begin_alpha, 
        float end_alpha,
        bool include_last = true)
{
    glm::vec2 p0 = control_points[0];
    glm::vec2 p1 = control_points[1];
    glm::vec2 p2 = control_points[2];
    glm::vec2 p3 = control_points[3];
    bool within_threshold = true;
    glm::vec2 dir = glm::normalize(p3 - p0);
    glm::vec2 ortho {-dir.y, dir.x};
    float risk = FloatAbs(dot(ortho, p1 - p0)) + FloatAbs(dot(ortho, p2 - p3));
    if (glm::length(p3 - p1) < DIST_THRESHOLD)
        risk = 0.f;
    if (risk <= RISK_THRESHOLD)
    {
        r_path.push_back(p0);
        r_alpha.push_back(begin_alpha);
        if (include_last)
        {
            r_path.push_back(p3);
            r_alpha.push_back(end_alpha);
        }
        return;
    }
    glm::vec2 p01 = 0.5f * (p0 + p1);
    glm::vec2 p12 = 0.5f * (p1 + p2);
    glm::vec2 p23 = 0.5f * (p2 + p3);
    glm::vec2 p012 = 0.5f * (p01 + p12);
    glm::vec2 p123 = 0.5f * (p12 + p23);
    glm::vec2 p0123 = 0.5f * (p012 + p123);
    ImplGenBezierCurve(
        {p0, p01, p012, p0123},
        r_path,
        r_alpha,
        begin_alpha, (begin_alpha + end_alpha) * 0.5f,
        false);
    ImplGenBezierCurve(
        {p0123, p123, p23, p3},
        r_path,
        r_alpha,
        (begin_alpha + end_alpha) * 0.5f, end_alpha,
        true);
}

void GenBezierCurve(
        const std::vector<glm::vec2>& control_points, 
        std::vector<glm::vec2>& r_path,
        std::vector<GLfloat>& r_alpha)
{
    ImplGenBezierCurve(control_points, r_path, r_alpha, 0.f, 1.f);
}
