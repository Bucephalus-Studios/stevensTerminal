#pragma once

#include <unordered_map>

namespace stevensTerminal {
namespace ParticleFX {

/**
 * @brief Registry for managing window snapshots across multiple particle effects
 *
 * This allows multiple overlapping particle effects to share the same "clean" window snapshot,
 * preventing trails and ensuring proper layering.
 */
class ParticleWindowRegistry {
private:
    struct WindowState {
        WINDOW* snapshot;
        int refCount;  // How many active effects are using this snapshot
    };

    static std::unordered_map<WINDOW*, WindowState> registry;

public:
    /**
     * @brief Register a window for particle effects (creates snapshot if first registration)
     */
    static void registerWindow(WINDOW* window) {
        if (registry.find(window) == registry.end()) {
            // First effect on this window - create snapshot
            registry[window] = {dupwin(window), 1};
        } else {
            // Additional effect - increment ref count
            registry[window].refCount++;
        }
    }

    /**
     * @brief Unregister a window (deletes snapshot when last effect finishes)
     */
    static void unregisterWindow(WINDOW* window) {
        auto it = registry.find(window);
        if (it != registry.end()) {
            it->second.refCount--;
            if (it->second.refCount == 0) {
                // Last effect finished - restore original and cleanup
                overwrite(it->second.snapshot, window);
                delwin(it->second.snapshot);
                registry.erase(it);
            }
        }
    }

    /**
     * @brief Restore window to its original state (before any particles)
     */
    static void restoreWindow(WINDOW* window) {
        auto it = registry.find(window);
        if (it != registry.end()) {
            overwrite(it->second.snapshot, window);
        }
    }

    /**
     * @brief Check if a window has active particle effects
     */
    static bool hasActiveEffects(WINDOW* window) {
        auto it = registry.find(window);
        return it != registry.end() && it->second.refCount > 0;
    }
};

// Static member declaration - definition moved to .cpp


} // namespace ParticleFX
} // namespace stevensTerminal

