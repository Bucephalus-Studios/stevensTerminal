/**
 * @file ParticleFX.hpp
 * @brief Particle effects system for ncurses terminal graphics
 * @version 1.0
 * @date 2026-01-21
 *
 * Provides a particle system for creating visual effects like fireworks, sparks,
 * rain, confetti, etc. in ncurses windows.
 *
 * Usage:
 *   using namespace stevensTerminal::ParticleFX;
 *   ParticleSystem particles(myWindow);
 *   particles.spawnBurst({40, 15}, 50, Particle()
 *       .setPhysics(ParticlePresets::Firework())
 *       .setColorPair(brightYellow)
 *       .setLifetime(2.0f));
 *
 * FUTURE ENHANCEMENT - Particle-to-Content Layering:
 * ===================================================
 * Currently, particles ALWAYS render above window content. This is intentional for
 * celebration/UI effects (success bursts, confetti, etc.).
 *
 * If background particle effects are needed (snow behind text, embers behind dialogue),
 * implement a ParticleLayer enum and modify spawnBurst():
 *
 *   enum class ParticleLayer {
 *       BehindContent,   // Render before restoring window snapshot
 *       AboveContent     // Render after restoring snapshot (current behavior)
 *   };
 *
 *   // Add to spawnBurst parameters:
 *   ParticleLayer renderLayer = ParticleLayer::AboveContent
 *
 *   // In animation loop:
 *   if (renderLayer == ParticleLayer::BehindContent) {
 *       effect.render();                           // Particles first
 *       ParticleWindowRegistry::restoreWindow(window);  // Content on top
 *   } else {
 *       ParticleWindowRegistry::restoreWindow(window);  // Content first
 *       effect.render();                           // Particles on top (current)
 *   }
 *
 * This provides background atmospheric effects while keeping implementation simple.
 * Per-particle layer mixing would require multiple snapshot layers (more complex).
 */

#include <vector>
#include <tuple>
#include <cmath>
#include <numbers>
#include <random>
#include <chrono>
#include <thread>
#include <algorithm>

// Include ParticleFX components
#include "Vec2.hpp"
#include "ParticlePhysics.hpp"
#include "ParticleWindowRegistry.hpp"

namespace stevensTerminal {
namespace ParticleFX {

/**
 * @brief A single particle in the particle system
 */
class Particle {
private:
    Vec2 position;
    Vec2 velocity;
    ParticlePhysics physics;

    float lifetime;      // Total lifetime in seconds
    float age;           // Current age in seconds

    int colorPair;       // ncurses color pair to use
    char character;      // Character to display (or ' ' for background-only)

    bool modifyBg;       // Should particle change background color?
    bool modifyFg;       // Should particle change foreground color?
    bool modifyChar;     // Should particle change the character?

    int layer;           // Z-order for layering (higher = in front)

public:
    Particle()
        : position(0, 0)
        , velocity(0, 0)
        , physics()
        , lifetime(1.0f)
        , age(0.0f)
        , colorPair(0)
        , character(' ')
        , modifyBg(true)
        , modifyFg(false)
        , modifyChar(false)
        , layer(0)
    {}

    // Setters
    void setPosition(float x, float y) { position = Vec2(x, y); }
    void setPosition(Vec2 pos) { position = pos; }
    void setVelocity(float vx, float vy) { velocity = Vec2(vx, vy); }
    void setVelocity(Vec2 vel) { velocity = vel; }
    void setPhysics(ParticlePhysics p) { physics = p; }
    void setLifetime(float t) { lifetime = t; }

    void setColorPair(int cp) {
        // Validate color pair number - must be in valid range
        if (cp < 0 || cp >= COLOR_PAIRS) {
            std::cerr << "WARNING: Invalid color pair " << cp
                      << " (must be 0-" << (COLOR_PAIRS-1) << "). Using default 0." << std::endl;
            colorPair = 0;  // Fallback to default
        } else {
            colorPair = cp;
        }
    }

    void setCharacter(char c) { character = c; }
    void setModifyBg(bool m) { modifyBg = m; }
    void setModifyFg(bool m) { modifyFg = m; }
    void setModifyChar(bool m) { modifyChar = m; }
    void setLayer(int l) { layer = l; }

    // Getters
    Vec2 getPosition() const { return position; }
    Vec2 getVelocity() const { return velocity; }
    std::tuple<int, int> getRenderPosition() const {
        return {static_cast<int>(std::round(position.x)),
                static_cast<int>(std::round(position.y))};
    }
    float getAge() const { return age; }
    float getLifetime() const { return lifetime; }
    int getColorPair() const { return colorPair; }
    char getCharacter() const { return character; }
    bool shouldModifyBg() const { return modifyBg; }
    bool shouldModifyFg() const { return modifyFg; }
    bool shouldModifyChar() const { return modifyChar; }
    int getLayer() const { return layer; }

    bool isAlive() const { return age < lifetime; }
    float getLifePercent() const { return lifetime > 0.0f ? age / lifetime : 1.0f; }

    /**
     * @brief Update particle physics
     * @param deltaTime Time elapsed since last update (in seconds)
     * @param rng Random number generator for turbulence
     */
    void update(float deltaTime, std::mt19937& rng) {
        age += deltaTime;

        // Apply gravity in the specified direction
        velocity += physics.gravityDirection * physics.gravity * deltaTime;

        // Apply drag
        velocity = velocity * (1.0f - physics.drag);

        // Apply turbulence
        if (physics.turbulence > 0.0f) {
            std::uniform_real_distribution<float> dist(-physics.turbulence, physics.turbulence);
            velocity.x += dist(rng);
            velocity.y += dist(rng);
        }

        // Update position
        position += velocity * deltaTime;
    }

    /**
     * @brief Handle collision with window boundaries
     * @param minX Minimum X boundary
     * @param maxX Maximum X boundary
     * @param minY Minimum Y boundary
     * @param maxY Maximum Y boundary
     */
    void handleBoundaries(int minX, int maxX, int minY, int maxY) {
        // Horizontal boundaries
        if (position.x < minX) {
            position.x = minX;
            velocity.x = std::abs(velocity.x) * physics.bounce;
        }
        if (position.x >= maxX) {
            position.x = maxX - 1;
            velocity.x = -std::abs(velocity.x) * physics.bounce;
        }

        // Vertical boundaries
        if (position.y < minY) {
            position.y = minY;
            velocity.y = std::abs(velocity.y) * physics.bounce;
        }
        if (position.y >= maxY) {
            position.y = maxY - 1;
            velocity.y = -std::abs(velocity.y) * physics.bounce;
        }
    }
};

/**
 * @brief High-level particle effect system
 *
 * Manages particle spawn points, creation, updating, and rendering.
 * Configure spawn points, then call spawn() to create particles.
 */
class ParticleEffect {
private:
    std::vector<Particle> particles;
    WINDOW* targetWindow;
    std::mt19937 rng;

    int minX, maxX, minY, maxY;  // Window boundaries

    // Spawn configuration
    std::vector<Vec2> spawnPoints;
    int particlesPerPoint;
    float minSpeed;
    float maxSpeed;

public:
    ParticleEffect(WINDOW* window = nullptr)
        : targetWindow(window)
        , rng(std::random_device{}())
        , minX(0), maxX(80), minY(0), maxY(24)
        , particlesPerPoint(3)
        , minSpeed(5.0f)
        , maxSpeed(12.0f)
    {
        if (targetWindow) {
            updateBoundaries();
        }
    }

    /**
     * @brief Set the target window for particle rendering
     */
    void setWindow(WINDOW* window) {
        targetWindow = window;
        updateBoundaries();
    }

    /**
     * @brief Update window boundaries from current window size
     */
    void updateBoundaries() {
        if (targetWindow) {
            getmaxyx(targetWindow, maxY, maxX);
            minX = 0;
            minY = 0;
        }
    }

    /**
     * @brief Update all particles
     * @param deltaTime Time elapsed since last update (in seconds)
     */
    void update(float deltaTime) {
        // Update all particles
        for (auto& particle : particles) {
            particle.update(deltaTime, rng);
            particle.handleBoundaries(minX, maxX, minY, maxY);
        }

        // Remove dead particles
        particles.erase(
            std::remove_if(particles.begin(), particles.end(),
                [](const Particle& p) { return !p.isAlive(); }),
            particles.end()
        );
    }

    /**
     * @brief Render all particles to the target window and stage it for screen update
     *
     * This function draws all particles to the window and then calls wnoutrefresh()
     * to stage the window for the next doupdate(). This ensures particles are properly
     * included in the screen update.
     */
    void render() {
        if (!targetWindow) return;

        // Sort by layer (lower layers drawn first)
        std::sort(particles.begin(), particles.end(),
            [](const Particle& a, const Particle& b) {
                return a.getLayer() < b.getLayer();
            });

        for (const auto& particle : particles) {
            auto [x, y] = particle.getRenderPosition();

            // Bounds check
            if (x < minX || x >= maxX || y < minY || y >= maxY) continue;

            // Read what's currently at this position
            chtype existingCell = mvwinch(targetWindow, y, x);
            char currentChar = existingCell & A_CHARTEXT;

            // Determine what to modify based on particle settings
            if (particle.shouldModifyChar()) {
                // Particle wants to replace the character completely
                wattron(targetWindow, COLOR_PAIR(particle.getColorPair()));
                mvwaddch(targetWindow, y, x, particle.getCharacter());
                wattroff(targetWindow, COLOR_PAIR(particle.getColorPair()));
            }
            else if (particle.shouldModifyBg()) {
                // Particle wants to change background color only
                // Preserve the existing foreground color and character, but use particle's background

                int existingPair = PAIR_NUMBER(existingCell);
                short existingFg = stevensTerminal::Colors::extractForegroundColor(existingPair);
                short particleBg = stevensTerminal::Colors::extractBackgroundColor(particle.getColorPair());

                // Combine existing fg with particle bg to preserve text visibility
                int combinedPair = stevensTerminal::Colors::lookupColorPair(existingFg, particleBg);

                wattron(targetWindow, COLOR_PAIR(combinedPair));
                mvwaddch(targetWindow, y, x, currentChar);
                wattroff(targetWindow, COLOR_PAIR(combinedPair));
            }
        }

        // Stage window for next doupdate() - must be called AFTER drawing particles
        wnoutrefresh(targetWindow);
    }

    // ========== Spawn Point Configuration ==========

    /**
     * @brief Add a single spawn point
     */
    void addSpawnPoint(float x, float y) {
        spawnPoints.push_back(Vec2(x, y));
    }

    /**
     * @brief Add spawn points from each non-space character in text
     * @param text The text std::string to create spawn points from
     * @param startY Row where the text appears
     * @param startX Column where the text starts
     */
    void addSpawnPointsFromText(const std::string& text, int startY, int startX) {
        int currentX = startX;
        for (char c : text) {
            if (c == '\n') continue;  // Skip newlines
            if (c == ' ') {
                currentX++;  // Advance position but don't add spawn point
                continue;
            }
            spawnPoints.push_back(Vec2(currentX, startY));
            currentX++;
        }
    }

    /**
     * @brief Clear all spawn points
     */
    void clearSpawnPoints() {
        spawnPoints.clear();
    }

    /**
     * @brief Set how many particles spawn from each configured spawn point
     */
    void setParticlesPerPoint(int count) {
        particlesPerPoint = count;
    }

    /**
     * @brief Set particle speed range for spawning
     */
    void setSpeedRange(float min, float max) {
        minSpeed = min;
        maxSpeed = max;
    }

    /**
     * @brief Get number of configured spawn points
     */
    size_t getSpawnPointCount() const {
        return spawnPoints.size();
    }

    /**
     * @brief Spawn particles from all configured spawn points
     *
     * Creates a burst effect from each spawn point added via addSpawnPoint()
     * or addSpawnPointsFromText().
     */
    void spawn(const Particle& prototype) {
        std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * std::numbers::pi_v<float>);
        std::uniform_real_distribution<float> speedDist(minSpeed, maxSpeed);

        for (const Vec2& origin : spawnPoints) {
            for (int i = 0; i < particlesPerPoint; ++i) {
                float angle = angleDist(rng);
                float speed = speedDist(rng);

                Particle p = prototype;
                p.setPosition(origin);
                p.setVelocity(std::cos(angle) * speed, std::sin(angle) * speed);

                particles.push_back(p);
            }
        }
    }

    // ========== Legacy Spawn Methods (for compatibility) ==========

    /**
     * @brief Spawn particles in a burst pattern from an origin point
     * @param origin Center point of the burst
     * @param count Number of particles to spawn
     * @param prototype Particle template to copy
     * @param minSpeed Minimum initial speed
     * @param maxSpeed Maximum initial speed
     */
    void spawnBurst(Vec2 origin, int count, const Particle& prototype,
                    float minSpeed = 5.0f, float maxSpeed = 15.0f) {
        std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * std::numbers::pi_v<float>);
        std::uniform_real_distribution<float> speedDist(minSpeed, maxSpeed);

        for (int i = 0; i < count; ++i) {
            float angle = angleDist(rng);
            float speed = speedDist(rng);

            Particle p = prototype;
            p.setPosition(origin);
            p.setVelocity(std::cos(angle) * speed, std::sin(angle) * speed);

            particles.push_back(p);
        }
    }

    /**
     * @brief Spawn particles in a fountain pattern (upward spray)
     * @param origin Base point of the fountain
     * @param count Number of particles to spawn
     * @param prototype Particle template to copy
     * @param angle Spray angle in radians (0 = straight up)
     * @param spread Angle spread (how wide the spray is)
     */
    void spawnFountain(Vec2 origin, int count, const Particle& prototype,
                       float angle = -std::numbers::pi_v<float>/2, float spread = std::numbers::pi_v<float>/6,
                       float minSpeed = 8.0f, float maxSpeed = 15.0f) {
        std::uniform_real_distribution<float> angleDist(angle - spread/2, angle + spread/2);
        std::uniform_real_distribution<float> speedDist(minSpeed, maxSpeed);

        for (int i = 0; i < count; ++i) {
            float particleAngle = angleDist(rng);
            float speed = speedDist(rng);

            Particle p = prototype;
            p.setPosition(origin);
            p.setVelocity(std::cos(particleAngle) * speed, std::sin(particleAngle) * speed);

            particles.push_back(p);
        }
    }

    /**
     * @brief Spawn particles in a rain pattern (falling from top)
     * @param minXPos Left boundary for rain
     * @param maxXPos Right boundary for rain
     * @param count Number of particles to spawn
     * @param prototype Particle template to copy
     */
    void spawnRain(int minXPos, int maxXPos, int count, const Particle& prototype) {
        std::uniform_int_distribution<int> xDist(minXPos, maxXPos);
        std::uniform_real_distribution<float> speedDist(2.0f, 5.0f);

        for (int i = 0; i < count; ++i) {
            int x = xDist(rng);
            float speed = speedDist(rng);

            Particle p = prototype;
            p.setPosition(x, minY);
            p.setVelocity(0, speed);

            particles.push_back(p);
        }
    }

    /**
     * @brief Spawn particles along a line
     * @param start Starting point
     * @param end Ending point
     * @param count Number of particles to spawn
     * @param prototype Particle template to copy
     */
    void spawnLine(Vec2 start, Vec2 end, int count, const Particle& prototype) {
        for (int i = 0; i < count; ++i) {
            float t = static_cast<float>(i) / (count - 1);
            Vec2 pos = start + (end - start) * t;

            Particle p = prototype;
            p.setPosition(pos);

            particles.push_back(p);
        }
    }

    /**
     * @brief Add a single particle
     */
    void addParticle(const Particle& particle) {
        particles.push_back(particle);
    }

    /**
     * @brief Get current particle count
     */
    size_t getParticleCount() const {
        return particles.size();
    }

    /**
     * @brief Clear all particles
     */
    void clear() {
        particles.clear();
    }
};

/**
 * @brief Spawn a burst particle effect and animate it for a specified duration
 *
 * This is a high-level helper function that creates, spawns, animates, and cleans up
 * a particle burst effect. Useful for quick one-off effects like success celebrations.
 *
 * @param window The window to render particles on
 * @param spawnPoints Vector of positions where particles will burst from
 * @param prototype The particle template (physics, color, lifetime, etc.)
 * @param duration How long to animate the effect in seconds (default: 0.5s)
 * @param particlesPerPoint How many particles spawn from each point (default: 3)
 * @param minSpeed Minimum particle speed (default: 6.0)
 * @param maxSpeed Maximum particle speed (default: 12.0)
 *
 * Example usage:
 * @code
 * std::vector<Vec2> points = {{10, 5}, {12, 5}, {14, 5}};
 * Particle p;
 * p.setPhysics(ParticlePresets::Firework());
 * p.setColorPair(greenColorPair);
 * p.setLifetime(0.5f);
 * p.setModifyBg(true);
 * spawnBurst(myWindow, points, p);
 * @endcode
 */
inline void spawnBurst(WINDOW* window,
                       const std::vector<Vec2>& spawnPoints,
                       const Particle& prototype,
                       float duration = 0.5f,
                       int particlesPerPoint = 3,
                       float minSpeed = 6.0f,
                       float maxSpeed = 12.0f)
{
    // Register window (creates snapshot if first effect on this window)
    ParticleWindowRegistry::registerWindow(window);

    // NOTE: Particles currently render above all window content.
    // See file header comment for future layering enhancement design.

    // Create and configure the effect
    ParticleEffect effect(window);
    for (const Vec2& point : spawnPoints) {
        effect.addSpawnPoint(point.x, point.y);
    }
    effect.setParticlesPerPoint(particlesPerPoint);
    effect.setSpeedRange(minSpeed, maxSpeed);

    // Spawn particles
    effect.spawn(prototype);

    // Animate for specified duration
    auto startTime = std::chrono::high_resolution_clock::now();

    while (true) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = currentTime - startTime;
        if (elapsed.count() >= duration) break;

        float deltaTime = 0.016f;  // ~60 FPS
        effect.update(deltaTime);

        // Restore original window content before rendering particles (prevents trails)
        ParticleWindowRegistry::restoreWindow(window);

        // Render particles (render() now handles wnoutrefresh internally)
        effect.render();
        doupdate();  // Push virtual screen to physical screen

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    // Unregister window (restores and cleans up snapshot if last effect)
    ParticleWindowRegistry::unregisterWindow(window);
}

} // namespace ParticleFX
} // namespace stevensTerminal
