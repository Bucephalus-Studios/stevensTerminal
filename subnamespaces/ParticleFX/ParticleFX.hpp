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
 *   ParticleEmitterSettings settings = ParticleEmitterPresets::Sparks();
 *   settings.colors = ParticleColors::Rainbow();
 *   settings.colorDistribution = ColorDistribution::SequencePerSpawnPoint;
 *   spawnBurst(myWindow, {{40, 15}, {41, 15}}, settings, { .duration = 1.0f });
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

#include <cstdint>
#include <string>
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
#include "ParticleColors.hpp"
#include "ParticleWindowRegistry.hpp"
#include <RouletteWheel.hpp>

namespace stevensTerminal {
namespace ParticleFX {

/**
 * @brief What an emitter's particles start with -- everything a ParticleEffect needs to create
 *        them, separate from the live particles themselves. Every field has a working default, so
 *        designated initializers only name what differs:
 * @code
 * ParticleEmitterSettings sparks = { .colors = {"bright-green_on_bright-green"}, .modifyChar = true };
 * ParticleEmitterSettings rainbow = { .colors = ParticleColors::Rainbow(),
 *                                     .colorDistribution = ColorDistribution::SequencePerSpawnPoint };
 * @endcode
 */
struct ParticleEmitterSettings
{
    ParticlePhysics physics = ParticlePresets::Firework();
    std::vector<std::string> colors;     // "foreground_on_background" pair names handed out at spawn; empty = default pair (0)
    std::vector<float> colorWeights;     // ColorDistribution::Random only, parallel to colors; empty = even
    ColorDistribution colorDistribution = ColorDistribution::Random;
    float lifetime = 1.0f;               // Total lifetime in seconds
    char character = ' ';                // Character to display (or ' ' for background-only)
    bool modifyBg = true;                // Should particle change background color?
    bool modifyFg = false;               // Should particle change foreground color? (not yet read by render())
    bool modifyChar = false;             // Should particle change the character?
    int layer = 0;                       // Z-order for layering (higher = in front)
};

/**
 * @brief Ready-made emitter settings for common looks, alongside ParticlePresets' physics.
 *        Start from one and set what differs (usually just colors).
 */
namespace ParticleEmitterPresets {
    /**
     * @brief Solid blocks of color that burst outward and fall like fireworks -- the cells they
     *        cross are blanked to their color while they pass.
     */
    inline ParticleEmitterSettings Sparks() {
        return { .physics = ParticlePresets::Firework(), .character = ' ', .modifyChar = true };
    }
}

/**
 * @brief A single live particle in the particle system. Created by a ParticleEffect from its
 *        ParticleEmitterSettings, with one resolved color pair.
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

    /**
     * @brief A particle starting from an emitter's `settings`, at rest at the origin, with the
     *        color pair the emitter picked for it from settings.colors.
     */
    Particle(const ParticleEmitterSettings& settings, int colorPairParam)
        : position(0, 0)
        , velocity(0, 0)
        , physics(settings.physics)
        , lifetime(settings.lifetime)
        , age(0.0f)
        , colorPair(0)
        , character(settings.character)
        , modifyBg(settings.modifyBg)
        , modifyFg(settings.modifyFg)
        , modifyChar(settings.modifyChar)
        , layer(settings.layer)
    {
        setColorPair(colorPairParam);
    }

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

    /**
     * @brief Set the color pair by "foreground_on_background" name (e.g. "bright-green_on_bright-green").
     *        Unregistered names fall back to the default pair (0).
     */
    void setColorPair(const std::string& colorPairName) {
        auto pairIt = stevensTerminal::Colors::curses_colorPairs.find(colorPairName);
        setColorPair(pairIt != stevensTerminal::Colors::curses_colorPairs.end() ? pairIt->second : 0);
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
 * @brief Hands out an emitter's colors to the particles one spawn call creates, per its
 *        ColorDistribution. Resolves the pair names to pair numbers once, up front, so the
 *        per-particle pick is just an index or one weighted draw.
 */
class ColorPicker {
private:
    std::vector<int> colorPairs;                    // Resolved pair numbers; unregistered names -> default pair (0)
    ColorDistribution distribution;
    RouletteWheel<size_t, float> randomIndexWheel;  // Indices into colorPairs, for ColorDistribution::Random

public:
    explicit ColorPicker(const ParticleEmitterSettings& settings)
        : distribution(settings.colorDistribution)
    {
        for (const std::string& colorPairName : settings.colors) {
            auto pairIt = stevensTerminal::Colors::curses_colorPairs.find(colorPairName);
            colorPairs.push_back(pairIt != stevensTerminal::Colors::curses_colorPairs.end() ? pairIt->second : 0);
        }
        if (colorPairs.empty()) {
            colorPairs.push_back(0);
        }

        // Even weights unless settings give a positive one per color
        const bool hasValidWeights = settings.colorWeights.size() == colorPairs.size() &&
                                     std::all_of(settings.colorWeights.begin(), settings.colorWeights.end(),
                                                 [](float weight) { return weight > 0.0f; });
        std::vector<std::tuple<size_t, float>> indexWeights;
        for (size_t i = 0; i < colorPairs.size(); ++i) {
            indexWeights.emplace_back(i, hasValidWeights ? settings.colorWeights[i] : 1.0f);
        }
        randomIndexWheel = RouletteWheel<size_t, float>(indexWeights);
    }

    /**
     * @brief The color pair for the next particle.
     * @param spawnPointIndex Which spawn point it comes from (0-based), for SequencePerSpawnPoint
     * @param particleIndex How many particles this spawn has created before it, for SequencePerParticle
     */
    int pick(size_t spawnPointIndex, size_t particleIndex) const {
        if (colorPairs.size() == 1) {
            return colorPairs.front();
        }
        switch (distribution) {
            case ColorDistribution::Random:
                return colorPairs[randomIndexWheel.select()];
            case ColorDistribution::SequencePerParticle:
                return colorPairs[particleIndex % colorPairs.size()];
            case ColorDistribution::SequencePerSpawnPoint:
                return colorPairs[spawnPointIndex % colorPairs.size()];
        }
        return colorPairs.front();
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
     * or addSpawnPointsFromText(), each particle starting from `settings`.
     */
    void spawn(const ParticleEmitterSettings& settings) {
        std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * std::numbers::pi_v<float>);
        std::uniform_real_distribution<float> speedDist(minSpeed, maxSpeed);

        const ColorPicker colorPicker(settings);
        size_t spawnPointIndex = 0;   // Which spawn point we're on, for ColorDistribution::SequencePerSpawnPoint
        size_t particleIndex = 0;     // Particles spawned so far, for ColorDistribution::SequencePerParticle

        for (const Vec2& origin : spawnPoints) {
            for (int i = 0; i < particlesPerPoint; ++i) {
                float angle = angleDist(rng);
                float speed = speedDist(rng);

                Particle p(settings, colorPicker.pick(spawnPointIndex, particleIndex));
                p.setPosition(origin);
                p.setVelocity(std::cos(angle) * speed, std::sin(angle) * speed);

                particles.push_back(p);
                ++particleIndex;
            }
            ++spawnPointIndex;
        }
    }

    
    // ========== Legacy Spawn Methods (for compatibility) ==========

    /**
     * @brief Spawn particles in a burst pattern from an origin point
     * @param origin Center point of the burst
     * @param count Number of particles to spawn
     * @param settings What each particle starts with
     * @param minSpeed Minimum initial speed
     * @param maxSpeed Maximum initial speed
     */
    void spawnBurst(Vec2 origin, int count, const ParticleEmitterSettings& settings,
                    float minSpeed = 5.0f, float maxSpeed = 15.0f) {
        std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * std::numbers::pi_v<float>);
        std::uniform_real_distribution<float> speedDist(minSpeed, maxSpeed);
        const ColorPicker colorPicker(settings);

        for (int i = 0; i < count; ++i) {
            float angle = angleDist(rng);
            float speed = speedDist(rng);

            Particle p(settings, colorPicker.pick(0, i));
            p.setPosition(origin);
            p.setVelocity(std::cos(angle) * speed, std::sin(angle) * speed);

            particles.push_back(p);
        }
    }

    /**
     * @brief Spawn particles in a fountain pattern (upward spray)
     * @param origin Base point of the fountain
     * @param count Number of particles to spawn
     * @param settings What each particle starts with
     * @param angle Spray angle in radians (0 = straight up)
     * @param spread Angle spread (how wide the spray is)
     */
    void spawnFountain(Vec2 origin, int count, const ParticleEmitterSettings& settings,
                       float angle = -std::numbers::pi_v<float>/2, float spread = std::numbers::pi_v<float>/6,
                       float minSpeed = 8.0f, float maxSpeed = 15.0f)
    {
        std::uniform_real_distribution<float> angleDist(angle - spread/2, angle + spread/2);
        std::uniform_real_distribution<float> speedDist(minSpeed, maxSpeed);
        const ColorPicker colorPicker(settings);

        for (int i = 0; i < count; ++i) {
            float particleAngle = angleDist(rng);
            float speed = speedDist(rng);

            Particle p(settings, colorPicker.pick(0, i));
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
     * @param settings What each particle starts with
     */
    void spawnRain(int minXPos, int maxXPos, int count, const ParticleEmitterSettings& settings) {
        std::uniform_int_distribution<int> xDist(minXPos, maxXPos);
        std::uniform_real_distribution<float> speedDist(2.0f, 5.0f);
        const ColorPicker colorPicker(settings);

        for (int i = 0; i < count; ++i) {
            int x = xDist(rng);
            float speed = speedDist(rng);

            Particle p(settings, colorPicker.pick(0, i));
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
     * @param settings What each particle starts with
     */
    void spawnLine(Vec2 start, Vec2 end, int count, const ParticleEmitterSettings& settings) {
        const ColorPicker colorPicker(settings);
        for (int i = 0; i < count; ++i) {
            float t = static_cast<float>(i) / (count - 1);
            Vec2 pos = start + (end - start) * t;

            Particle p(settings, colorPicker.pick(0, i));
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
 * @brief What a particle animation leaves on screen when it finishes.
 */
enum class ParticleEndBehavior : uint8_t
{
    RestoreAndShow,  // Put the window's pre-effect content back and push it to the screen
    RestoreOnly      // Put it back in the window buffer only -- the caller refreshes (e.g. batched with other windows)
};


/**
 * @brief Options for spawnBurst().
 */
struct ParticleBurstOptions
{
    float duration = 0.5f;          // How long to animate the effect, in seconds
    int particlesPerPoint = 3;      // How many particles spawn from each point
    float minSpeed = 6.0f;          // Minimum particle speed
    float maxSpeed = 12.0f;         // Maximum particle speed
    ParticleEndBehavior onEnd = ParticleEndBehavior::RestoreAndShow;
};


/**
 * @brief Spawn a burst particle effect and animate it for a specified duration
 *
 * This is a high-level helper function that creates, spawns, animates, and cleans up
 * a particle burst effect. Useful for quick one-off effects like success celebrations.
 * Blocks for options.duration.
 *
 * @param window The window to render particles on
 * @param spawnPoints Vector of positions where particles will burst from
 * @param settings What each particle starts with (physics, colors, lifetime, etc.)
 * @param options Duration, density, speed range, and what's left on screen afterwards
 *
 * Example usage:
 * @code
 * std::vector<Vec2> points = {{10, 5}, {12, 5}, {14, 5}};
 * ParticleEmitterSettings sparks = ParticleEmitterPresets::Sparks();
 * sparks.colors = {"bright-green_on_bright-green"};
 * spawnBurst(myWindow, points, sparks, { .duration = 1.0f });
 * @endcode
 */
inline void spawnBurst(WINDOW* window,
                       const std::vector<Vec2>& spawnPoints,
                       const ParticleEmitterSettings& settings,
                       const ParticleBurstOptions& options = {})
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
    effect.setParticlesPerPoint(options.particlesPerPoint);
    effect.setSpeedRange(options.minSpeed, options.maxSpeed);

    // Spawn particles
    effect.spawn(settings);

    // Animate for specified duration
    auto startTime = std::chrono::high_resolution_clock::now();

    while (true) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = currentTime - startTime;
        if (elapsed.count() >= options.duration) break;

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

    // The restore above only reaches the window buffer -- without pushing it, the last particle
    // frame stays on the physical screen until something else redraws this window
    if (options.onEnd == ParticleEndBehavior::RestoreAndShow) {
        wnoutrefresh(window);
        doupdate();
    }
}

} // namespace ParticleFX
} // namespace stevensTerminal
