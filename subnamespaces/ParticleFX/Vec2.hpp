#pragma once

#include <cmath>

namespace stevensTerminal {
namespace ParticleFX {

/**
 * @brief Simple 2D vector for particle positions and velocities
 */
struct Vec2 {
    float x, y;

    Vec2() : x(0), y(0) {}
    Vec2(float x_, float y_) : x(x_), y(y_) {}

    Vec2 operator+(const Vec2& other) const { return {x + other.x, y + other.y}; }
    Vec2 operator-(const Vec2& other) const { return {x - other.x, y - other.y}; }
    Vec2 operator*(float scalar) const { return {x * scalar, y * scalar}; }
    Vec2& operator+=(const Vec2& other) { x += other.x; y += other.y; return *this; }

    float length() const { return std::sqrt(x * x + y * y); }
    Vec2 normalized() const {
        float len = length();
        return len > 0.0f ? Vec2(x / len, y / len) : Vec2(0, 0);
    }
};

} // namespace ParticleFX
} // namespace stevensTerminal
