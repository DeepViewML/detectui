#include "shared.h"

struct Vertex {
    float x, y, z;
};
typedef struct Vertex Vertex;

struct TexCoord {
    float x, y;
};

struct Color {
    float r, g, b, a;
};

#ifdef __cplusplus
extern "C" {
#endif

int
init_opengl();
void
print_opengl_version_info();
void
init_overlay();
void
init_labels();
void
clear_draw();
int
draw_camera(void* buffer, int camera_width, int camera_height);
void
clear_labels();
void
clear_boxes();

void
build_box(float x0, float y0, float x1, float y1, struct Color color, float thickness);

Vertex
build_string(const char*  str,
             float        x0,
             float        y0,
             struct Color color,
             float        scale);
Vertex
build_score(float score, float x0, float y0, struct Color color, float scale);

void
build_box_score(float        score,
                float        x0,
                float        y0,
                float        x1,
                float        y1,
                struct Color color,
                float        scale);
void
build_label_score(const char*  label,
                  float        score,
                  float        x0,
                  float        y0,
                  struct Color color,
                  float        scale);
void
build_box_label(const char*  label,
                float        x0,
                float        y0,
                float        x1,
                float        y1,
                struct Color color,
                float        scale);
void
build_box_label_score(const char*  label,
                      float        score,
                      float        x0,
                      float        y0,
                      float        x1,
                      float        y1,
                      struct Color color,
                      float        scale,
                      float        thickness);

void
draw_built_labels_scores();
void
clear_labels();
void
draw_built_boxes();
void
draw_built_boxes_as_blur();
void
clear_boxes();
void
build_rect(float x0, float y0, float x1, float y1, struct Color color);
void
draw_head_pose(float yaw, float pitch, float roll, Vertex center, float size);
void
clear_shapes();
void
draw_shapes();
void
make_segmentation(unsigned char* segment, int width, int height, struct Color* colors);
void
draw_segmentation();
void
draw_overlay();
void
uninit_gl();
void
normalized_to_gl_coords_float(float* x, float* y);

void
draw_skeleton(int*  connections,
              void* values,
              float threshold,
              int   connections_size);

#ifdef __cplusplus
}
#endif