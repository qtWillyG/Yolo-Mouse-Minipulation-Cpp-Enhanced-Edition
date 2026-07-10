<div align="center">

# 🖱️ YOLO Mouse

**A tiny, configurable C++ cursor-motion demo for Windows.**

[![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C?logo=cplusplus)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.20%2B-064F8C?logo=cmake)](https://cmake.org/)
[![Platform](https://img.shields.io/badge/platform-Windows-0078D4?logo=windows)](https://www.microsoft.com/windows)
[![License: MIT](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

Draw a smooth figure-eight with your cursor, adjust the size and speed, and stop instantly with **Esc**.

</div>

## Features

- Simple C++20 source with no third-party runtime dependencies
- Three-second safety countdown before movement begins
- **Esc** emergency stop and automatic return to the starting position
- Configurable duration, radius, and speed
- Dry-run mode that never moves the cursor

> [!NOTE]
> “YOLO” is the project name. This starter does not include the YOLO computer-vision model; it demonstrates safe, programmable mouse movement using the Windows API.

## Setup: step by step

### 1. Install the tools

Install both of these:

1. [Visual Studio 2022 Community](https://visualstudio.microsoft.com/vs/community/)
2. In the Visual Studio Installer, select **Desktop development with C++**
3. [CMake 3.20 or newer](https://cmake.org/download/) — select **Add CMake to the system PATH** during installation

### 2. Download the project

Open PowerShell and run:

```powershell
git clone https://github.com/YOUR-USERNAME/yolo-mouse.git
cd yolo-mouse
```

If you downloaded the ZIP instead, extract it, open the extracted folder, right-click an empty area, and choose **Open in Terminal**.

### 3. Configure the build

```powershell
cmake -S . -B build
```

### 4. Compile

```powershell
cmake --build build --config Release
```

### 5. Test without moving the mouse

```powershell
.\build\Release\yolo-mouse.exe --dry-run
```

### 6. Run it

```powershell
.\build\Release\yolo-mouse.exe
```

You have three seconds to prepare. Press **Esc** at any time to stop.

## Usage

```text
yolo-mouse.exe [options]

--duration <seconds>  Run time from 1 to 300 (default: 15)
--radius <pixels>     Pattern radius from 20 to 1000 (default: 180)
--speed <value>       Motion speed from 0.1 to 10 (default: 1.0)
--dry-run             Show configuration without moving the cursor
--help                Show all options
```

Example: move for 10 seconds in a larger, slower pattern:

```powershell
.\build\Release\yolo-mouse.exe --duration 10 --radius 300 --speed 0.6
```

## Safety

Run this only on your own computer and only when you can see the screen. The program does not click, type, install a service, start with Windows, use the network, or run invisibly. Keep the console focused during your first test and use **Esc** to stop.

## Project layout

```text
yolo-mouse/
├── src/
│   └── main.cpp
├── CMakeLists.txt
├── LICENSE
└── README.md
```

## Contributing

Issues and pull requests are welcome. Keep changes small, readable, and easy to test.

## License

Released under the [MIT License](LICENSE).
