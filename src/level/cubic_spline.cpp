#include "level/cubic_spline.hpp"

#include <cmath>
#include <algorithm>

namespace {
    // Helper: Evaluate cubic polynomial at t
    inline float cubicEval(float a, float b, float c, float d, float t) {
        return a + b * t + c * t * t + d * t * t * t;
    }

    // Solve tridiagonal matrix for natural cubic spline
    void solveTridiagonal(int n, const std::vector<float>& a,
                          const std::vector<float>& b,
                          const std::vector<float>& c,
                          const std::vector<float>& d,
                          std::vector<float>& x) {
        std::vector<float> c_prime(n);
        std::vector<float> d_prime(n);

        c_prime[0] = c[0] / b[0];
        d_prime[0] = d[0] / b[0];

        for (int i = 1; i < n; ++i) {
            float m = b[i] - a[i] * c_prime[i - 1];
            c_prime[i] = (i < n - 1) ? c[i] / m : 0.0f;
            d_prime[i] = (d[i] - a[i] * d_prime[i - 1]) / m;
        }

        x[n - 1] = d_prime[n - 1];
        for (int i = n - 2; i >= 0; --i) {
            x[i] = d_prime[i] - c_prime[i] * x[i + 1];
        }
    }
}

std::vector<b2Vec2> CubicSpline::interpolate(
    const std::vector<b2Vec2>& control_points,
    float sample_interval) {

    if (control_points.size() < 2) {
        return control_points;
    }
    if (control_points.size() == 2) {
        // Linear interpolation for 2 points
        std::vector<b2Vec2> result;
        b2Vec2 a = control_points[0];
        b2Vec2 b = control_points[1];
        float dist = sqrtf((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
        int samples = std::max(2, static_cast<int>(dist / sample_interval));
        for (int i = 0; i < samples; ++i) {
            float t = static_cast<float>(i) / (samples - 1);
            result.push_back({a.x + t * (b.x - a.x), a.y + t * (b.y - a.y)});
        }
        return result;
    }

    int n = static_cast<int>(control_points.size());

    // Natural cubic spline (second derivatives at endpoints = 0)
    std::vector<float> h(n - 1);
    for (int i = 0; i < n - 1; ++i) {
        h[i] = control_points[i + 1].x - control_points[i].x;
    }

    // Solve for second derivatives (M values) in Y direction
    std::vector<float> alpha(n);
    for (int i = 1; i < n - 1; ++i) {
        alpha[i] = (3.0f / h[i]) * (control_points[i + 1].y - control_points[i].y) -
                   (3.0f / h[i - 1]) * (control_points[i].y - control_points[i - 1].y);
    }

    std::vector<float> a_diag(n), b_diag(n), c_diag(n), d_diag(n), M(n);
    b_diag[0] = 1.0f;
    c_diag[0] = 0.0f;
    d_diag[0] = 0.0f;

    for (int i = 1; i < n - 1; ++i) {
        a_diag[i] = h[i - 1];
        b_diag[i] = 2.0f * (h[i - 1] + h[i]);
        c_diag[i] = h[i];
        d_diag[i] = alpha[i];
    }

    a_diag[n - 1] = 0.0f;
    b_diag[n - 1] = 1.0f;
    d_diag[n - 1] = 0.0f;

    solveTridiagonal(n, a_diag, b_diag, c_diag, d_diag, M);

    // Sample the spline
    std::vector<b2Vec2> result;
    for (int i = 0; i < n - 1; ++i) {
        float x0 = control_points[i].x;
        float x1 = control_points[i + 1].x;
        float segment_length = x1 - x0;
        int samples = std::max(2, static_cast<int>(segment_length / sample_interval));

        for (int j = 0; j < samples; ++j) {
            float t = static_cast<float>(j) / (samples - 1);
            float x = x0 + t * segment_length;

            // Cubic spline formula
            float A = (x1 - x) / h[i];
            float B = (x - x0) / h[i];
            float C = (A * A * A - A) * h[i] * h[i] / 6.0f;
            float D = (B * B * B - B) * h[i] * h[i] / 6.0f;

            float y = A * control_points[i].y + B * control_points[i + 1].y +
                      C * M[i] + D * M[i + 1];

            result.push_back({x, y});
        }
    }

    return result;
}

std::vector<b2Vec2> CubicSpline::interpolateClosed(
    const std::vector<b2Vec2>& control_points,
    float sample_interval) {

    if (control_points.size() < 3) {
        return control_points;
    }

    // For closed spline, duplicate first point at end
    std::vector<b2Vec2> closed_points = control_points;
    closed_points.push_back(control_points[0]);

    // Simple parameterization by angle for closed curves
    std::vector<b2Vec2> result;
    int n = static_cast<int>(control_points.size());

    for (int i = 0; i < n; ++i) {
        b2Vec2 p0 = control_points[(i - 1 + n) % n];
        b2Vec2 p1 = control_points[i];
        b2Vec2 p2 = control_points[(i + 1) % n];
        b2Vec2 p3 = control_points[(i + 2) % n];

        int samples = std::max(2, static_cast<int>(1.0f / sample_interval));
        for (int j = 0; j < samples; ++j) {
            float t = static_cast<float>(j) / samples;
            float t2 = t * t;
            float t3 = t2 * t;

            // Catmull-Rom spline
            float x = 0.5f * ((2.0f * p1.x) +
                             (-p0.x + p2.x) * t +
                             (2.0f * p0.x - 5.0f * p1.x + 4.0f * p2.x - p3.x) * t2 +
                             (-p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x) * t3);

            float y = 0.5f * ((2.0f * p1.y) +
                             (-p0.y + p2.y) * t +
                             (2.0f * p0.y - 5.0f * p1.y + 4.0f * p2.y - p3.y) * t2 +
                             (-p0.y + 3.0f * p1.y - 3.0f * p2.y + p3.y) * t3);

            result.push_back({x, y});
        }
    }

    return result;
}
