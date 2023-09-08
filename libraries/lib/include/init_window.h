#ifndef INIT_WINDOW_H
#define INIT_WINDOW_H

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct nk_context*
initEGLWindow(const char* title, int width, int height, bool vsync);

struct nk_context*
initFullscreenEGLWindow(const char* title, bool vsync);

struct nk_context*
initHeadlessEGLWindow(const char* title, int width, int height, bool vsync);

void
uninitEGLWindow();

void
displayDispatch();

void
RefreshWindow();

int
ctrl_key_clicks(char key);

struct nk_context*
vpkui_window_create();

#ifdef __cplusplus
}
#endif

#endif /* INIT_WINDOW_H */
