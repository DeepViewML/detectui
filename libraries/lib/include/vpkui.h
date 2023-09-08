/**
 * Copyright 2021 by Au-Zone Technologies.  All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential.
 *
 * Authorization to use this header is provided a part of DeepView VisionPack.
 *
 * @file vpkui.h
 */

#include "getopt.h"
#include "opengl.h"
#include "shared.h"
#include "streaming.h"
#include "vaal.h"
#include "videostream.h"
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define GL_GLEXT_PROTOTYPES 1
#include "init_window.h"
#include <EGL/egl.h>
#include <EGL/eglplatform.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#include "nuklear.h"
#include "nuklear_wayland_gles2.h"

#ifndef USEC_PER_SEC
#define USEC_PER_SEC 1000000ll /* microseconds per second */
#endif

#ifndef NSEC_PER_SEC
#define NSEC_PER_SEC (1000ll * USEC_PER_SEC)
#endif

#define MAX_COLORS 6
#define NSEC_PER_MIN (60l * NSEC_PER_SEC)
#define TRIAL_DURATION (15l * NSEC_PER_MIN)
#define FRAME_LOAD_METHOD_PHYS 0
#define FRAME_LOAD_METHOD_DMA 1
#define FRAME_LOAD_METHOD_MEM 2
#define SAMPLE_VERSION "1.2.1.20221104"
#define make_fourcc(a, b, c, d)                                        \
    ((uint32_t) (a) | ((uint32_t) (b) << 8) | ((uint32_t) (c) << 16) | \
     ((uint32_t) (d) << 24))

enum my_enum { headless_opengl = 0, wayland_opengl };

/**

* Returns the current system time in nanoseconds.

* @return An int64_t value representing the current system time in nanoseconds.
*/
int64_t
clock_now();

/**
 * Signal handler function for VPKUI for proper exiting.
 *
 * @param _ The signal number that triggered the handler.
 *
 * @return void.
 *
 */
void
vpkui_sig_handler(int _);

/**
 * Refreshes the screen with the new frame.
 *
 * @param start_time Keeps track of the time to skip frame rate
 *
 * @return void.
 *
 */
void
vpkui_window_update(int64_t start_time);

/**
 * Releases resources used by VPKUI, i.e. destroys the window and unints opengl.
 *
 * @return void.
 *
 */
void
vpkui_release_resources();

/**
 * Calculates and displays the duration of the trial.
 *
 * @param start_time The start time of the trial, in nanoseconds since an
 * unspecified epoch.
 *
 * @return void.
 *
 */
void
vpkui_show_trial_duration(int64_t start_time);

/**
 * Returns the current system time in nanoseconds.
 *
 * @return An int64_t value representing the current system time in nanoseconds.
 *
 */
int64_t
vpkui_clock_now();

/**
 * Initializes the color settings for VPKUI overlays and model results.
 *
 * @return void.
 *
 */
void
vpkui_init_color();

/**
 * Returns the current value of the 'keep_running' flag atomically. The
 * 'keep_running' flag is used to indicate whether the main loop of the program
 * should keep running or not.
 *
 * @return An sig_atomic_t value of the 'keep_running' flag.
 *
 */
sig_atomic_t
get_keep_running();

/**
 * Draws the overlay images on the stream
 *
 * @return void.
 *
 */
void
vpkui_draw_overlay_image();

/**
 * Draws bounding boxes for all the detections found using VAALBoxes.
 *
 * @param detections_founds The number of detections found.
 * @param detect An array of VAALBox structs containing the detection
 * information.
 * @param myColor An array of Color structs containing color of the box.
 * @param thickness A float containing the thickness of the box
 * information.
 *
 * @return void.
 *
 */
void
vpkui_draw_boxes(int          detections_founds,
                 VAALBox*     detect,
                 struct Color myColor,
                 float        thickness);

/**
 * Draws Skeleton for all the keypoints found using VAALKeypoints.
 *
 * @param detections_founds The number of detections found.
 * @param detect An array of VAALKeypoint structs containing the keypoint
 * information.
 *
 * @return void.
 *
 */
void
vpkui_draw_skeleton(int detections_founds, VAALKeypoint* detect);

/**
 *  * Draws blur boxes for all the detections found using VAALBoxes.
 *
 * @param detections_founds The number of detections found.
 * @param detect An array of VAALBox structs containing the detection
 * information.
 *
 * @return void.
 *
 */
void
vpkui_draw_boxes_blur(int detections_founds, VAALBox* detect);

/**
 *  * Draws shapes for all the detections found using VAALBoxes.
 *
 * @param detections_founds The number of detections found.
 * @param detect An array of VAALBox structs containing the detection
 * information.
 *
 * @return void.
 *
 */
void
vpkui_draw_shapes(int detections_founds, VAALBox* detect);

/**
 *  * Draws eulers pointing in XYZ axis for all the eulers found using
 * VAALEuler.
 *
 * @param detections_founds The number of detections found.
 * @param detect An array of VAALBox structs containing the detection
 * information.
 * @param euler An array of VAALEuler structs containing the euler
 * information.
 *
 * @return void.
 *
 */
void
vpkui_draw_euler(int detections_founds, VAALBox* detect, VAALEuler* euler);

/**
 * Draws a segmentation mask on the screen
 *
 * @param mask The segmentation mask.
 * @param width The width of the mask
 * @param height The height of the mask
 *
 * @return void.
 *
 */
void
vpkui_draw_segmentation(uint8_t* mask, int32_t width, int32_t height);

/**
 * Draws camera frame using the last buffer based on the hardware
 * acceleration avaliable.
 *
 * @param last_buffer The last frame found.
 * @param camera_width Width of the camera frame
 * @param camera_height Height of the camera frame
 *
 * @return void.
 *
 */
void
vpkui_draw_camera(struct vsl_camera_buffer* last_buffer,
                  int                       camera_width,
                  int                       camera_height);
/**
 * Clears the screen from all the boxes, shapes and overlays.
 *
 * @return void.
 *
 */
void
vpkui_draw_clear();