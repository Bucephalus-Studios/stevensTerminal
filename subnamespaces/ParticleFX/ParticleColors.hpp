#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace stevensTerminal {
namespace ParticleFX {

/**
 * @brief How an emitter hands out its colors (ParticleEmitterSettings::colors) to the particles it
 *        spawns -- mirroring Unity's startColor modes / Niagara's array-select modes.
 *
 * TODO: a std::function escape hatch that picks the pair per particle (spawn point index, particle
 *       index), like Phaser's onEmit / Niagara's custom modules -- add when a mode below can't
 *       express what's needed.
 * TODO: color over lifetime (a gradient sampled by a particle's age, e.g. sparks fading out) --
 *       every major particle system has it as a separate stage, so it would sit beside these
 *       spawn-time colors rather than change them.
 */
enum class ColorDistribution : uint8_t
{
    Random,                 // Each particle draws a pair at random, weighted by colorWeights (even by default)
    SequencePerParticle,    // Pairs in order, one per particle, looping
    SequencePerSpawnPoint   // Pairs in order, one per spawn point (all of its particles share it), looping
};

/**
 * @brief Ready-made color lists for ParticleEmitterSettings::colors, by "foreground_on_background"
 *        pair name -- alongside ParticlePresets' physics.
 */
namespace ParticleColors {
    /**
     * @brief Bright red through magenta, as solid blocks. Pair with
     *        ColorDistribution::SequencePerSpawnPoint for a rainbow wave across e.g. a line of text.
     */
    inline std::vector<std::string> Rainbow() {
        return {    "bright-red_on_bright-red",
                    "bright-yellow_on_bright-yellow",
                    "bright-green_on_bright-green",
                    "bright-cyan_on_bright-cyan",
                    "bright-blue_on_bright-blue",
                    "bright-magenta_on_bright-magenta"  };
    }
}

} // namespace ParticleFX
} // namespace stevensTerminal
