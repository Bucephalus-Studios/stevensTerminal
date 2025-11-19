/**
 * @file stevensFileLib.hpp
 * @author Jeff Stevens (jeff@bucephalusstudios.com)
 * @brief A library for working with files and directories
 * @version 0.2
 * @date 2024-02-21
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef STEVENS_FILE_LIB_HPP
#define STEVENS_FILE_LIB_HPP

#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <stdexcept>
#include <random>
#include <algorithm>
#include <sstream>

namespace stevensFileLib
{
    // ============================================================================
    // Configuration Structures
    // ============================================================================

    /**
     * @brief Configuration for loading files into vectors
     */
    struct LoadSettings
    {
        std::vector<std::string> skipIfStartsWith;
        std::vector<std::string> skipIfContains;
        bool skipEmptyLines = true;
        char separator = '\n';

        LoadSettings() = default;

        LoadSettings(const std::unordered_map<std::string, std::vector<std::string>>& settingsMap,
                    char sep = '\n', bool skipEmpty = true)
            : skipEmptyLines(skipEmpty), separator(sep)
        {
            auto startsWith = settingsMap.find("skip if starts with");
            if (startsWith != settingsMap.end())
                skipIfStartsWith = startsWith->second;

            auto contains = settingsMap.find("skip if contains");
            if (contains != settingsMap.end())
                skipIfContains = contains->second;
        }
    };

    /**
     * @brief Configuration for listing files in directories
     */
    struct ListFilesSettings
    {
        std::unordered_set<std::string> targetExtensions;
        std::unordered_set<std::string> excludeExtensions;
        std::unordered_set<std::string> excludeFiles;

        ListFilesSettings() = default;

        static ListFilesSettings fromMap(const std::unordered_map<std::string, std::string>& settingsMap)
        {
            ListFilesSettings settings;

            auto target = settingsMap.find("targetFileExtensions");
            if (target != settingsMap.end() && !target->second.empty())
                settings.targetExtensions = parseCommaSeparated(target->second);

            auto exclude = settingsMap.find("excludeFileExtensions");
            if (exclude != settingsMap.end() && !exclude->second.empty())
                settings.excludeExtensions = parseCommaSeparated(exclude->second);

            auto excludeFilesEntry = settingsMap.find("excludeFiles");
            if (excludeFilesEntry != settingsMap.end() && !excludeFilesEntry->second.empty())
                settings.excludeFiles = parseCommaSeparated(excludeFilesEntry->second);

            return settings;
        }

    private:
        static std::unordered_set<std::string> parseCommaSeparated(const std::string& input)
        {
            std::unordered_set<std::string> result;
            std::stringstream stream(input);
            std::string item;

            while (std::getline(stream, item, ','))
            {
                if (!item.empty())
                    result.insert(item);
            }

            return result;
        }
    };

    // ============================================================================
    // Internal String Utilities (to remove external dependency)
    // ============================================================================

    namespace internal
    {
        inline bool startsWith(const std::string& str, const std::string& prefix)
        {
            if (prefix.size() > str.size())
                return false;
            return str.compare(0, prefix.size(), prefix) == 0;
        }

        inline bool contains(const std::string& str, const std::string& substring)
        {
            return str.find(substring) != std::string::npos;
        }

        inline std::vector<std::string> splitString(const std::string& str, const std::string& delimiter)
        {
            std::vector<std::string> result;
            size_t start = 0;
            size_t end = str.find(delimiter);

            while (end != std::string::npos)
            {
                result.push_back(str.substr(start, end - start));
                start = end + delimiter.length();
                end = str.find(delimiter, start);
            }

            result.push_back(str.substr(start));
            return result;
        }
    }

    // ============================================================================
    // File Opening Functions
    // ============================================================================

    /**
     * @brief Opens a file for reading with validation
     *
     * @param filePath Path to the file
     * @return std::ifstream Input file stream object
     * @throws std::invalid_argument if file cannot be opened
     */
    inline std::ifstream openInputFile(const std::string& filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
            throw std::invalid_argument("Failed to open file for reading: " + filePath);
        return file;
    }

    /**
     * @brief Opens a file for appending with validation
     *
     * @param filePath Path to the file
     * @return std::ofstream Output file stream object
     * @throws std::invalid_argument if file cannot be opened
     */
    inline std::ofstream openOutputFile(const std::string& filePath)
    {
        std::ofstream file(filePath, std::ios::app);
        if (!file.is_open())
            throw std::invalid_argument("Failed to open file for writing: " + filePath);
        return file;
    }

    // ============================================================================
    // File Writing Functions
    // ============================================================================

    /**
     * @brief Appends content to a file, optionally creating it if it doesn't exist
     *
     * @tparam ContentType Type of content to append (must support operator<<)
     * @param filePath Path to the file
     * @param content Content to append
     * @param createIfNonExistent If true, creates the file if it doesn't exist
     * @throws std::invalid_argument if file doesn't exist and createIfNonExistent is false
     * @throws std::runtime_error if file creation fails
     */
    template<typename ContentType>
    void appendToFile(const std::string& filePath, const ContentType& content,
                     bool createIfNonExistent = true)
    {
        // Check if file exists when createIfNonExistent is false
        if (!createIfNonExistent && !std::filesystem::exists(filePath))
            throw std::invalid_argument("File does not exist: " + filePath);

        std::ofstream file(filePath, std::ios::app);
        if (!file.is_open())
            throw std::runtime_error("Failed to open file for writing: " + filePath);

        file << content;
    }

    // ============================================================================
    // Line Filtering Helper Functions
    // ============================================================================

    namespace internal
    {
        inline bool shouldSkipLine(const std::string& line, const LoadSettings& settings)
        {
            if (settings.skipEmptyLines && line.empty())
                return true;

            for (const auto& prefix : settings.skipIfStartsWith)
            {
                if (startsWith(line, prefix))
                    return true;
            }

            for (const auto& substring : settings.skipIfContains)
            {
                if (contains(line, substring))
                    return true;
            }

            return false;
        }

        inline bool shouldIncludeFile(const std::filesystem::path& filePath,
                                     const ListFilesSettings& settings)
        {
            const std::string filename = filePath.filename().string();
            const std::string extension = filePath.extension().string();

            if (settings.excludeFiles.count(filename) > 0)
                return false;

            if (!settings.targetExtensions.empty())
                return settings.targetExtensions.count(extension) > 0;

            if (!settings.excludeExtensions.empty())
                return settings.excludeExtensions.count(extension) == 0;

            return true;
        }
    }

    // ============================================================================
    // File Reading Functions
    // ============================================================================

    /**
     * @brief Loads file contents line-by-line into a vector of strings
     *
     * @param filePath Path to the file
     * @param settingsMap Settings for filtering lines (see LoadSettings)
     * @param separator Character used to separate lines
     * @param skipEmptyLines If true, skip empty lines
     * @return std::vector<std::string> Vector containing file lines
     * @throws std::invalid_argument if file cannot be opened
     */
    inline std::vector<std::string> loadFileIntoVector(
        const std::string& filePath,
        const std::unordered_map<std::string, std::vector<std::string>>& settingsMap = {},
        char separator = '\n',
        bool skipEmptyLines = true)
    {
        std::ifstream file = openInputFile(filePath);
        LoadSettings settings(settingsMap, separator, skipEmptyLines);

        std::vector<std::string> lines;
        std::string line;

        while (std::getline(file, line, separator))
        {
            if (!internal::shouldSkipLine(line, settings))
                lines.push_back(line);
        }

        return lines;
    }

    /**
     * @brief Loads file contents into a vector of integers
     *
     * @param filePath Path to the file
     * @param settingsMap Settings for filtering lines (currently unused but kept for API compatibility)
     * @param separator Character used to separate values
     * @param skipEmptyLines If true, skip empty lines
     * @return std::vector<int> Vector containing integer values
     * @throws std::invalid_argument if file cannot be opened
     */
    inline std::vector<int> loadFileIntoVectorOfInts(
        const std::string& filePath,
        [[maybe_unused]] const std::unordered_map<std::string, std::vector<std::string>>& settingsMap = {},
        [[maybe_unused]] char separator = '\n',
        [[maybe_unused]] bool skipEmptyLines = true)
    {
        std::ifstream file = openInputFile(filePath);
        std::vector<int> numbers;
        int value;

        while (file >> value)
            numbers.push_back(value);

        return numbers;
    }

    /**
     * @brief Returns a random line from a file
     *
     * @param filePath Path to the file
     * @param separator Character used to separate lines
     * @return std::string A random line from the file
     * @throws std::invalid_argument if file cannot be opened
     * @throws std::runtime_error if file is empty
     */
    inline std::string getRandomFileLine(const std::string& filePath, char separator = '\n')
    {
        std::vector<std::string> lines = loadFileIntoVector(filePath, {}, separator, false);

        if (lines.empty())
            throw std::runtime_error("Cannot get random line from empty file: " + filePath);

        static std::random_device randomDevice;
        static std::mt19937 generator(randomDevice());
        std::uniform_int_distribution<size_t> distribution(0, lines.size() - 1);

        return lines[distribution(generator)];
    }

    // ============================================================================
    // Directory Functions
    // ============================================================================

    /**
     * @brief Lists all files in a directory with optional filtering
     *
     * @param directoryPath Path to the directory
     * @param settingsMap Settings for filtering files (see ListFilesSettings)
     * @return std::vector<std::string> Vector of filenames
     * @throws std::invalid_argument if directory doesn't exist
     */
    inline std::vector<std::string> listFiles(
        const std::string& directoryPath,
        const std::unordered_map<std::string, std::string>& settingsMap = {
            {"targetFileExtensions", ""},
            {"excludeFileExtensions", ""},
            {"excludeFiles", ""}
        })
    {
        if (!std::filesystem::exists(directoryPath) ||
            !std::filesystem::is_directory(directoryPath))
        {
            throw std::invalid_argument("Directory does not exist: " + directoryPath);
        }

        ListFilesSettings settings = ListFilesSettings::fromMap(settingsMap);
        std::vector<std::string> fileNames;

        for (const auto& entry : std::filesystem::directory_iterator(directoryPath))
        {
            if (!entry.is_regular_file())
                continue;

            if (internal::shouldIncludeFile(entry.path(), settings))
                fileNames.push_back(entry.path().filename().string());
        }

        return fileNames;
    }

} // namespace stevensFileLib

#endif // STEVENS_FILE_LIB_HPP
