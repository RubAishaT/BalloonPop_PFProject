#ifndef FAKE_WINDOWS_H
#define FAKE_WINDOWS_H
#include <stdint.h>

typedef unsigned int COLORREF;
typedef void *HDC, *HFONT, *HBRUSH, *HPEN, *HGDIOBJ;
typedef long LONG;
typedef struct { LONG x, y; } POINT;
typedef struct { LONG left, top, right, bottom; } RECT;
typedef struct { LONG cx, cy; } SIZE;

#define RGB(r, g, b)   ((COLORREF)(((unsigned)(r) & 255) | (((unsigned)(g) & 255) << 8) | (((unsigned)(b) & 255) << 16)))
#define GetRValue(c)   ((int)((c) & 255))
#define GetGValue(c)   ((int)(((c) >> 8) & 255))
#define GetBValue(c)   ((int)(((c) >> 16) & 255))

#define TRANSPARENT 1
#define PS_SOLID 0
#define NULL_BRUSH 5
#define NULL_PEN 8
#define FW_NORMAL 400
#define FW_BOLD 700
#define DEFAULT_CHARSET 1
#define OUT_DEFAULT_PRECIS 0
#define CLIP_DEFAULT_PRECIS 0
#define ANTIALIASED_QUALITY 4
#define DEFAULT_PITCH 0
#define SND_ASYNC 1
#define SND_MEMORY 4
typedef const char *LPCSTR;

HBRUSH CreateSolidBrush(COLORREF c);
HPEN CreatePen(int style, int width, COLORREF c);
HFONT CreateFont(int h, int w, int e, int o, int weight, unsigned long i, unsigned long u,
                 unsigned long s, unsigned long cs, unsigned long op, unsigned long cp,
                 unsigned long q, unsigned long pf, const char *face);
HGDIOBJ GetStockObject(int which);
HGDIOBJ SelectObject(HDC dc, HGDIOBJ obj);
int DeleteObject(HGDIOBJ obj);
int SetBkMode(HDC dc, int mode);
COLORREF SetTextColor(HDC dc, COLORREF c);
int FillRect(HDC dc, const RECT *r, HBRUSH b);
int Ellipse(HDC dc, int l, int t, int r, int b);
int RoundRect(HDC dc, int l, int t, int r, int b, int ew, int eh);
int Polygon(HDC dc, const POINT *pts, int n);
int MoveToEx(HDC dc, int x, int y, POINT *old);
int LineTo(HDC dc, int x, int y);
int TextOut(HDC dc, int x, int y, const char *s, int n);
int GetTextExtentPoint32(HDC dc, const char *s, int n, SIZE *size);
void PostQuitMessage(int code);
int PlaySound(LPCSTR sound, void *module, unsigned long flags);


void shim_begin(uint32_t *pixels, int w, int h);   
HDC  shim_dc(void);
int  shim_beeps(void);
const void *shim_last_sound(void);
int  shim_quit_requested(void);
#endif
