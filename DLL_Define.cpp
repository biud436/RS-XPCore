/**
 * @file DLL_Define.cpp
 * @date 2018/04/02 0:30
 *
 * @author biud436
 * Contact: biud436@gmail.com
 *
 * @brief 
 *
 *
 * @note
*/
#define _RSDLL
#include "DLL_Define.h"
#include <Wincodec.h>
#include "resource.h"
#include <stdio.h>

#pragma comment(lib, "Windowscodecs.lib")

HWND g_hWnd = NULL;

// 메인 코어 객체
RSCore* g_prsCore = NULL;

// DLL 핸들
HINSTANCE g_hDllHandle = NULL;

// 현재 프레임
int g_frame = 0;

// 최대 프레임
int g_nMaxFrame = 4;

// 딜레이
int g_nFrameDelay[128] = { 0, };

// 프레임 설정
int g_nStartFrame = 1;
int g_nEndFrame = 4;

BOOL g_bDebugLog = FALSE;

// 즉각 종료
BOOL g_bExitDirectly = FALSE;

BOOL is_another_instance();
WNDPROC OldProc;
LRESULT CALLBACK SuperProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/**
 * 다중 인스턴스 방지
 */
BOOL is_another_instance()
{
	HANDLE hMutex;

	hMutex = CreateMutex(NULL, TRUE, XP_CORE_MUTEX);

	if (GetLastError() == ERROR_ALREADY_EXISTS)
		return TRUE;

	return FALSE;
}


RSDLL void RSSnapToBitmap(unsigned int object)
{
#pragma warning (disable:4312)
	RGSSBMINFO *bitmap = ((RGSSBITMAP*)(object << 1))->bm->bminfo;
#pragma warning (default:4312)

	HWND hWnd = FindWindow("RGSS Player", NULL);

	HDC mainDC = GetDC(hWnd);
	HDC memDC = CreateCompatibleDC(mainDC);
	HBITMAP hBitmap = CreateCompatibleBitmap(mainDC, 640, 480);
	HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);
	
	BitBlt(memDC, 0, 0, 640, 480, mainDC, 0, 0, SRCCOPY);

	BITMAPINFOHEADER bmih = { 0, };
	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = 640;
	bmih.biHeight = 480;
	bmih.biPlanes = 1;
	bmih.biBitCount = 32;

	GetDIBits(memDC, hBitmap, 0, 480, (LPVOID)bitmap->lastRow, (BITMAPINFO*)&bmih, NULL);
	SelectObject(memDC, oldBitmap);
	DeleteObject(hBitmap);
	DeleteDC(memDC);
	ReleaseDC(hWnd, mainDC);
}

/**
 * RGSSBITMAP을 GDI 비트맵으로 변환합니다.
 * 비트맵 스캔 라인 코드와 동일하므로 모든 픽셀을 읽어와야 합니다.
 * CPU로 처리되므로 저사양 컴퓨터에선 시간이 꽤 걸릴 수 있습니다.
 */
RSDLL HBITMAP ConvertRGSSBitmapToBitmap(unsigned int object)
{
#pragma warning (disable:4312)
	RGSSBMINFO *bitmap = ((RGSSBITMAP*)(object << 1))->bm->bminfo;
#pragma warning (default:4312)
	
	HBITMAP hBitmap;
	HDC mainDC;
	VOID *pvBits;
	int width, height;

	mainDC = GetDC(g_hWnd);

	BITMAPINFOHEADER bmih;

	width = bitmap->infoheader->biWidth;
	height = bitmap->infoheader->biHeight;

	bmih.biWidth = width;
	bmih.biHeight = height; // 비트맵은 상하 반전이므로 음수 값으로 반전 처리
	bmih.biBitCount = 32; // 32비트
	bmih.biCompression = BI_RGB;
	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biSizeImage = width * height * 4;
	bmih.biPlanes = 1;
	bmih.biXPelsPerMeter = 2835; // 72 DPI × 39.3701 인치, 2834.6472
	bmih.biYPelsPerMeter = 2835;

	BITMAPINFO* bmi = (BITMAPINFO*)&bmih;

	printf_s("width : %d, height : %d \n", width, height);

	hBitmap = CreateDIBSection(mainDC, bmi, DIB_RGB_COLORS, &pvBits, NULL, 0x0);

	printf_s("hBitmap : %d \n", hBitmap);
	
	RGSSRGBA *row = bitmap->lastRow;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			if ((x * 4) % 4 == 0) {
				((UINT32 *)pvBits)[y * width + x] = (row->pixel);
			}
			row++;
		}
	}

	ReleaseDC(g_hWnd, mainDC);

	return hBitmap;

}

RSDLL void RSSetFrame(int id, unsigned int object)
{
	if (id < 0) id = 0;
	//if (id > g_nMaxFrame - 1) id = g_nMaxFrame - 1;
	if (g_prsCore == NULL) return;
	g_prsCore->hBitmap[id] = ConvertRGSSBitmapToBitmap(object);
}

/************************************************************************/
/* DllMain                                                              */
/************************************************************************/

BOOL WINAPI DllMain(HINSTANCE hDllHandle,
	DWORD     nReason,
	LPVOID    Reserved)
{

	//  Perform global initialization.

	switch (nReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		g_hDllHandle = hDllHandle;
		RSCreateDirectory("Graphics\\SplashScreen\\");
		RSInitWithCoreSystem();
	}
		break;

	case DLL_PROCESS_DETACH:
		RSRemoveCoreSystem();
		break;
	}

	return TRUE;

}

/************************************************************************/
/* RSDLL                                                                */
/************************************************************************/

RSDLL void RSInitWithCoreSystem()
{
	if (g_prsCore != NULL) {
		g_prsCore->init();
	}
	else {
		g_prsCore = new RSCore;
		g_prsCore->init();
	}
	RSCreateWindow();
}

RSDLL void RSRemoveCoreSystem()
{
	if (g_prsCore != NULL) {
		if (g_prsCore->is_open) {
			g_prsCore->close();
		}
		g_prsCore->remove();
		delete g_prsCore;
		g_prsCore = NULL;
	}
		
}

RSDLL void RSInitWithBitmap()
{
	if (g_prsCore != NULL) {
		g_prsCore->initWithBitmap();
	}
}

RSDLL void RSRemoveBitmap()
{
	if (g_prsCore != NULL) {
		g_prsCore->removeBitmaps();
	}
}

RSDLL void RSCreateWindow(void)
{
	if (g_prsCore != NULL) {
		g_prsCore->create();
	}
}	

RSDLL void RSOpenWindow(void)
{
	if (g_prsCore != NULL) {
		g_prsCore->open();
	}
}

RSDLL void RSCloseWindow(void)
{
	if (g_prsCore != NULL) {
		if (g_prsCore->is_open) {
			g_prsCore->close();
		}
	}
}

RSDLL void RSDebugLog(char* s)
{
	if(g_bDebugLog)
		printf(s);
}

RSDLL void RSSetMaxFrame(int n)
{
	if (n < 1) n = 1;
	g_nMaxFrame = n;
}

RSDLL BOOL RSWindowIsOpen(void)
{
	BOOL ret = FALSE;
	if (g_prsCore != NULL)
		ret = g_prsCore->is_open;

	return ret;
}

RSDLL void RSEnableDebugLog(void)
{
	g_bDebugLog = TRUE;

	// 콘솔 창을 엽니다.
	if (g_bDebugLog) {
		AllocConsole();
		freopen("CON", "w", stdout);
	}
}

RSDLL void RSDisableDebugLog(void)
{
	g_bDebugLog = FALSE;
}

RSDLL void RSSetFrameDelay(int frame_id, int delay)
{
	int n = frame_id;

	g_nFrameDelay[n] = delay;
}

RSDLL void RSSetStartEndFrame(int start_frame, int end_frame)
{
	g_nStartFrame = start_frame;
	g_nEndFrame = end_frame;
}

RSDLL void RSCreateDirectory(char* path)
{
	char dirName[MAX_PATH];
	char* q = dirName;

	char* p = path;

	while (*p) {
		if (('\\' == *p) || ('/' == *p))
		{
			if (':' != *(p - 1))
			{
				CreateDirectory(dirName, NULL);
			}
		}
		*q++ = *p++;
		*q = '\0';
	}
	
	CreateDirectory(dirName, NULL);

}

RSDLL void RSErrorHandling(HWND hWnd)
{
	DWORD errorCode = GetLastError();
	char errorMsg[MAX_PATH];

	if (errorCode == 0) {
		return;
	}

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		errorMsg,
		MAX_PATH,
		NULL);

	MessageBox(hWnd, errorMsg, "경고", MB_OK | MB_ICONERROR);

	exit(0);
}

/************************************************************************/
/* Main                                                                 */
/************************************************************************/

unsigned WINAPI ThreadFunc(LPVOID arg)
{
	BOOL done = FALSE;
	UINT32 frameStart, frameTime;

	RSCore& inst = *g_prsCore;

	int frame = g_nStartFrame - 1;
	g_frame = g_nStartFrame - 1;

	WaitForSingleObject(inst.hMutex, INFINITE);

	BOOL temp = g_prsCore->initialized;
	InvalidateRect(g_prsCore->hSubWindow, NULL, TRUE);
	g_prsCore->initialized = temp;

	DWORD tick = GetTickCount();

	const int FPS = 60;
	const int DELAY_TIME = 1000.0f / FPS;

	/************************************************************************/
	/* 임계 영역 시작                                                       */
	/************************************************************************/

	// Fixed Frames per second (FPS)

	while (!done) {

		frameStart = GetTickCount();

		frameTime = GetTickCount() - frameStart;

		if (GetAsyncKeyState(VK_F12) & 0x8000 || g_bExitDirectly) {
			frame = g_nEndFrame - 1;
			g_frame = frame;
			done = TRUE;
		}

		if ((GetTickCount() - tick) >= g_nFrameDelay[frame]) {
			
			frame = (frame + 1) % g_nEndFrame;

			if (frame < g_nStartFrame - 1)
				g_frame = g_nStartFrame - 1;

			if (frame >= g_nEndFrame - 1) {
				frame = g_nEndFrame - 1;
				g_frame = frame;
				// 마지막 프레임을 그린다
				InvalidateRect(g_prsCore->hSubWindow, NULL, TRUE);
				// 마지막 프레임의 딜레이 만큼 쉰다. (없으면 바로 나가짐)
				Sleep(g_nFrameDelay[frame]);
				done = TRUE;
			}
			else {
				g_frame = frame;
				InvalidateRect(g_prsCore->hSubWindow, NULL, TRUE);
			}

			tick = GetTickCount();

		}

		if (frameTime < DELAY_TIME) {
			Sleep(DELAY_TIME - frameTime);
		}

		UpdateWindow(g_prsCore->hSubWindow);
			
	}

	/************************************************************************/
	/* 임계 영역 종료                                                       */
	/************************************************************************/

	ReleaseMutex(inst.hMutex);

	if (g_prsCore) {
		
		// 화면 갱신합니다.	
		g_prsCore->initialized = FALSE;
		
		InvalidateRect(g_prsCore->hSubWindow, NULL, TRUE);
		
		Sleep(100);

		// 창을 닫습니다.
		g_prsCore->close();
	}


	return 0;
}

LRESULT CALLBACK SuperProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE: 
		{
			TCHAR title[256];
			GetWindowText(hWnd, title, 256);
			if (MessageBox(hWnd, "게임을 정말 종료하시겠습니까?", title, MB_YESNOCANCEL) == IDYES) {

				if (g_prsCore != NULL) {
					g_prsCore->callDisposeFromRGSSPlayer();
					g_prsCore->remove();
				}

				Sleep(10);

				if (g_bDebugLog) {
					FreeConsole();
				}

				break;
			}
			else {
				return 0;
			}

			break;
		}
	}
	return CallWindowProc(OldProc, hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;

	switch (uMsg)
	{
	case WM_CREATE:
		g_hWnd = hWnd;
		break;
	case WM_KILLFOCUS:
		break;
	case WM_SETFOCUS:
		break;
	case WM_PAINT:
	{
		if (g_prsCore->initialized) {
			g_prsCore->Render(hdc, hWnd, ps);
		} else {
			g_prsCore->RenderClear(hdc, hWnd, ps);
		}
			
	}
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	
	return 0;

}

/************************************************************************/
/* RSCore                                                               */
/************************************************************************/

RSCore* RSCore::init()
{

	initialized = FALSE;
	is_open = FALSE;
	hRGSSPlayer = FindWindow("RGSS Player", NULL);

	return this;
}

void RSCore::initWithBitmap()
{
	hBitmap = (HBITMAP*)malloc(sizeof(HBITMAP) * g_nMaxFrame);
}

void RSCore::create()
{
	// 해상도 값을 얻는다.
	windowSize.x = GetSystemMetrics(SM_CXSCREEN);
	windowSize.y = GetSystemMetrics(SM_CYSCREEN);

	// 창 구조체를 만듭니다.
	WNDCLASSEX wx;
	wx.cbClsExtra = 0;
	wx.cbSize = sizeof(WNDCLASSEX);
	wx.cbWndExtra = 0;
	wx.hbrBackground = NULL;
	wx.hCursor = (HCURSOR)LoadCursor(0, IDC_ARROW);
	wx.hIcon = LoadIcon(0, IDI_APPLICATION);
	wx.hIconSm = LoadIcon(0, IDI_APPLICATION);
	wx.hInstance = g_hDllHandle;
	wx.lpfnWndProc = WndProc;
	wx.lpszClassName = CLASS_NAME;
	wx.lpszMenuName = NULL;
	wx.style = CS_HREDRAW | CS_VREDRAW;

	if (SUCCEEDED(GetClassInfoEx(g_hDllHandle, CLASS_NAME, &wx))) {
		UnregisterClass(CLASS_NAME, g_hDllHandle);
	}

	RegisterClassEx(&wx);

	// 창을 생성합니다.
	hSubWindow = CreateWindowEx(RS_EX_STYLE, CLASS_NAME, WINDOW_NAME, WS_POPUP | WS_VISIBLE,
		0, 0, windowSize.x, windowSize.y, NULL, NULL, g_hDllHandle, NULL);

	LONG style = GetWindowLong(hSubWindow, GWL_STYLE);
	LONG ex_style = GetWindowLong(hSubWindow, GWL_EXSTYLE);

	SetWindowLong(hSubWindow, GWL_STYLE,
		style & ~(WS_CAPTION | WS_THICKFRAME));
	SetWindowLong(hSubWindow, GWL_EXSTYLE,
		ex_style & ~(WS_EX_DLGMODALFRAME |
			WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));

	BOOL bSuccess = SetLayeredWindowAttributes(hSubWindow, RS_TRANSPARENT_COLOR, 0, LWA_COLORKEY);
	if (!bSuccess) {
		RSErrorHandling(hSubWindow);
	}

	WNDCLASS wndClass;

	// 서브 클래싱
	HINSTANCE hInstance = (HINSTANCE)GetWindowLong(hRGSSPlayer, GWL_HINSTANCE);
	OldProc = (WNDPROC)SetWindowLong(hRGSSPlayer, GWL_WNDPROC, (LONG)SuperProc);


}

void RSCore::open()
{
	// 창을 원점으로 이동시킵니다.
	SetWindowPos(hSubWindow, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);

	// 포커스없이 창을 표시합니다.
	ShowWindow(hSubWindow, SW_SHOWNOACTIVATE);
	UpdateWindow(hSubWindow);

	// 뮤텍스 생성
	hMutex = CreateMutex(NULL, FALSE, NULL);
	
	// 쓰레드를 생성합니다.
	hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, NULL, 0, NULL);

	// 초기화 완료
	initialized = TRUE;

	is_open = TRUE;

}

void RSCore::close()
{
	initialized = FALSE;

	if (IsWindow(hSubWindow) == true) {
		// dll로 창 클래스를 등록하면 프로세스 종료 시 자동으로 언로드 되지 않습니다.
		UnregisterClass(CLASS_NAME, g_hDllHandle);
		DestroyWindow(hSubWindow);
	}

	// 핸들 해제
	CloseHandle(hMutex);
	CloseHandle(hThread);

	is_open = FALSE;

}

void RSCore::removeBitmaps()
{
	int i;

	// 비트맵 해제
	for (i = 0; i < g_nMaxFrame; i++) {
			if(hBitmap[i] != NULL) {
			DeleteObject(hBitmap[i]);
		}
	}

	free(hBitmap);
	hBitmap = NULL;

}

void RSCore::remove()
{
	if (hBitmap != NULL) {
		removeBitmaps();
	}
}

/**
 * @link https://goo.gl/ykXFpK
 */
void RSCore::callDisposeFromRGSSPlayer()
{
	HMODULE RGSSSystemDLL = NULL;
	TCHAR DLLName[MAX_PATH];
	TCHAR RGSSSystemFilePath[MAX_PATH];
	TCHAR IniDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, IniDir);
	_tcsncat_s(IniDir, MAX_PATH, _T("\\game.ini"), MAX_PATH);

	GetPrivateProfileString(_T("Game"), _T("Library"), _T("RGSS104E.dll"), RGSSSystemFilePath, MAX_PATH, IniDir);
	RGSSSystemDLL = GetModuleHandle(RGSSSystemFilePath);

	typedef int(*RGSSEVAL)(char *);
	RGSSEVAL RGSSEval;
	RGSSEval = (RGSSEVAL)GetProcAddress(RGSSSystemDLL, "RGSSEval");
	if (RGSSEval)
	{
		RGSSEval("RSCore.dispose if not RSCore.disposed?");
	}

}

void RSCore::Update()
{
	ObjectUpdate();
}

void RSCore::ObjectUpdate()
{

}

void RSCore::RenderClear(HDC hdc, HWND hWnd, PAINTSTRUCT& ps)
{
	int width, height;
	RECT rt;
	HBRUSH OldBrush, CurrentBrush;

	width = GetSystemMetrics(SM_CXSCREEN);
	height = GetSystemMetrics(SM_CYSCREEN);

	hdc = BeginPaint(hWnd, &ps);
	SetRect(&rt, 0, 0, width, height);
	CurrentBrush = CreateSolidBrush(RS_TRANSPARENT_COLOR);
	OldBrush = (HBRUSH)SelectObject(hdc, CurrentBrush);
	FillRect(hdc, &rt, CurrentBrush);
	DeleteObject(SelectObject(hdc, OldBrush));
	EndPaint(hWnd, &ps);
}

void RSCore::Render(HDC hdc, HWND hWnd, PAINTSTRUCT& ps)
{
	if (!initialized)
		return;

	hdc = BeginPaint(hWnd, &ps);
	HBITMAP hBit;


	HDC MemDC, MemDC2;
	HBITMAP OldBitmap;
	HBRUSH OldBrush, CurrentBrush;
	int bx, by;
	BITMAP bit;

	MemDC2 = CreateCompatibleDC(hdc);

	MemDC = CreateCompatibleDC(hdc);
	hBit = CreateCompatibleBitmap(hdc, windowSize.x, windowSize.y);
	SelectObject(MemDC, hBit);
	OldBitmap = (HBITMAP)SelectObject(MemDC2, g_prsCore->hBitmap[g_frame]);
	SetBkMode(hdc, OPAQUE);

	GetObject(g_prsCore->hBitmap[g_frame], sizeof(BITMAP), &bit);
	bx = bit.bmWidth;
	by = bit.bmHeight;

	// 기본 모니터 정보 가져 오기
	POINT ptZero = { 0 };
	HMONITOR hmonPrimary = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO monitorinfo = { 0 };
	monitorinfo.cbSize = sizeof(monitorinfo);
	GetMonitorInfo(hmonPrimary, &monitorinfo);

	printf_s("현재 프레임 : %d \n", g_frame);

	// 주 작업 영역의 중앙에 시작 화면 놓기
	const RECT & rcWork = monitorinfo.rcWork;
	POINT ptOrigin;

	// 작업 표시줄의 높이
	const RECT& __rect = monitorinfo.rcMonitor;
	int x_offset = (__rect.right - __rect.left) - (rcWork.right - rcWork.left);
	int y_offset = (__rect.bottom - __rect.top) - (rcWork.bottom - rcWork.top);

	printf_s("비트맵의 폭 : %d\n", bx);
	printf_s("비트맵의 높이 : %d\n", by);
	printf_s("작업 표시줄의 폭 : %d\n", x_offset);
	printf_s("작업 표시줄의 높이 : %d\n", y_offset);
	printf_s("g_prsCore->windowSize.y : %d\n", g_prsCore->windowSize.y);
	printf_s("rcWork.bottom : %d \n", rcWork.bottom);
	printf_s("rcWork.left : %d \n", rcWork.left);
	printf_s("rcWork.top : %d \n", rcWork.top);
	printf_s("rcWork.right : %d \n", rcWork.right);

	int m_width = (FLOAT)((FLOAT)( windowSize.x ) / (FLOAT)bx ) * bx;
	int m_height = (FLOAT)((FLOAT)( windowSize.y ) / (FLOAT)by ) * by;

	ptOrigin.x = rcWork.left + (windowSize.x / 2) - (m_width / 2);
	ptOrigin.y = rcWork.top + (windowSize.y / 2) - (m_height / 2);

	printf_s("그림의 좌표 : %d %d \n", ptOrigin.x, ptOrigin.y);

	//SetGraphicsMode(MemDC, GM_ADVANCED);
	//SetWorldTransform(MemDC, &transform);

	RECT rt;
	SetRect(&rt, 0, 0, g_prsCore->windowSize.x, g_prsCore->windowSize.y);

	// 흰색으로 채우기
	CurrentBrush = CreateSolidBrush(RS_TRANSPARENT_COLOR);
	OldBrush = (HBRUSH)SelectObject(MemDC, CurrentBrush);
	FillRect(MemDC, &rt, CurrentBrush);

	SetStretchBltMode(MemDC, COLORONCOLOR);
	StretchBlt(MemDC, ptOrigin.x, ptOrigin.y, m_width, m_height, MemDC2, 0, 0, bx, by, SRCCOPY);
	BitBlt(hdc, 0, 0, windowSize.x, windowSize.y, MemDC, 0, 0, SRCCOPY);

	//// 트랜스폼 복구
	//XFORM normalTransform = { 1, 0, 0, 1, 0, 0 };
	//SetGraphicsMode(MemDC, GM_ADVANCED);
	//SetWorldTransform(MemDC, &normalTransform);

	DeleteObject(SelectObject(hdc, OldBrush));
	DeleteObject(SelectObject(MemDC, OldBitmap));
	DeleteObject(hBit);
	DeleteDC(MemDC2);
	DeleteDC(MemDC);
	
	EndPaint(hWnd, &ps);

}

void RSCore::RenderPresent()
{

}