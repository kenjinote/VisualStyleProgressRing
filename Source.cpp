#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "gdiplus")

#include <windows.h>
#include <gdiplus.h>

#define IMAGE_WIDTH 25

using namespace Gdiplus;

TCHAR szClassName[] = TEXT("Window");

Gdiplus::Image* LoadImageFromSystemResource()
{
	Gdiplus::Image* pImage = nullptr;
	HMODULE hHandle = LoadLibrary(L"SystemResources\\imageres.dll.mun");
	if (hHandle) {
		HRSRC hRes = FindResource(hHandle, MAKEINTRESOURCE(5024), L"PNG");
		if (hRes) {
			HGLOBAL hResLoad = LoadResource(hHandle, hRes);
			if (hResLoad) {
				LPVOID lpResLock = LockResource(hResLoad);
				if (lpResLock)
				{
					const int nSize = SizeofResource(hHandle, hRes);
					IStream* pIStream;
					ULARGE_INTEGER LargeUInt = {};
					LARGE_INTEGER LargeInt = {};
					(void)CreateStreamOnHGlobal(0, 1, &pIStream);
					LargeUInt.QuadPart = nSize;
					pIStream->SetSize(LargeUInt);
					LargeInt.QuadPart = 0;
					pIStream->Seek(LargeInt, STREAM_SEEK_SET, NULL);
					pIStream->Write(lpResLock, nSize, NULL);
					pImage = Gdiplus::Image::FromStream(pIStream);
					pIStream->Release();
				}
			}
		}
		FreeLibrary(hHandle);
	}
	return pImage;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static Image* image;
	static INT nCount;
	static HDC hMemDC;
	static HBITMAP hBitmap;
	static HBITMAP hOldBitmap;
	switch (msg)
	{
	case WM_CREATE:
		image = LoadImageFromSystemResource();
		if (image) {
			HDC hdc = GetDC(hWnd);
			if (hdc) {
				hMemDC = CreateCompatibleDC(hdc);
				if (hMemDC) {
					RECT rect;
					GetClientRect(hWnd, &rect);
					hBitmap = CreateCompatibleBitmap(hdc, IMAGE_WIDTH, IMAGE_WIDTH);
					hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
				}
				ReleaseDC(hWnd, hdc);
			}
		}
		else {
			MessageBox(hWnd, L"イメージが読み込めませんでした。", 0, 0);
			return -1;
		}
		SetTimer(hWnd, 0x1234, 50, 0);
		break;
	case WM_TIMER:
		{
			PatBlt(hMemDC, 0, 0, IMAGE_WIDTH, IMAGE_WIDTH, WHITENESS);
			::Graphics g(hMemDC);
			Point point[] = { {0,0}, {IMAGE_WIDTH,0}, {0,IMAGE_WIDTH} };
			g.DrawImage(image, point, _countof(point), 0, IMAGE_WIDTH * nCount, IMAGE_WIDTH, IMAGE_WIDTH, Unit::UnitPixel);
		}
		nCount++;
		if (nCount >= 18) nCount = 0;
		InvalidateRect(hWnd, 0, 0);
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			BitBlt(hdc, 10, 10, IMAGE_WIDTH, IMAGE_WIDTH, hMemDC, 0, 0, SRCCOPY);
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_DESTROY:
		KillTimer(hWnd, 0x1234);
		delete image;
		if (hMemDC) {
			if (hOldBitmap) {
				SelectObject(hMemDC, hOldBitmap);
			}
			if (hBitmap) {
				DeleteObject(hBitmap);
				hBitmap = 0;
			}
			DeleteDC(hMemDC);
			hMemDC = 0;
		}
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	ULONG_PTR gdiToken;
	GdiplusStartupInput gdiSI;
	GdiplusStartup(&gdiToken, &gdiSI, NULL);
	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		L"Window",
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	GdiplusShutdown(gdiToken);
	return (int)msg.wParam;
}
