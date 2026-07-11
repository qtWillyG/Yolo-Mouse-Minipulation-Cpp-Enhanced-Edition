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
constexpr COLORREF Background = RGB(13, 16, 25);
constexpr COLORREF Panel = RGB(24, 29, 43);
constexpr COLORREF Text = RGB(242, 244, 250);
constexpr COLORREF Muted = RGB(154, 164, 184);
constexpr COLORREF Accent = RGB(99, 102, 241);
constexpr COLORREF Success = RGB(94, 234, 212);
constexpr UINT_PTR MotionTimer = 1;

enum ControlId {
    DurationId = 101, RadiusId, SpeedId, HeightId, CountdownId, PatternId,
    ReturnId, TopmostId, StartId = 201, StopId, DefaultsId
};

struct Settings {
    int duration = 15;
    int radius = 180;
    int speedTenths = 10;
    int heightPercent = 50;
    int countdown = 3;
    int pattern = 0;
    bool returnCursor = true;
    bool topmost = false;
    int accentRed = 99;
    int accentGreen = 102;
    int accentBlue = 241;
};

struct AppState {
    HWND window{};
    HWND duration{}, radius{}, speed{}, height{}, countdown{}, pattern{};
    HWND returnCursor{}, topmost{}, start{}, stop{}, defaults{};
    HFONT titleFont{}, bodyFont{}, smallFont{};
    bool running = false;
    POINT origin{};
    Settings active{};
    Settings startup{};
    std::chrono::steady_clock::time_point started{};
    std::wstring status = L"Ready — customize the settings and press Start";
} app;

std::wstring settingsPath() {
    wchar_t directory[MAX_PATH]{};
    GetCurrentDirectoryW(MAX_PATH, directory);
    return std::wstring(directory) + L"\\yolo-mouse.ini";
}

Settings loadSettings() {
    Settings s;
    const std::wstring path = settingsPath();
    auto value = [&](const wchar_t* key, int fallback) {
        return static_cast<int>(GetPrivateProfileIntW(L"Settings", key, fallback, path.c_str()));
    };
    s.duration = std::clamp(value(L"Duration", s.duration), 5, 300);
    s.radius = std::clamp(value(L"Radius", s.radius), 20, 800);
    s.speedTenths = std::clamp(value(L"SpeedTenths", s.speedTenths), 1, 100);
    s.heightPercent = std::clamp(value(L"HeightPercent", s.heightPercent), 10, 100);
    s.countdown = std::clamp(value(L"Countdown", s.countdown), 0, 10);
    s.pattern = std::clamp(value(L"Pattern", s.pattern), 0, 3);
    s.returnCursor = value(L"ReturnCursor", 1) != 0;
    s.topmost = value(L"AlwaysOnTop", 0) != 0;
    s.accentRed = std::clamp(value(L"AccentRed", s.accentRed), 0, 255);
    s.accentGreen = std::clamp(value(L"AccentGreen", s.accentGreen), 0, 255);
    s.accentBlue = std::clamp(value(L"AccentBlue", s.accentBlue), 0, 255);
    return s;
}

COLORREF accentColor() { return RGB(app.startup.accentRed, app.startup.accentGreen, app.startup.accentBlue); }

int slider(HWND control) { return static_cast<int>(SendMessageW(control, TBM_GETPOS, 0, 0)); }
bool checked(HWND control) { return SendMessageW(control, BM_GETCHECK, 0, 0) == BST_CHECKED; }

void setStatus(const std::wstring& text) {
    app.status = text;
    InvalidateRect(app.window, nullptr, FALSE);
}

Settings readSettings() {
    Settings s;
    s.duration = slider(app.duration);
    s.radius = slider(app.radius);
    s.speedTenths = slider(app.speed);
    s.heightPercent = slider(app.height);
    s.countdown = slider(app.countdown);
    s.pattern = static_cast<int>(SendMessageW(app.pattern, CB_GETCURSEL, 0, 0));
    s.returnCursor = checked(app.returnCursor);
    s.topmost = checked(app.topmost);
    return s;
}

void enableSettings(bool enabled) {
    for (HWND control : {app.duration, app.radius, app.speed, app.height, app.countdown,
                         app.pattern, app.returnCursor, app.topmost, app.defaults}) {
        EnableWindow(control, enabled);
    }
    EnableWindow(app.start, enabled);
    EnableWindow(app.stop, !enabled);
}

void applyTopmost(bool enabled) {
    SetWindowPos(app.window, enabled ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void stopMotion(const wchar_t* message) {
    KillTimer(app.window, MotionTimer);
    if (app.running && app.active.returnCursor) SetCursorPos(app.origin.x, app.origin.y);
    app.running = false;
    enableSettings(true);
    setStatus(message);
}

void resetDefaults() {
    const Settings s;
    SendMessageW(app.duration, TBM_SETPOS, TRUE, s.duration);
    SendMessageW(app.radius, TBM_SETPOS, TRUE, s.radius);
    SendMessageW(app.speed, TBM_SETPOS, TRUE, s.speedTenths);
    SendMessageW(app.height, TBM_SETPOS, TRUE, s.heightPercent);
    SendMessageW(app.countdown, TBM_SETPOS, TRUE, s.countdown);
    SendMessageW(app.pattern, CB_SETCURSEL, s.pattern, 0);
    SendMessageW(app.returnCursor, BM_SETCHECK, BST_CHECKED, 0);
    SendMessageW(app.topmost, BM_SETCHECK, BST_UNCHECKED, 0);
    applyTopmost(false);
    setStatus(L"Default settings restored");
}

void drawLabel(HDC dc, const std::wstring& text, RECT rect, HFONT font, COLORREF color, UINT align = DT_LEFT) {
    SelectObject(dc, font);
    SetTextColor(dc, color);
    SetBkMode(dc, TRANSPARENT);
    DrawTextW(dc, text.c_str(), -1, &rect, align | DT_SINGLELINE | DT_VCENTER);
}

HWND makeSlider(int id, int minimum, int maximum, int value, int y) {
    HWND control = CreateWindowExW(0, TRACKBAR_CLASSW, L"", WS_CHILD | WS_VISIBLE | TBS_NOTICKS,
        48, y, 504, 30, app.window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), nullptr, nullptr);
    SendMessageW(control, TBM_SETRANGE, TRUE, MAKELPARAM(minimum, maximum));
    SendMessageW(control, TBM_SETPOS, TRUE, value);
    SendMessageW(control, TBM_SETPAGESIZE, 0, std::max(1, (maximum - minimum) / 10));
    return control;
}

HWND makeButton(const wchar_t* text, int id, int x, int y, int width, bool enabled = true) {
    return CreateWindowW(L"BUTTON", text, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | (enabled ? 0 : WS_DISABLED),
        x, y, width, 48, app.window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), nullptr, nullptr);
}

void createControls() {
    app.duration = makeSlider(DurationId, 5, 300, app.startup.duration, 190);
    app.radius = makeSlider(RadiusId, 20, 800, app.startup.radius, 270);
    app.speed = makeSlider(SpeedId, 1, 100, app.startup.speedTenths, 350);
    app.height = makeSlider(HeightId, 10, 100, app.startup.heightPercent, 430);
    app.countdown = makeSlider(CountdownId, 0, 10, app.startup.countdown, 510);

    app.pattern = CreateWindowW(WC_COMBOBOXW, L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
        48, 582, 240, 180, app.window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(PatternId)), nullptr, nullptr);
    for (const wchar_t* name : {L"Figure eight", L"Circle", L"Horizontal wave", L"Vertical wave"})
        SendMessageW(app.pattern, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(name));
    SendMessageW(app.pattern, CB_SETCURSEL, app.startup.pattern, 0);

    app.returnCursor = CreateWindowW(L"BUTTON", L" Return cursor when stopped", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        318, 575, 250, 28, app.window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(ReturnId)), nullptr, nullptr);
    app.topmost = CreateWindowW(L"BUTTON", L" Keep window always on top", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        318, 611, 250, 28, app.window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(TopmostId)), nullptr, nullptr);
    SendMessageW(app.returnCursor, BM_SETCHECK, app.startup.returnCursor ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(app.topmost, BM_SETCHECK, app.startup.topmost ? BST_CHECKED : BST_UNCHECKED, 0);
    applyTopmost(app.startup.topmost);

    app.start = makeButton(L"Start motion", StartId, 48, 676, 240);
    app.stop = makeButton(L"Stop", StopId, 312, 676, 112, false);
    app.defaults = makeButton(L"Reset", DefaultsId, 440, 676, 112);
}

void moveCursor(double motionTime) {
    const double angle = motionTime * (app.active.speedTenths / 10.0) * 2.0 * std::numbers::pi;
    const double vertical = app.active.radius * (app.active.heightPercent / 100.0);
    double dx = 0.0, dy = 0.0;
    switch (app.active.pattern) {
    case 1: dx = app.active.radius * std::cos(angle); dy = vertical * std::sin(angle); break;
    case 2: dx = app.active.radius * std::sin(angle); dy = 0.0; break;
    case 3: dx = 0.0; dy = vertical * std::sin(angle); break;
    default: dx = app.active.radius * std::sin(angle); dy = vertical * std::sin(2.0 * angle); break;
    }
    const int maxX = std::max(0, GetSystemMetrics(SM_CXSCREEN) - 1);
    const int maxY = std::max(0, GetSystemMetrics(SM_CYSCREEN) - 1);
    SetCursorPos(std::clamp(static_cast<int>(app.origin.x + dx), 0, maxX),
                 std::clamp(static_cast<int>(app.origin.y + dy), 0, maxY));
}

void paintWindow(HDC dc) {
    RECT client{}; GetClientRect(app.window, &client);
    HBRUSH background = CreateSolidBrush(Background); FillRect(dc, &client, background); DeleteObject(background);
    RECT panel{24, 116, client.right - 24, 655}; HBRUSH card = CreateSolidBrush(Panel); FillRect(dc, &panel, card); DeleteObject(card);

    RECT r{48, 24, 570, 66}; drawLabel(dc, L"YOLO Mouse Studio", r, app.titleFont, Text);
    r = {48, 66, 570, 96}; drawLabel(dc, L"Design your cursor motion, your way", r, app.smallFont, Muted);
    const Settings s = readSettings();
    const std::wstring labels[] = {
        L"Duration", L"Pattern size", L"Motion speed", L"Vertical scale", L"Start countdown"
    };
    const std::wstring values[] = {
        std::to_wstring(s.duration) + L" sec", std::to_wstring(s.radius) + L" px",
        std::to_wstring(s.speedTenths / 10) + L"." + std::to_wstring(s.speedTenths % 10) + L"x",
        std::to_wstring(s.heightPercent) + L"%", std::to_wstring(s.countdown) + L" sec"
    };
    for (int i = 0; i < 5; ++i) {
        const int y = 143 + i * 80;
        r = {48, y, 400, y + 32}; drawLabel(dc, labels[i], r, app.bodyFont, Text);
        r = {400, y, 552, y + 32}; drawLabel(dc, values[i], r, app.bodyFont, Success, DT_RIGHT);
    }
    r = {48, 548, 288, 578}; drawLabel(dc, L"Motion pattern", r, app.bodyFont, Text);
    r = {48, 742, 552, 778}; drawLabel(dc, app.status, r, app.bodyFont, app.running ? Success : Muted, DT_CENTER);
    r = {48, 782, 552, 810}; drawLabel(dc, L"Press Esc anytime for an immediate safety stop", r, app.smallFont, Muted, DT_CENTER);
}

LRESULT CALLBACK windowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        app.window = window;
        app.startup = loadSettings();
        app.titleFont = CreateFontW(31, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
        app.bodyFont = CreateFontW(17, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
        app.smallFont = CreateFontW(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
        createControls();
        return 0;
    case WM_HSCROLL:
        InvalidateRect(window, nullptr, FALSE);
        return 0;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case StartId:
            if (!app.running) {
                if (!GetCursorPos(&app.origin)) { setStatus(L"Error: could not read the cursor position"); break; }
                app.active = readSettings();
                app.running = true;
                enableSettings(false);
                app.started = std::chrono::steady_clock::now();
                setStatus(app.active.countdown ? L"Get ready — countdown started" : L"Motion running");
                SetTimer(window, MotionTimer, 16, nullptr);
            }
            break;
        case StopId: stopMotion(L"Stopped safely"); break;
        case DefaultsId: resetDefaults(); break;
        case TopmostId: applyTopmost(checked(app.topmost)); break;
        }
        return 0;
    case WM_TIMER: {
        if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0) { stopMotion(L"Stopped safely with Esc"); return 0; }
        const double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - app.started).count();
        if (elapsed < app.active.countdown) {
            const int remaining = app.active.countdown - static_cast<int>(elapsed);
            setStatus(L"Starting in " + std::to_wstring(remaining) + L"… press Esc to cancel");
            return 0;
        }
        const double motionTime = elapsed - app.active.countdown;
        if (motionTime >= app.active.duration) { stopMotion(L"Complete — motion finished"); return 0; }
        moveCursor(motionTime);
        setStatus(L"Running — " + std::to_wstring(app.active.duration - static_cast<int>(motionTime)) + L" seconds remaining");
        return 0;
    }
    case WM_DRAWITEM: {
        auto* item = reinterpret_cast<DRAWITEMSTRUCT*>(lParam);
        const bool disabled = (item->itemState & ODS_DISABLED) != 0;
        COLORREF color = RGB(49, 56, 73);
        if (item->CtlID == StartId) color = accentColor();
        if (disabled) color = RGB(55, 61, 76);
        HBRUSH brush = CreateSolidBrush(color); FillRect(item->hDC, &item->rcItem, brush); DeleteObject(brush);
        wchar_t caption[64]{}; GetWindowTextW(item->hwndItem, caption, 64);
        drawLabel(item->hDC, caption, item->rcItem, app.bodyFont, disabled ? Muted : Text, DT_CENTER);
        return TRUE;
    }
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN:
        SetBkColor(reinterpret_cast<HDC>(wParam), Panel);
        SetTextColor(reinterpret_cast<HDC>(wParam), Text);
        return reinterpret_cast<LRESULT>(GetStockObject(NULL_BRUSH));
    case WM_ERASEBKGND: return 1;
    case WM_PAINT: {
        PAINTSTRUCT ps{}; HDC dc = BeginPaint(window, &ps); paintWindow(dc); EndPaint(window, &ps); return 0;
    }
    case WM_DESTROY:
        if (app.running && app.active.returnCursor) SetCursorPos(app.origin.x, app.origin.y);
        DeleteObject(app.titleFont); DeleteObject(app.bodyFont); DeleteObject(app.smallFont);
        PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show) {
    INITCOMMONCONTROLSEX controls{sizeof(controls), ICC_BAR_CLASSES}; InitCommonControlsEx(&controls);
    const wchar_t className[] = L"YoloMouseStudio";
    HBRUSH classBrush = CreateSolidBrush(Background);
    WNDCLASSEXW wc{sizeof(wc)}; wc.lpfnWndProc = windowProc; wc.hInstance = instance; wc.lpszClassName = className;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW); wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION); wc.hbrBackground = classBrush;
    if (!RegisterClassExW(&wc)) return 1;
    HWND window = CreateWindowExW(0, className, L"YOLO Mouse Studio", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 620, 870, nullptr, nullptr, instance, nullptr);
    if (!window) return 1;
    ShowWindow(window, show); UpdateWindow(window);
    MSG message{}; while (GetMessageW(&message, nullptr, 0, 0) > 0) { TranslateMessage(&message); DispatchMessageW(&message); }
    DeleteObject(classBrush);
    return static_cast<int>(message.wParam);
}
