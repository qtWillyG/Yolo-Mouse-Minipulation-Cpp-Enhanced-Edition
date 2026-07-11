#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <commctrl.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <numbers>
#include <string>

namespace {
constexpr COLORREF kBackground = RGB(15, 18, 28);
constexpr COLORREF kPanel = RGB(25, 30, 44);
constexpr COLORREF kText = RGB(240, 243, 250);
constexpr COLORREF kMuted = RGB(153, 163, 184);
constexpr COLORREF kAccent = RGB(99, 102, 241);
constexpr UINT_PTR kMotionTimer = 1;
constexpr int ID_DURATION = 101, ID_RADIUS = 102, ID_SPEED = 103;
constexpr int ID_START = 201, ID_STOP = 202;

HWND durationSlider{}, radiusSlider{}, speedSlider{}, startButton{}, stopButton{};
HFONT titleFont{}, bodyFont{}, smallFont{};
bool running = false;
int countdown = 0;
POINT origin{};
std::chrono::steady_clock::time_point started;
std::wstring statusText = L"Ready to begin";

int sliderValue(HWND control) { return static_cast<int>(SendMessage(control, TBM_GETPOS, 0, 0)); }

void setStatus(HWND window, const std::wstring& text) {
    statusText = text;
    InvalidateRect(window, nullptr, FALSE);
}

void stopMotion(HWND window, const wchar_t* message) {
    KillTimer(window, kMotionTimer);
    if (running) SetCursorPos(origin.x, origin.y);
    running = false;
    countdown = 0;
    EnableWindow(startButton, TRUE);
    EnableWindow(stopButton, FALSE);
    setStatus(window, message);
}

void drawText(HDC dc, const wchar_t* text, RECT rect, HFONT font, COLORREF color, UINT format = DT_LEFT) {
    SelectObject(dc, font);
    SetTextColor(dc, color);
    SetBkMode(dc, TRANSPARENT);
    DrawTextW(dc, text, -1, &rect, format | DT_SINGLELINE | DT_VCENTER);
}

void createSlider(HWND window, HWND& slider, int id, int min, int max, int value, int y) {
    slider = CreateWindowExW(0, TRACKBAR_CLASSW, L"", WS_CHILD | WS_VISIBLE | TBS_NOTICKS,
        48, y, 500, 34, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), nullptr, nullptr);
    SendMessage(slider, TBM_SETRANGE, TRUE, MAKELPARAM(min, max));
    SendMessage(slider, TBM_SETPOS, TRUE, value);
    SendMessage(slider, TBM_SETPAGESIZE, 0, std::max(1, (max - min) / 10));
}

LRESULT CALLBACK windowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
        titleFont = CreateFontW(30, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
        bodyFont = CreateFontW(18, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
        smallFont = CreateFontW(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
        createSlider(window, durationSlider, ID_DURATION, 5, 120, 15, 175);
        createSlider(window, radiusSlider, ID_RADIUS, 40, 500, 180, 265);
        createSlider(window, speedSlider, ID_SPEED, 1, 50, 10, 355);
        startButton = CreateWindowW(L"BUTTON", L"Start motion", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 48, 430, 238, 52, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_START)), nullptr, nullptr);
        stopButton = CreateWindowW(L"BUTTON", L"Stop", WS_CHILD | WS_VISIBLE | WS_DISABLED | BS_OWNERDRAW, 310, 430, 238, 52, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_STOP)), nullptr, nullptr);
        return 0;
    }
    case WM_HSCROLL:
        InvalidateRect(window, nullptr, FALSE);
        return 0;
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_START && !running) {
            if (!GetCursorPos(&origin)) { setStatus(window, L"Could not read cursor position"); return 0; }
            running = true; countdown = 3;
            EnableWindow(startButton, FALSE); EnableWindow(stopButton, TRUE);
            setStatus(window, L"Starting in 3 seconds — press Esc to cancel");
            SetTimer(window, kMotionTimer, 16, nullptr);
            started = std::chrono::steady_clock::now();
        } else if (LOWORD(wParam) == ID_STOP) stopMotion(window, L"Stopped — cursor returned to start");
        return 0;
    case WM_TIMER: {
        if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0) { stopMotion(window, L"Stopped safely with Esc"); return 0; }
        const double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
        if (elapsed < 3.0) {
            const int next = 3 - static_cast<int>(elapsed);
            if (next != countdown) { countdown = next; setStatus(window, L"Starting in " + std::to_wstring(countdown) + L" seconds — press Esc to cancel"); }
            return 0;
        }
        const double motionTime = elapsed - 3.0;
        const int duration = sliderValue(durationSlider);
        if (motionTime >= duration) { stopMotion(window, L"Complete — cursor returned to start"); return 0; }
        const double speed = sliderValue(speedSlider) / 10.0;
        const int radius = sliderValue(radiusSlider);
        const double angle = motionTime * speed * 2.0 * std::numbers::pi;
        const int maxX = std::max(0, GetSystemMetrics(SM_CXSCREEN) - 1);
        const int maxY = std::max(0, GetSystemMetrics(SM_CYSCREEN) - 1);
        SetCursorPos(std::clamp(static_cast<int>(origin.x) + static_cast<int>(radius * std::sin(angle)), 0, maxX),
            std::clamp(static_cast<int>(origin.y) + static_cast<int>(radius * .5 * std::sin(2 * angle)), 0, maxY));
        setStatus(window, L"Motion running — " + std::to_wstring(duration - static_cast<int>(motionTime)) + L" seconds remaining");
        return 0;
    }
    case WM_DRAWITEM: {
        auto* item = reinterpret_cast<DRAWITEMSTRUCT*>(lParam);
        const bool isStart = item->CtlID == ID_START;
        const bool disabled = (item->itemState & ODS_DISABLED) != 0;
        HBRUSH brush = CreateSolidBrush(disabled ? RGB(55, 61, 77) : (isStart ? kAccent : RGB(48, 55, 72)));
        FillRect(item->hDC, &item->rcItem, brush); DeleteObject(brush);
        RECT textRect = item->rcItem;
        drawText(item->hDC, isStart ? L"Start motion" : L"Stop", textRect, bodyFont, disabled ? kMuted : kText, DT_CENTER);
        return TRUE;
    }
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN:
        SetBkColor(reinterpret_cast<HDC>(wParam), kBackground);
        SetTextColor(reinterpret_cast<HDC>(wParam), kText);
        return reinterpret_cast<LRESULT>(GetStockObject(NULL_BRUSH));
    case WM_ERASEBKGND: return 1;
    case WM_PAINT: {
        PAINTSTRUCT ps{}; HDC dc = BeginPaint(window, &ps);
        RECT client{}; GetClientRect(window, &client);
        HBRUSH bg = CreateSolidBrush(kBackground); FillRect(dc, &client, bg); DeleteObject(bg);
        RECT panel{24, 108, client.right - 24, 410}; HBRUSH p = CreateSolidBrush(kPanel); FillRect(dc, &panel, p); DeleteObject(p);
        RECT r{48, 25, 560, 68}; drawText(dc, L"YOLO Mouse", r, titleFont, kText);
        r = {48, 66, 560, 94}; drawText(dc, L"A safe, configurable cursor-motion playground", r, smallFont, kMuted);
        const int duration = sliderValue(durationSlider), radius = sliderValue(radiusSlider), speed10 = sliderValue(speedSlider);
        std::wstring value;
        r = {48, 125, 548, 158}; value = L"Duration                                      " + std::to_wstring(duration) + L" sec"; drawText(dc, value.c_str(), r, bodyFont, kText);
        r = {48, 215, 548, 248}; value = L"Pattern size                                  " + std::to_wstring(radius) + L" px"; drawText(dc, value.c_str(), r, bodyFont, kText);
        r = {48, 305, 548, 338}; value = L"Motion speed                                  " + std::to_wstring(speed10 / 10) + L"." + std::to_wstring(speed10 % 10) + L"x"; drawText(dc, value.c_str(), r, bodyFont, kText);
        r = {48, 500, 548, 535}; drawText(dc, statusText.c_str(), r, bodyFont, running ? RGB(129, 230, 217) : kMuted, DT_CENTER);
        r = {48, 540, 548, 570}; drawText(dc, L"Safety: press Esc at any time to stop and restore the cursor", r, smallFont, kMuted, DT_CENTER);
        EndPaint(window, &ps); return 0;
    }
    case WM_DESTROY:
        if (running) SetCursorPos(origin.x, origin.y);
        DeleteObject(titleFont); DeleteObject(bodyFont); DeleteObject(smallFont);
        PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show) {
    INITCOMMONCONTROLSEX controls{sizeof(controls), ICC_BAR_CLASSES}; InitCommonControlsEx(&controls);
    const wchar_t className[] = L"YoloMouseControlPanel";
    WNDCLASSEXW wc{sizeof(wc)}; wc.lpfnWndProc = windowProc; wc.hInstance = instance; wc.lpszClassName = className;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW); wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION); wc.hbrBackground = CreateSolidBrush(kBackground);
    RegisterClassExW(&wc);
    HWND window = CreateWindowExW(0, className, L"YOLO Mouse", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 620, 640, nullptr, nullptr, instance, nullptr);
    if (!window) return 1;
    ShowWindow(window, show); UpdateWindow(window);
    MSG message{}; while (GetMessageW(&message, nullptr, 0, 0) > 0) { TranslateMessage(&message); DispatchMessageW(&message); }
    return static_cast<int>(message.wParam);
}
