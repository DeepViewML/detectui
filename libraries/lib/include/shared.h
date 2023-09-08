#pragma once

#ifndef MIN
#define MIN(a, b) (a < b ? a : b)
#endif

#ifndef MAX
#define MAX(a, b) (a > b ? a : b)
#endif

#define CLAMP01(x) MAX(0, MIN(x, 1))

#ifndef MAX_GL_BOXES
#define MAX_GL_BOXES 400
#endif

extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;