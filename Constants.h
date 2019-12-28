/**
 * @file Constants.h
 * @date 2018/03/26 11:19
 *
 * @author biud436
 * Contact: biud436@gmail.com
 *
 * @brief 
 *
 *
 * @note
*/

#ifndef Constants_h
#define Constants_h

/**
* @def WINDOW_NAME
*/
#define WINDOW_NAME	"rgssplayer"

/**
* @def CLASS_NAME
*/
#define CLASS_NAME	"rgssplayer"

/**
 * @def MAX_FRAME
 */
#define MAX_FRAME	4

/**
* @def WINDOW_WIDTH
*/
#define WINDOW_WIDTH	640

/**
* @def WINDOW_HEIGHT
*/
#define WINDOW_HEIGHT	480

const float FRAME_RATE = 240.0f;
const float MIN_FRAME_RATE = 10.0f;
const float MIN_FRAME_TIME = 1.0f / FRAME_RATE;
const float MAX_FRAME_TIME = 1.0f / MIN_FRAME_RATE;

/**
* @def PI
*/
#define PI				3.141592653589793

/**
* @def SPRITE_SHEET_COLS
* 스프라이트 시트 열 갯수
*/
#define SPRITE_SHEET_COLS	4

/**
* @def SPRITE_SHEET_COLS
* 스프라이트 시트 행 갯수
*/
#define SPRITE_SHEET_ROWS	4

/**
* @def SAFE_DELETE(p)
*/
#define SAFE_DELETE(p) {\
	if(( p )!=NULL) { \
		delete (p); \
		( p )=NULL; \
	} \
}

#define XP_CORE_MUTEX "RMXPCore-Standard-Mutex"

#endif