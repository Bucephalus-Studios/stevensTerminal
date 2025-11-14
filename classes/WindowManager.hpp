/**
 * @brief Window Manager for efficient ncurses window handling
 * 
 * This class manages a set of reusable windows that can be dynamically
 * resized and repositioned for different views, eliminating the need
 * for constant window creation/destruction that causes flickering.
 * 
 * Part of the stevensTerminal library.
 */

#ifndef STEVENS_TERMINAL_WINDOW_MANAGER_HPP
#define STEVENS_TERMINAL_WINDOW_MANAGER_HPP

#include <unordered_map>
#include <string>
#include <vector>

#if defined(__linux__)
    #include<ncurses.h>
#elif defined(_WIN32)
    #include<pdcurses.h>
#endif

namespace stevensTerminal {

    /**
     * @brief Simple data structure for window specifications
     */
    struct WindowSpec {
        int height;
        int width;
        int startY;
        int startX;
        
        WindowSpec(int h = 0, int w = 0, int y = 0, int x = 0) 
            : height(h), width(w), startY(y), startX(x) {}
    };

    /**
     * @brief Window Manager class for efficient window lifecycle management
     */
    class WindowManager {
    private:
        std::unordered_map<std::string, WINDOW*> windows;
        
    public:
        WindowManager() = default;
        
        ~WindowManager() {
            cleanup();
        }
        
        /**
         * @brief Initialize the window manager with client-specified window names
         * @param windowNames Vector of window names to create
         */
        void initialize(const std::vector<std::string>& windowNames) {
            for (const std::string& name : windowNames) {
                // Create minimal windows - will be resized as needed
                windows[name] = newwin(1, 1, 0, 0);
            }
        }
        
        /**
         * @brief Initialize with default window set (for backward compatibility)
         */
        void initialize() {
            initialize({"header", "art", "prompt", "responses", "debug"});
        }
        
        /**
         * @brief Get a window, creating it if it doesn't exist
         */
        WINDOW* getWindow(const std::string& name) {
            auto it = windows.find(name);
            if (it == windows.end()) {
                windows[name] = newwin(1, 1, 0, 0);
                return windows[name];
            }
            return it->second;
        }
        
        /**
         * @brief Resize and reposition a window efficiently
         */
        void configureWindow(const std::string& name, const WindowSpec& spec) {
            WINDOW* win = getWindow(name);
            
            // Resize and move the window
            wresize(win, spec.height, spec.width);
            mvwin(win, spec.startY, spec.startX);
            
            // Clear the window for fresh content
            werase(win);
        }
        
        /**
         * @brief Configure multiple windows at once for a specific view layout
         */
        void configureLayout(const std::unordered_map<std::string, WindowSpec>& layout) {
            for (const auto& [name, spec] : layout) {
                configureWindow(name, spec);
            }
        }
        
        /**
         * @brief Get window by name (operator overload)
         */
        WINDOW* operator[](const std::string& name) {
            return getWindow(name);
        }
        
        /**
         * @brief Check if a window exists
         */
        bool hasWindow(const std::string& name) const {
            return windows.find(name) != windows.end();
        }
        
        /**
         * @brief Get list of all window names
         */
        std::vector<std::string> getWindowNames() const {
            std::vector<std::string> names;
            for (const auto& [name, window] : windows) {
                names.push_back(name);
            }
            return names;
        }
        
        /**
         * @brief Clean up all windows
         */
        void cleanup() {
            for (auto& [name, window] : windows) {
                if (window != nullptr) {
                    delwin(window);
                }
            }
            windows.clear();
        }
        
        /**
         * @brief Refresh all windows efficiently using double buffering
         */
        void refreshAll() {
            for (const auto& [name, window] : windows) {
                wnoutrefresh(window);
            }
            doupdate(); // Single screen update
        }
        
        /**
         * @brief Complete shutdown of window manager and ncurses
         * 
         * This is the recommended way to clean up when terminating a program
         * that uses stevensTerminal. It handles proper cleanup order:
         * 1. Clean up all managed windows
         * 2. Terminate ncurses properly
         */
        void shutdown() {
            cleanup();  // Clean up all managed windows first
            endwin();   // Then terminate ncurses
        }
    };

} // namespace stevensTerminal

#endif // STEVENS_TERMINAL_WINDOW_MANAGER_HPP