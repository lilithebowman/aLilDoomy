// aLilDoomy.cpp : Defines the entry point for the application.
//

#include <windows.h>
#include <tchar.h>
#include <d3d9.h>
#define PIXMAP_WIDTH 640
#define PIXMAP_HEIGHT 480

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);



LPDIRECT3D9 d9 = 0;
LPDIRECT3DDEVICE9 d3dev;
D3DPRESENT_PARAMETERS d3dpp;
LPDIRECT3DSURFACE9 surf;



LPDIRECT3DSURFACE9 create_surface(LPDIRECT3DDEVICE9 dev) {
	LPDIRECT3DSURFACE9 surf;
	D3DLOCKED_RECT lockedRect;
	int x, y;
	char* data;
	if (FAILED(IDirect3DDevice9_CreateOffscreenPlainSurface(dev, PIXMAP_WIDTH, PIXMAP_HEIGHT, D3DFMT_X8R8G8B8,
		D3DPOOL_DEFAULT, &surf, NULL))) {
		return NULL;
	}

	IDirect3DSurface9_LockRect(surf, &lockedRect, NULL, D3DLOCK_DISCARD);
	data = (char*)lockedRect.pBits;
	for (y = 0; y < PIXMAP_HEIGHT; ++y) {
		DWORD* row = (DWORD*)data;
		for (x = 0; x < PIXMAP_WIDTH; ++x) {
			*row++ = D3DCOLOR_XRGB(255 - 255 * y / PIXMAP_HEIGHT,
				255 * x / PIXMAP_WIDTH,
				255 * y / PIXMAP_HEIGHT);
		}
		data += lockedRect.Pitch;
	}
	IDirect3DSurface9_UnlockRect(surf);
	return surf;
}

static void
config_parameters(D3DPRESENT_PARAMETERS* d3dpp, HWND hWnd) {
	ZeroMemory(d3dpp, sizeof(D3DPRESENT_PARAMETERS));
	d3dpp->Windowed = TRUE;
	d3dpp->SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp->hDeviceWindow = hWnd;
	d3dpp->BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp->BackBufferWidth = PIXMAP_WIDTH;
	d3dpp->BackBufferHeight = PIXMAP_HEIGHT;
	d3dpp->EnableAutoDepthStencil = TRUE;
	d3dpp->AutoDepthStencilFormat = D3DFMT_D16;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow) {
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = _T("DX9Pixmap");

	RegisterClassEx(&wc);

	RECT winRect;
	winRect.left = 0;
	winRect.top = 0;
	winRect.right = PIXMAP_WIDTH;
	winRect.bottom = PIXMAP_HEIGHT;
	AdjustWindowRect(&winRect, WS_OVERLAPPEDWINDOW, FALSE);

	HWND hWnd = CreateWindow(_T("DX9Pixmap"), _T("DX9 Pixmap Shell"),
		WS_OVERLAPPEDWINDOW,
		0, 0,
		winRect.right - winRect.left, winRect.bottom - winRect.top,
		NULL, NULL, hInstance, NULL);

	ShowWindow(hWnd, SW_SHOWDEFAULT);

	if (NULL == (d9 = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;

	config_parameters(&d3dpp, hWnd);
	if (FAILED(IDirect3D9_CreateDevice(d9, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &d3dev))) {
		IDirect3D9_Release(d9);
		return 1;
	}
	surf = create_surface(d3dev);

	DWORD frameTimer = GetTickCount();
	MSG msg;
	while (TRUE) {
		DWORD endFrame;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) {
				break;
			}
		}

		if (msg.message == WM_QUIT) {
			break;
		}

		/* Actually do our work for this frame here... */

		endFrame = GetTickCount();
		if (endFrame - frameTimer < 20) {
			Sleep(20 - (endFrame - frameTimer));
		}
		frameTimer = endFrame;
	}

	return (int)(msg.wParam);
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_CLOSE:
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

