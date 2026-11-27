#pragma once

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace kpart_cli {

inline bool isStandaloneConsole() {
#ifdef _WIN32
    DWORD pids[4];
    return GetConsoleProcessList(pids, 4) == 1;
#else
    return false;
#endif
}

struct ConsolePauseGuard {
    bool enabled = false;

    explicit ConsolePauseGuard(bool shouldPause = isStandaloneConsole())
        : enabled(shouldPause) {}

    ~ConsolePauseGuard() {
        if (!enabled) {
            return;
        }
        std::cout.flush();
        std::cerr.flush();
        if (std::cin.eof()) {
            return;
        }
        std::cin.clear();
        std::cout << "\nPress Enter to exit...";
        std::cout.flush();
        std::string dummy;
        std::getline(std::cin, dummy);
    }
};

inline std::string trimWhitespace(std::string s) {
    auto notSpace = [](unsigned char ch) { return !std::isspace(ch); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
    return s;
}

inline std::string trimAndUnquote(std::string s) {
    s = trimWhitespace(std::move(s));
    if (s.size() >= 2 &&
        ((s.front() == '"' && s.back() == '"') ||
         (s.front() == '\'' && s.back() == '\''))) {
        s = trimWhitespace(s.substr(1, s.size() - 2));
    }
    return s;
}

inline bool parsePositiveInt(const std::string& raw, int& outValue) {
    const std::string text = trimWhitespace(raw);
    if (text.empty()) {
        return false;
    }
    try {
        size_t parsedCharacters = 0;
        int parsed = std::stoi(text, &parsedCharacters);
        if (parsedCharacters != text.size() || parsed <= 0) {
            return false;
        }
        outValue = parsed;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

inline std::vector<std::filesystem::path> discoverInputFiles() {
    namespace fs = std::filesystem;
    std::vector<fs::path> found;
    const std::vector<fs::path> searchDirs = {
        "../Examples/input",
        "Examples/input",
        "."
    };

    std::error_code ec;
    for (const auto& dir : searchDirs) {
        if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec)) {
            continue;
        }
        for (const auto& entry : fs::directory_iterator(dir, ec)) {
            if (!entry.is_regular_file(ec)) {
                continue;
            }
            const fs::path& p = entry.path();
            if (p.extension() != ".txt") {
                continue;
            }
            const std::string name = p.filename().string();
            if (name.size() >= 11 && name.compare(name.size() - 11, 11, "_output.txt") == 0) {
                continue;
            }
            found.push_back(p.lexically_normal());
        }
        if (!found.empty()) {
            break;
        }
    }

    std::sort(found.begin(), found.end(), [](const fs::path& a, const fs::path& b) {
        return a.filename().string() < b.filename().string();
    });
    return found;
}

inline bool promptForK(int& k) {
    while (true) {
        std::cout << "Enter k (positive integer): ";
        std::cout.flush();
        std::string line;
        if (!std::getline(std::cin, line)) {
            std::cerr << "Invalid k: must be a positive integer.\n";
            return false;
        }
        if (parsePositiveInt(line, k)) {
            return true;
        }
        std::cout << "Invalid k. Please enter a positive integer (e.g., 2, 3, 4).\n";
    }
}

inline bool resolveSolverArguments(
    int argc,
    char* argv[],
    const std::string& solverTitle,
    std::filesystem::path& inputPath,
    int& k
) {
    namespace fs = std::filesystem;

    if (argc == 3) {
        inputPath = fs::path(argv[1]);
        if (!parsePositiveInt(argv[2], k)) {
            std::cerr << "Invalid k: must be a positive integer.\n";
            return false;
        }
        return true;
    }

    if (argc == 2) {
        if (!isStandaloneConsole()) {
            std::cerr << "Please provide the following arguments: <input_file.txt> <k>\n";
            return false;
        }
        inputPath = fs::path(argv[1]);
        std::cout << "=== " << solverTitle << " ===\n";
        std::cout << "Input file: " << inputPath.string() << "\n";
        return promptForK(k);
    }

    if (argc != 1) {
        std::cerr << "Please provide the following arguments: <input_file.txt> <k>\n";
        return false;
    }

    std::cout << "=== " << solverTitle << " ===\n";
    const auto discovered = discoverInputFiles();
    if (!discovered.empty()) {
        std::cout << "Available input files:\n";
        for (size_t i = 0; i < discovered.size(); ++i) {
            std::cout << "  [" << (i + 1) << "] " << discovered[i].string() << "\n";
        }
    }

    while (true) {
        std::cout << "Enter input file path"
                  << (discovered.empty() ? "" : " (or number from list)")
                  << " (you can drag & drop a .txt file here): ";
        std::cout.flush();

        std::string rawInput;
        if (!std::getline(std::cin, rawInput)) {
            std::cerr << "Please provide the following arguments: <input_file.txt> <k>\n";
            return false;
        }

        const std::string cleaned = trimAndUnquote(rawInput);
        if (cleaned.empty()) {
            continue;
        }
        if (cleaned == "q" || cleaned == "quit") {
            return false;
        }

        int choice = 0;
        if (!discovered.empty() && parsePositiveInt(cleaned, choice) &&
            choice >= 1 && static_cast<size_t>(choice) <= discovered.size() &&
            !fs::exists(fs::path(cleaned))) {
            inputPath = discovered[static_cast<size_t>(choice - 1)];
            std::cout << "Selected: " << inputPath.string() << "\n";
            break;
        }

        fs::path candidate(cleaned);
        if (candidate.extension() != ".txt") {
            std::cout << "Input file must have a .txt extension. Try again.\n";
            continue;
        }
        std::error_code ec;
        if (!fs::exists(candidate, ec)) {
            std::cout << "File not found: " << candidate.string() << ". Try again.\n";
            continue;
        }
        inputPath = candidate;
        break;
    }

    return promptForK(k);
}

inline std::filesystem::path resolveOutputDirectory(
    int argc,
    const std::filesystem::path& inputPath,
    const std::string& outputFolderName
) {
    namespace fs = std::filesystem;
    if (argc == 3) {
        return fs::path("../Examples/output") / outputFolderName;
    }

    std::error_code ec;
    if (fs::exists("../Examples", ec)) {
        return fs::path("../Examples/output") / outputFolderName;
    }
    if (fs::exists("Examples", ec)) {
        return fs::path("Examples/output") / outputFolderName;
    }
    if (inputPath.has_parent_path() && fs::exists(inputPath.parent_path(), ec)) {
        return inputPath.parent_path() / outputFolderName;
    }
    return fs::path(outputFolderName);
}

} // namespace kpart_cli
