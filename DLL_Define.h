/**
 * @file DLL_Define.h
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
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif 

#include <Windows.h>
#include <process.h>
#include <tchar.h>
#include "Constants.h"

#define WIN32_LEAN_AND_MEAN

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "libzlib.lib")
#pragma comment(lib, "libpng16.lib")
#pragma comment(lib, "Msimg32.lib")

#define RS_EX_STYLE WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_COMPOSITED 
#define RS_WND_STYLE WS_POPUP

// 투명색 설정
#define RS_TRANSPARENT_COLOR RGB(0, 255, 0)

#ifdef _RSDLL
#define RSDLL __declspec(dllexport)
#else
#define RSDLL __declspec(dllimport)
#endif

	/**
	 * 다음 RGSSRGBA, RGSSBMINFO, RGSSBITMAP 구조체 정보는 아래 사이트에서 참조하였습니다
	 * link : https://github.com/scanti/RPG-Maker-Bitmap-Functions
	 */
	typedef union {
		unsigned int pixel;
		struct {
			unsigned char blue;
			unsigned char green;
			unsigned char red;
			unsigned char alpha;
		};
	} RGSSRGBA;

	typedef struct {
		DWORD unk1;
		DWORD unk2;
		BITMAPINFOHEADER *infoheader;
		RGSSRGBA *firstRow;
		RGSSRGBA *lastRow;
	} RGSSBMINFO;

	typedef struct {
		DWORD unk1;
		DWORD unk2;
		RGSSBMINFO *bminfo;
	} BITMAPSTRUCT;

	typedef struct {
		DWORD flags;
		DWORD klass;
		void(*dmark) (void*);
		void(*dfree) (void*);
		BITMAPSTRUCT *bm;
	} RGSSBITMAP;

	struct TextureData;

	struct RSCore
	{
		HWND hSubWindow;		/** 서브 윈도우의 핸들 */
		HWND hRGSSPlayer;		/** RGSS Player의 핸들 */
		HANDLE hThread;			/** 쓰레드의 핸들 */
		HANDLE hMutex;			/** 뮤텍스의 핸들 */
		POINT windowSize;		/** 창 크기 */
		BOOL initialized;		/** 초기화 여부 */

		HBITMAP *hBitmap;

		/**
		* 프레임 타임 관리
		*/
		BOOL is_open;

		TextureData *m_pTextureData;

		RSCore* init();
		void initWithBitmap();
		void create();
		void open();
		void close();
		void removeBitmaps();
		void remove();
		void callDisposeFromRGSSPlayer();

		void Update();
		void ObjectUpdate();
		void RenderClear(HDC hdc, HWND hWnd, PAINTSTRUCT& ps);
		void Render(HDC hdc, HWND hWnd, PAINTSTRUCT& ps);
		void RenderPresent();

	};

	RSDLL void RSInitWithCoreSystem();
	RSDLL void RSRemoveCoreSystem();
	RSDLL void RSInitWithBitmap();
	RSDLL void RSRemoveBitmap();

	RSDLL void RSCreateWindow(void);
	RSDLL void RSOpenWindow(void);
	RSDLL void RSCloseWindow(void);

	RSDLL void RSDebugLog(char* s);

	RSDLL void RSSetMaxFrame(int n);

	RSDLL BOOL RSWindowIsOpen(void);

	RSDLL void RSEnableDebugLog(void);
	RSDLL void RSDisableDebugLog(void);

	RSDLL void RSSetFrameDelay(int frame_id, int delay);

	RSDLL void RSSetStartEndFrame(int start_frame, int end_frame);

	RSDLL void RSCreateDirectory(char* path);

	RSDLL void RSErrorHandling(HWND hWnd);

	RSDLL void RSSnapToBitmap(unsigned int object);

	/**
	 * RGSSBITMAP에서 윈도우즈 호환 비트맵으로 변환합니다.
	 * @param object 비트맵의 object_id
	 */
	RSDLL HBITMAP ConvertRGSSBitmapToBitmap(unsigned int object);

	/**
	 * 프레임을 설정합니다.
	 * @param id 프레임 ID
	 * @param object 비트맵의 id 값
	 */
	RSDLL void RSSetFrame(int id, unsigned int object);

	LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	unsigned WINAPI ThreadFunc(LPVOID arg);

#ifdef __cplusplus
}
#endif