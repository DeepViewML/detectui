#include "shared.h"
#include <GLES2/gl2.h>
#define GL_BGRA 0x80E1

struct Label {
    float u0, v0, u1, v1;
    float x0, y0, x1, y1;
    float x_delta;
};

struct Score_Labels {
    struct Label* labels;
};

#ifdef __cplusplus
extern "C" {
#endif

void
uninit_fonts(struct Score_Labels* labels);

struct Score_Labels*
init_fonts(float font_size, int width, int height, unsigned int texture_id);

#ifdef __cplusplus
}
#endif