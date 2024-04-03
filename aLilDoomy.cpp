// aLilDoomy.cpp : Defines the entry point for the application.
//

#include <math.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include "framework.h"
#include "aLilDoomy.h"

#define MAX_LOADSTRING 100
#define _USE_MATH_DEFINES

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


typedef struct {
    int fr1, fr2;   // frame 1, frame 2, to create constant framerate
}time; time T;

typedef struct {
    int w, s, a, d; //move up, down, left, right
    int sl, sr;     // strafe left, right
    int m;          // move up, down look up, down
}keys; volatile keys K;

typedef struct {
    float cos[360];
    float sin[360];
}math; math M;

typedef struct {
    int x, y, z;    // player position
    int a;          // player angle of rotation l/r
    int l;          // variable to look up and down
}player; volatile player P;

typedef struct {
    int width;
    int height;
}displayBuffer; displayBuffer DBuf;

volatile HWND windowRef;
volatile HDC displayContextRef;


void draw3D(HDC memDC, int width, int height) {
    // SetPixel(memDC, x, y, RGB(127, x, y));
    int wx[4], wy[4], wz[4];
    float CS = M.cos[P.a]; float SN = M.sin[P.a];

    // offset bottom two points by player
    int x1 = 40 - P.x;
    int y1 = 10 - P.y;
    int x2 = 40 - P.x;
    int y2 = 290 - P.y;

    // world X position
    wx[0] = x1 * CS - y1 * SN;
    wx[1] = x2 * CS - y2 * SN;
    // world Y position
    wy[0] = y1 * CS + x1 * SN;
    wy[1] = y2 * CS + x2 * SN;
    // world Z height
    wz[0] = 0 - P.z + ((P.l * wy[0]) / 32.0);
    wz[1] = 0 - P.z + ((P.l * wy[1]) / 32.0);

    // calculate screen X and Y position
    if (wy[0] > 0) wx[0] = wx[0] * 200 / wy[0] + (width / 2);
    if (wy[0] > 0) wy[0] = wz[0] * 200 / wy[0] + (height / 2);
    if (wy[1] > 0) wx[1] = wx[1] * 200 / wy[1] + (width / 2);
    if (wy[1] > 0) wy[1] = wz[1] * 200 / wy[1] + (height / 2);

    // don't draw off screen points
    if (
        wx[0] > 0 &&
        wx[0] < width &&
        wy[0] > 0 &&
        wy[0] < height
        ) {
        // draw pixel
        SetPixel(memDC, wx[0], wy[0], RGB(55, 55, 55));
    }
    if (
        wx[1] > 0 &&
        wx[1] < width &&
        wy[1] > 0 &&
        wy[1] < height
        ) {
        // draw pixel
        SetPixel(memDC, wx[1], wy[1], RGB(55, 55, 55));
    }
}

void getKeys() {
    K.w = GetKeyState(0x57); // w key code
    K.a = GetKeyState(0x41); // a key code
    K.s = GetKeyState(0x53); // s key code
    K.d = GetKeyState(0x44); // d key code

    K.sl = GetKeyState(VK_OEM_COMMA); // comma key code
    K.sr = GetKeyState(VK_OEM_PERIOD); // period key code

    K.m = GetKeyState(0x4D); // m key code

    //OutputDebugStringW(L"*");
}

void movePlayer() {
    // move up, down, left, right
    if (K.a == 1 && K.m == 0) { P.a -= 4; if (P.a < 0) { P.a += 360; } }
    if (K.d == 1 && K.m == 0) { P.a += 4; if (P.a << 0 > 359) { P.a -= 360; } }
    int dx = M.sin[P.a] * 10;
    int dy = M.cos[P.a] * 10;
    if (K.w == 1 && K.m == 0) { P.x += dx; P.y += dy; }
    if (K.s == 1 && K.m == 0) { P.x -= dx; P.y -= dy; }
    // strafe left, right
    if (K.sl == 1) { P.x += dy; P.y -= dx; }
    if (K.sr == 1) { P.x -= dy; P.y += dx; }
    // move up, down / look up, down
    if (K.a == 1 && K.m == 1) { P.l -= 1; }
    if (K.d == 1 && K.m == 1) { P.l += 1; }
    if (K.w == 1 && K.m == 1) { P.z -= 4; }
    if (K.s == 1 && K.m == 1) { P.z += 4; }
}

// Main Draw Function
void PaintScreen(HWND hWnd, HDC hdc) {
    RECT windowRect;
    int width, height;
    // init display buffer
    if (GetWindowRect(hWnd, &windowRect)) {
        DBuf.width = windowRect.right - windowRect.left;
        DBuf.height = windowRect.bottom - windowRect.top;
    }

    width = DBuf.width / 4; height = DBuf.height / 4;

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBM = CreateCompatibleBitmap(hdc, width , height);

    SelectObject(memDC, memBM);

    draw3D(memDC, width, height);

    StretchBlt(hdc, 0, 0, DBuf.width, DBuf.height, memDC, 0, 0, width, height, SRCCOPY);   // Copy memory buffer to window 2x stretched

    DeleteDC(memDC);
    DeleteObject(memBM);
}

HANDLE threadHandle;

DWORD WINAPI MainThread(LPVOID lpParam) {
    OutputDebugStringW(L"Spawned PaintScreen thread");

    while (true) {

        PaintScreen(windowRef, displayContextRef);

    }
}

// Init function
void init(HWND hWnd) {
    int x;
    // store sin/cos in degrees
    for (x = 0; x < 350; x++) {
        M.cos[x] = cos(x / 180.0 * M_PI);
        M.sin[x] = sin(x / 180.0 * M_PI);
    }

    // init player values
    P.x = 70; P.y = -110; P.z = 20; P.a = 0; P.l = 0;

    // Launch the display thread
    threadHandle = CreateThread(
        NULL,                   // default security attributes
        0,                      // use default stack size  
        MainThread,             // thread function name
        NULL,                   // argument to thread function 
        0,                      // use default creation flags 
        NULL                    // returns the thread identifier 
    );
}







int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_ALILDOOMY, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ALILDOOMY));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ALILDOOMY));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_ALILDOOMY);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   init(hWnd);
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_KEYDOWN: {
            K.w = K.a = K.s = K.d = K.sl = K.sr = K.m = 0;
            getKeys();
            movePlayer();
        }
        break;
    case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId) {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            windowRef = hWnd;
            displayContextRef = hdc;
            PaintScreen(windowRef, displayContextRef);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
