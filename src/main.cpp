#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <numbers>
#include <string>
#include <thread>

namespace {
struct Options {
    int durationSeconds = 15;
    int radiusPixels = 180;
    double speed = 1.0;
    bool dryRun = false;
};

void printHelp() {
    std::cout
        << "YOLO Mouse - a small, visible Windows cursor-motion demo\n\n"
        << "Usage: yolo-mouse.exe [options]\n\n"
        << "Options:\n"
        << "  --duration <seconds>  Run time from 1 to 300 (default: 15)\n"
        << "  --radius <pixels>     Pattern radius from 20 to 1000 (default: 180)\n"
        << "  --speed <value>       Motion speed from 0.1 to 10 (default: 1.0)\n"
        << "  --dry-run             Show configuration without moving the cursor\n"
        << "  --help                Show this help\n\n"
        << "Press Esc at any time to stop.\n";
}

bool parseOptions(int argc, char* argv[], Options& options) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help") {
            printHelp();
            std::exit(EXIT_SUCCESS);
        }
        if (arg == "--dry-run") {
            options.dryRun = true;
            continue;
        }
        if (i + 1 >= argc) {
            std::cerr << "Missing value after " << arg << ".\n";
            return false;
        }
        try {
            if (arg == "--duration") {
                options.durationSeconds = std::stoi(argv[++i]);
            } else if (arg == "--radius") {
                options.radiusPixels = std::stoi(argv[++i]);
            } else if (arg == "--speed") {
                options.speed = std::stod(argv[++i]);
            } else {
                std::cerr << "Unknown option: " << arg << ".\n";
                return false;
            }
        } catch (...) {
            std::cerr << "Invalid value for " << arg << ".\n";
            return false;
        }
    }

    if (options.durationSeconds < 1 || options.durationSeconds > 300 ||
        options.radiusPixels < 20 || options.radiusPixels > 1000 ||
        options.speed < 0.1 || options.speed > 10.0) {
        std::cerr << "One or more values are outside the supported range.\n";
        return false;
    }
    return true;
}
} // namespace

int main(int argc, char* argv[]) {
    Options options;
    if (!parseOptions(argc, argv, options)) {
        printHelp();
        return EXIT_FAILURE;
    }

    std::cout << "Duration: " << options.durationSeconds << " s\n"
              << "Radius:   " << options.radiusPixels << " px\n"
              << "Speed:    " << options.speed << "x\n";
    if (options.dryRun) {
        std::cout << "Dry run complete; the cursor was not moved.\n";
        return EXIT_SUCCESS;
    }

    POINT origin{};
    if (!GetCursorPos(&origin)) {
        std::cerr << "Could not read the cursor position.\n";
        return EXIT_FAILURE;
    }

    std::cout << "Starting in 3 seconds. Press Esc to cancel or stop.\n";
    for (int remaining = 3; remaining > 0; --remaining) {
        std::cout << remaining << "...\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0) {
            std::cout << "Cancelled.\n";
            return EXIT_SUCCESS;
        }
    }

    const int maxX = std::max(0, GetSystemMetrics(SM_CXSCREEN) - 1);
    const int maxY = std::max(0, GetSystemMetrics(SM_CYSCREEN) - 1);
    const auto started = std::chrono::steady_clock::now();

    while (true) {
        const auto now = std::chrono::steady_clock::now();
        const double elapsed = std::chrono::duration<double>(now - started).count();
        if (elapsed >= options.durationSeconds || (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0) {
            break;
        }

        const double angle = elapsed * options.speed * 2.0 * std::numbers::pi;
        const int x = origin.x + static_cast<int>(options.radiusPixels * std::sin(angle));
        const int y = origin.y + static_cast<int>(options.radiusPixels * 0.5 * std::sin(2.0 * angle));
        SetCursorPos(std::clamp(x, 0, maxX), std::clamp(y, 0, maxY));
        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }

    SetCursorPos(origin.x, origin.y);
    std::cout << "Stopped. Cursor returned to its starting position.\n";
    return EXIT_SUCCESS;
}
