#pragma once

#include "Vec2.hpp"

namespace stevensTerminal {
namespace ParticleFX {

/**
 * @brief Physics properties for particle behavior
 */
struct ParticlePhysics {
    float gravity;          // Gravity strength
    float drag;             // Air resistance (0.0 = none, 1.0 = full stop)
    float bounce;           // Bounciness on collision (0.0 = stick, 1.0 = perfect bounce)
    float turbulence;       // Random movement intensity
    Vec2 gravityDirection;  // Direction of gravity (default: {0, 1} = downward)

    ParticlePhysics()
        : gravity(0.0f), drag(0.0f), bounce(0.0f), turbulence(0.0f), gravityDirection(0, 1) {}

    ParticlePhysics(float g, float d, float b, float t)
        : gravity(g), drag(d), bounce(b), turbulence(t), gravityDirection(0, 1) {}

    ParticlePhysics(float g, float d, float b, float t, Vec2 gravDir)
        : gravity(g), drag(d), bounce(b), turbulence(t), gravityDirection(gravDir) {}
};

/**
 * @brief Predefined physics presets for common particle types
 */
namespace ParticlePresets {
    inline ParticlePhysics Firework() {
        return {3.0f, 0.02f, 0.0f, 0.01f};  // Strong gravity (falls like stones), low drag, no bounce
    }

    inline ParticlePhysics Ember() {
        return {-0.1f, 0.1f, 0.0f, 0.05f};  // Floats upward, high drag, turbulent
    }

    inline ParticlePhysics Stone() {
        return {1.0f, 0.0f, 0.3f, 0.0f};    // Heavy gravity, bounces
    }

    inline ParticlePhysics Confetti() {
        return {0.3f, 0.15f, 0.0f, 0.1f};   // Light gravity, lots of drag and turbulence
    }

    inline ParticlePhysics Bubble() {
        return {-0.2f, 0.08f, 0.8f, 0.03f}; // Floats up, bouncy
    }

    inline ParticlePhysics Snow() {
        return {0.1f, 0.2f, 0.0f, 0.08f};   // Slow fall, drifty
    }
}

} // namespace ParticleFX
} // namespace stevensTerminal

