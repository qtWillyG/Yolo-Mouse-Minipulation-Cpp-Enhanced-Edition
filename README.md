# YOLO Mouse

A polished Windows cursor-motion playground with a simple native GUI. Adjust the duration, pattern size, and speed, then run a smooth figure-eight animation from your cursor's current position.

> This is a cursor animation demo—not a YOLO object-detection model.

## Highlights

- Modern dark Windows control panel
- Live sliders for duration, pattern size, and speed
- Three-second safety countdown
- Start and Stop controls with live status
- Press **Esc** at any time to stop
- Returns the cursor to its starting position
- Lightweight native C++ with no runtime dependencies

## Quick start

### Easiest method: project dashboard

Double-click **`dashboard.bat`**, then press **1**. The dashboard checks your tools, configures the project, and builds the program. After setup, use option **2** to launch it.

The numbered dashboard also lets you rebuild, open the project folder, inspect build errors, check requirements, clean the build, and view help.

### Manual method

### 1. Install the tools

Install [Visual Studio 2022 Community](https://visualstudio.microsoft.com/vs/community/) and select **Desktop development with C++**. Ensure CMake support is included.

### 2. Open a developer terminal

From the Start menu, open **Developer PowerShell for VS 2022**.

### 3. Build

```powershell
git clone <your-repository-url>
cd <your-repository-folder>
cmake -S . -B build
cmake --build build --config Release
```

### 4. Run

```powershell
.\build\Release\yolo-mouse.exe
```

Set the controls, click **Start motion**, and keep **Esc** ready as the universal safety stop.

## Controls

| Control | Range | Purpose |
|---|---:|---|
| Duration | 5–120 seconds | How long the animation runs |
| Pattern size | 40–500 pixels | Width of the figure-eight motion |
| Motion speed | 0.1–5.0x | How quickly the pattern repeats |
| Stop / Esc | Anytime | Stops and restores the cursor |

## Requirements

- Windows 10 or Windows 11
- Visual Studio 2022 C++ build tools
- CMake 3.20 or newer

## Safety

The app moves only the system cursor. It does not click, type, capture the screen, use a camera, or run object detection. Closing the window, clicking **Stop**, reaching the duration limit, or pressing **Esc** restores the cursor to where it started.

## License

MIT — see [LICENSE](LICENSE).
