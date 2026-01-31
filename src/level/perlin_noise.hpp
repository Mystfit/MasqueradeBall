#pragma once

#include <vector>
#include <cmath>
#include <random>
#include <algorithm>

class PerlinNoise {
public:
    explicit PerlinNoise(unsigned int seed = std::random_device{}()) {
        // Initialize permutation table with values 0-255
        p.resize(256);
        std::iota(p.begin(), p.end(), 0);

        // Shuffle using the seed
        std::mt19937 rng(seed);
        std::shuffle(p.begin(), p.end(), rng);

        // Duplicate the permutation table
        p.insert(p.end(), p.begin(), p.end());
    }

    // Get 1D Perlin noise value at x
    // frequency: controls how quickly the noise changes (higher = more variation)
    // Returns value in range [-1, 1]
    float noise(float x, float frequency = 1.0f) {
        x *= frequency;

        // Find unit grid cell containing point
        int X = static_cast<int>(std::floor(x)) & 255;

        // Get relative x coordinate within cell
        x -= std::floor(x);

        // Compute fade curve for x
        float u = fade(x);

        // Hash coordinates of the 2 grid corners
        int a = p[X];
        int b = p[X + 1];

        // Blend results from the two corners
        return lerp(u, grad(a, x), grad(b, x - 1.0f));
    }

    // Multi-octave Perlin noise for more natural terrain
    // Returns value in range [-1, 1]
    float octaveNoise(float x, float frequency = 1.0f, int octaves = 2,
                      float persistence = 0.5f, float lacunarity = 2.0f) {
        float total = 0.0f;
        float amplitude = 1.0f;
        float max_amplitude = 0.0f;
        float freq = frequency;

        for (int i = 0; i < octaves; ++i) {
            total += noise(x, freq) * amplitude;
            max_amplitude += amplitude;
            amplitude *= persistence;
            freq *= lacunarity;
        }

        return total / max_amplitude;
    }

private:
    std::vector<int> p;

    // Fade function: 6t^5 - 15t^4 + 10t^3
    static float fade(float t) {
        return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
    }

    // Linear interpolation
    static float lerp(float t, float a, float b) {
        return a + t * (b - a);
    }

    // Gradient function for 1D (just return pseudorandom slope)
    float grad(int hash, float x) {
        // Use the hash to determine slope direction (-1 or +1)
        return (hash & 1) ? -x : x;
    }
};
