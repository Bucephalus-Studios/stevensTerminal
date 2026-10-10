#pragma once
/**
 * @file Animation.hpp
 * @brief General-purpose animation loop for stevensTerminal.
 *
 * TODO (long-term): unify animation so it can drive particle effects too.
 *   - An Animation interface: update(dt), render(WINDOW*), isFinished(), and an end behaviour
 *     (restore and show / restore only / clear). ParticleEffect, Spinner and caller timelines
 *     (e.g. a skill check roll) implement it.
 *   - A scheduler that owns the frame loop on the calling (UI) thread. Windows are designated
 *     as animated and share one framerate: each tick, restore each window's base layer, render
 *     its animations in layer order, wnoutrefresh; then one doupdate(). Curses is not
 *     thread-safe -- background threads may only produce data, never draw.
 *   - A base layer per animated window, generalising ParticleWindowRegistry's snapshots. It must
 *     be re-saved whenever the caller redraws that window's real content.
 *   - Sequencing primitives (Sequence, Delay, Parallel) to replace hand-written sleep_for timelines.
 *   - Decide between blocking (runUntilFinished) and non-blocking (the caller's input loop polls
 *     with wtimeout and ticks the scheduler); non-blocking reaches into every caller's input path.
 */

#include <atomic>
#include <chrono>
#include <functional>
#include <thread>

namespace stevensTerminal
{
    struct AnimationOptions
    {
        int frameDuration = 300;  // time between frame advances (ms)
        int renderMs      = 100;  // how often onFrame is called (ms)
    };

    /**
     * @brief Run an animation loop on the calling thread until keepRunning() returns false.
     *
     * A background ticker thread increments a frame counter every opts.frameDuration milliseconds.
     * onFrame(frame) is called on the calling thread every opts.renderMs milliseconds with
     * the current frame number — use it to redraw whatever window(s) you like.
     *
     * All threading, atomics, and timing are handled internally; the caller only
     * provides what to draw and when to stop.
     *
     * @param keepRunning  Returns true while the operation is still in progress.
     * @param onFrame      Called each render tick with the current frame index.
     * @param opts         Timing options (tickMs, renderMs).
     */
    inline void runAnimation(std::function<bool()>    keepRunning,
                             std::function<void(int)> onFrame,
                             AnimationOptions         opts = {})
    {
        const int tickMs   = opts.frameDuration;
        const int renderMs = opts.renderMs;
        std::atomic<int>  frame  { 0 };
        std::atomic<bool> running{ true };

        std::thread ticker([&]() {
            while (running.load(std::memory_order_relaxed))
            {
                frame.fetch_add(1, std::memory_order_relaxed);
                std::this_thread::sleep_for(std::chrono::milliseconds(tickMs));
            }
        });

        while (keepRunning())
        {
            onFrame(frame.load(std::memory_order_relaxed));
            std::this_thread::sleep_for(std::chrono::milliseconds(renderMs));
        }

        running.store(false);
        ticker.join();
    }

} // namespace stevensTerminal
