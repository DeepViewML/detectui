#include "models.h"
#include "vpkui.h"

const char*  timing_window_title = "Timing Data ";
static char* device_name         = (char*) "/dev/video3";
char* engine = (char*) "npu"; // Allow the user to pick the processing unit
                              // between CPU/GPU/NPU
int          camera_width    = 1920;
int          camera_height   = 1080;
static float score_threshold = 0.7;
static float iou_threshold   = 0.5;
static bool  mirror          = false;
static bool  mirror_v        = false;
static bool  verbose         = false;
struct Color myColor = {1.0f, 1.0f, 1.0f, 1.0f}; // Sets the color for the boxes
                                                 // drawn as well as the label

const char* usage = "\
\n\
USAGE: \n\
    %s\n\
        -h, --help\n\
            Display help information\n\
        -v --version\n\
            Display application version\n\
        -V, --verbose\n\
            Print timings while running\n\
        -d, --capture_device\n\
            The capture device for streaming. (default /dev/video3)\n\
        -e, --engine\n\
            Compute engine type \"cpu\", \"npu\". (default npu)\n\
        -t, --threshold \n\
            Threshold for valid scores, (default 0.7)\n\
        -m, --mirror\n\
            Mirrors the camera\n\
        -f, --mirror_v\n\
            Mirrors the camera vertically\n\
        -u, --iou \n\
            IOU threshold for NMS, (default 0.5)\n\
\n\
";

/**
 * The entry point of the program.
 *
 * @param argc The number of arguments passed to the program.
 * @param argv An array of strings representing the arguments passed to the
 * program.
 *
 * @return An integer value representing the exit status of the program.
 *
 */
int
main(int argc, char** argv)
{
    static struct option options[] = {
        {"help", no_argument, NULL, 'h'},
        {"version", no_argument, NULL, 'v'},
        {"verbose", no_argument, NULL, 'V'},
        {"capture_device", required_argument, NULL, 'd'},
        {"threshold", required_argument, NULL, 't'},
        {"mirror", no_argument, NULL, 'm'},
        {"mirror_v", no_argument, NULL, 'f'},
        {"iou", required_argument, NULL, 'u'},
        {"engine", required_argument, NULL, 'e'},
        {NULL, 0, NULL, 0},
    };

    for (int i = 0; i < 100; i++) {
        int opt = getopt_long(argc, argv, "hvVd:fmt:u:e:", options, NULL);
        if (opt == -1) break;
        switch (opt) {
        case 'h': {
            printf(usage, argv[0]);
            return EXIT_SUCCESS;
        }
        case 'v': {
            printf("facedetect v%s\n", GIT_LATEST_TAG);
            return EXIT_SUCCESS;
        }
        case 'V': {
            verbose = true;
            break;
        }
        case 'd': {
            device_name = optarg;
            break;
        }
        case 't': {
            score_threshold = atof(optarg);
            break;
        }
        case 'u': {
            iou_threshold = atof(optarg);
            break;
        }
        case 'e': {
            engine = optarg;
            if (strcmp(engine, "NULL") == 0) { return EXIT_FAILURE; }
            break;
        }
        case 'm': {
            mirror = !mirror;
            break;
        }
        case 'f': {
            mirror_v = !mirror_v;
            break;
        }
        default: {
            printf(usage, argv[0]);
            return EXIT_SUCCESS;
        }
        }
    }

    int frame_load_method = FRAME_LOAD_METHOD_DMA;

    struct nk_context* nk_ctx =
        vpkui_window_create(); // Creats the VPKUI Window and a nuklear context
                               // for timing details

    signal(SIGINT, vpkui_sig_handler);
    vpkui_init_color();

#ifdef HEADLESS // Allows for web streaming for units without HDMI output
    streaming_start(WINDOW_WIDTH, WINDOW_HEIGHT);
#endif

    vsl_camera* camera           = vsl_camera_open_device(device_name);
    uint32_t    cam_fourcc       = make_fourcc('Y', 'U', 'Y', 'V');
    uint32_t    requested_fourcc = cam_fourcc;
    int         buf_count        = 4;

    int err = vsl_camera_init_device(camera,
                                     &camera_width,
                                     &camera_height,
                                     &buf_count,
                                     &cam_fourcc);
    if (err || cam_fourcc != requested_fourcc) {
        printf("Could not initialize device to stream in YUYV\n");
        vsl_camera_close_device(camera);
        return -1;
    }

    vsl_camera_mirror(camera, mirror);
    vsl_camera_mirror_v(camera, mirror_v);
    vsl_camera_start_capturing(camera);
    if ((frame_load_method == FRAME_LOAD_METHOD_PHYS ||
         frame_load_method == FRAME_LOAD_METHOD_DMA) &&
        !vsl_camera_is_dmabuf_supported(camera)) {
        printf(
            "DMA is not supported, fallback to mmap buffers\n"); // Add warning
        frame_load_method = FRAME_LOAD_METHOD_MEM;
    }
    printf("Init camera\n");

    void*  model      = facedetect_rtm;
    size_t model_size = facedetect_rtm_len;

    size_t       num_boxes = 0;
    VAALBox*     boxes     = calloc(25, sizeof(VAALBox));
    VAALContext* ctx       = vaal_context_create(engine);

    vaal_parameter_setf(ctx, "score_threshold", &score_threshold, 1);
    vaal_parameter_setf(ctx, "iou_threshold", &iou_threshold, 1);

    // vaal_load_model_file(ctx, "/apps/facedetect.rtm"); // Model is loaded
    // from a file
    vaal_load_model(ctx, model_size,
                    model); // Embedded model is loaded.
    printf("Model Loaded\n");
    struct vsl_camera_buffer* last_buffer = 0;
    int64_t                   start_time  = vaal_clock_now();
    float draw_FPS = 0, inference_FPS = 0, camera_FPS = 0, avg_get_im_data = 0,
          avg_draw_im_data = 0, avg_draw_box_time = 0, avg_overlay_time = 0,
          avg_nk_build_time = 0, avg_nk_draw_time = 0, avg_swap_buf_time = 0,
          avg_clear_buf_time = 0, avg_load_frame_time = 0,
          avg_inference_time = 0, avg_decode_time = 0;
    while (get_keep_running()) {

        int     counter                = 0;
        int     camera_frames          = 0;
        int     inference_frames       = 0;
        int64_t start                  = vaal_clock_now();
        long    framebuffer_clear_time = 0;
        long    get_im_data_time       = 0;
        long    display_im_data_time   = 0;
        long    display_box_time       = 0;
        long    swap_buf_time          = 0;
        long    inference_time         = 0;
        long    decode_time            = 0;
        long    load_frame_time        = 0;
        long    draw_time              = 0;
        long    overlay_time           = 0;
        long    nk_build_time          = 0;
        long    nk_draw_time           = 0;

        while (counter < 30 && get_keep_running() &&
               vaal_clock_now() - start < 0.5 * NSEC_PER_SEC) {
            vpkui_draw_clear(); // Clears the drawn screen
            vpkui_show_trial_duration(start_time);
            struct vsl_camera_buffer* buffer =
                vsl_camera_get_data(camera); // Gets the camera data
            if (buffer) {
                if (last_buffer) vsl_camera_release_buffer(camera, last_buffer);
                last_buffer = buffer;
                camera_frames++;
            }
            int64_t timing_start = vaal_clock_now();
            if (last_buffer) {
                vpkui_draw_camera(last_buffer, camera_width, camera_height);
                display_im_data_time += vaal_clock_now() - timing_start;
                vaal_load_frame_physical(ctx,
                                         NULL,
                                         vsl_camera_buffer_phys_addr(
                                             last_buffer),
                                         cam_fourcc,
                                         camera_width,
                                         camera_height,
                                         NULL,
                                         0);
                timing_start = vaal_clock_now();
                vaal_run_model(ctx);
                inference_time += (vaal_clock_now() - timing_start);
                inference_frames++;
                vaal_boxes(ctx, boxes, 25, &num_boxes);
            }
            timing_start = vaal_clock_now();
            vpkui_draw_boxes(num_boxes, boxes, myColor, 10);
            display_box_time += vaal_clock_now() - timing_start;

            timing_start = vaal_clock_now();
            vpkui_draw_overlay_image();
            overlay_time += vaal_clock_now() - timing_start;

            if (nk_begin(nk_ctx,
                         timing_window_title,
                         nk_rect(WINDOW_WIDTH - 300, 0, 300, 430),
                         NK_WINDOW_BORDER | NK_WINDOW_MOVABLE |
                             NK_WINDOW_SCALABLE | NK_WINDOW_MINIMIZABLE |
                             NK_WINDOW_TITLE)) {

                const float ratio[] = {0.7, 0.3};
                nk_layout_row(nk_ctx, NK_DYNAMIC, 20, 1, ratio);
                nk_label(nk_ctx, "Display", NK_TEXT_ALIGN_LEFT);
                nk_layout_row(nk_ctx, NK_DYNAMIC, 20, 2, ratio);
                nk_label(nk_ctx, "    FPS", NK_TEXT_ALIGN_LEFT);
                nk_labelf(nk_ctx, NK_TEXT_ALIGN_RIGHT, "%.2f", draw_FPS);
                nk_label(nk_ctx, "    Resolution", NK_TEXT_ALIGN_LEFT);
                nk_labelf(nk_ctx,
                          NK_TEXT_ALIGN_RIGHT,
                          "%ix%i",
                          WINDOW_WIDTH,
                          WINDOW_HEIGHT);

                nk_layout_row(nk_ctx, NK_DYNAMIC, 20, 1, ratio);
                nk_label(nk_ctx, "Camera", NK_TEXT_ALIGN_LEFT);
                nk_layout_row(nk_ctx, NK_DYNAMIC, 20, 2, ratio);
                nk_label(nk_ctx, "    FPS", NK_TEXT_ALIGN_LEFT);
                nk_labelf(nk_ctx, NK_TEXT_ALIGN_RIGHT, "%.2f", camera_FPS);
                nk_label(nk_ctx, "    Resolution", NK_TEXT_ALIGN_LEFT);
                nk_labelf(nk_ctx,
                          NK_TEXT_ALIGN_RIGHT,
                          "%ix%i",
                          camera_width,
                          camera_height);

                nk_layout_row(nk_ctx, NK_DYNAMIC, 20, 1, ratio);
                nk_label(nk_ctx, "Inference", NK_TEXT_ALIGN_LEFT);
                nk_layout_row(nk_ctx, NK_DYNAMIC, 20, 2, ratio);
                nk_label(nk_ctx, "    FPS", NK_TEXT_ALIGN_LEFT);
                nk_labelf(nk_ctx, NK_TEXT_ALIGN_RIGHT, "%.2f", inference_FPS);

                nk_layout_row_dynamic(nk_ctx, 10, 1);
                nk_layout_row(nk_ctx, NK_DYNAMIC, 20, 2, ratio);

                nk_label(nk_ctx, "Draw camera frame", NK_TEXT_ALIGN_LEFT);
                nk_labelf(nk_ctx,
                          NK_TEXT_ALIGN_RIGHT,
                          "%.2f",
                          avg_draw_im_data);
                nk_label(nk_ctx, "Draw Boxes", NK_TEXT_ALIGN_LEFT);
                nk_labelf(nk_ctx,
                          NK_TEXT_ALIGN_RIGHT,
                          "%.2f",
                          avg_draw_box_time);
                nk_label(nk_ctx, "Draw overlay", NK_TEXT_ALIGN_LEFT);
                nk_labelf(nk_ctx,
                          NK_TEXT_ALIGN_RIGHT,
                          "%.2f",
                          avg_overlay_time);
                nk_label(nk_ctx, "Draw GUI", NK_TEXT_ALIGN_LEFT);
                nk_labelf(nk_ctx,
                          NK_TEXT_ALIGN_RIGHT,
                          "%.2f",
                          avg_nk_draw_time);
                nk_label(nk_ctx, "Swap framebuffer", NK_TEXT_ALIGN_LEFT);
                nk_labelf(nk_ctx,
                          NK_TEXT_ALIGN_RIGHT,
                          "%.2f",
                          avg_swap_buf_time);
                nk_layout_row_dynamic(nk_ctx, 10, 1);
                nk_layout_row(nk_ctx, NK_DYNAMIC, 20, 2, ratio);
                nk_label(nk_ctx, "Run model inference", NK_TEXT_ALIGN_LEFT);
                if (!isnan(avg_inference_time)) {
                    nk_labelf(nk_ctx,
                              NK_TEXT_ALIGN_RIGHT,
                              "%.2f",
                              avg_inference_time);
                } else {
                    nk_labelf(nk_ctx, NK_TEXT_ALIGN_RIGHT, "0.0");
                }
            } else {
                // draw_timings = false;
            }
            nk_end(nk_ctx);
            timing_start = vaal_clock_now();
            nk_wayland_render(NK_ANTI_ALIASING_ON, 512 * 1024, 128 * 1024);
            nk_draw_time += vaal_clock_now() - timing_start;
            timing_start = vaal_clock_now();
            nk_input_begin(nk_ctx);
            vpkui_window_update(start_time);
            nk_input_end(nk_ctx);
            counter++;
            swap_buf_time += vaal_clock_now() - timing_start;
        }
        int64_t elapsed = vaal_clock_now() - start;
        draw_FPS        = (double) counter / elapsed * NSEC_PER_SEC;
        inference_FPS   = (double) inference_frames / elapsed * NSEC_PER_SEC;
        camera_FPS      = (double) camera_frames / elapsed * NSEC_PER_SEC;

        avg_draw_im_data =
            (double) display_im_data_time / NSEC_PER_SEC * 1000 / camera_frames;
        avg_draw_box_time =
            (double) display_box_time / NSEC_PER_SEC * 1000 / counter;
        avg_overlay_time =
            (double) overlay_time / NSEC_PER_SEC * 1000 / counter;
        avg_nk_draw_time =
            (double) nk_draw_time / NSEC_PER_SEC * 1000 / counter;
        avg_swap_buf_time =
            (double) swap_buf_time / NSEC_PER_SEC * 1000 / counter;
        avg_inference_time =
            (double) inference_time / NSEC_PER_SEC * 1000 / inference_frames;

        if (verbose) {

            printf("Took %.2f ms to display %i frames. Average FPS: %.2f\n",
                   (double) elapsed / NSEC_PER_SEC * 1000,
                   counter,
                   draw_FPS);
            printf("Took %.2f ms to inference %i frames. Average FPS: %.2f\n",
                   (double) elapsed / NSEC_PER_SEC * 1000,
                   inference_frames,
                   inference_FPS);
            printf("Took %.2f ms to capture %i camera frames. Average FPS: "
                   "%.2f\n",
                   (double) elapsed / NSEC_PER_SEC * 1000,
                   camera_frames,
                   camera_FPS);

            printf("Average frame time is:\t%.2f ms\n", 1000.0 / draw_FPS);

            printf("\tAverage %.2f ms to display camera image data.\n",
                   avg_draw_im_data);
            printf("\tAverage %.2f ms to display boxes and labels.\n",
                   avg_draw_box_time);
            printf("\tAverage %.2f ms to draw overlay.\n", avg_overlay_time);
            printf("\tAverage %.2f ms to draw GUI.\n", avg_nk_draw_time);
            printf("\tAverage %.2f ms to swap framebuffers.\n",
                   avg_swap_buf_time);
            printf("\t\t%.2f ms to run inference.\n", avg_inference_time);

            printf("-----------------------------------------------------------"
                   "----\n");
        }
    }
    // Below functions calls helps with the cleanup of the code and resources
    // once the program has been terminated
    vpkui_release_resources();
    vaal_context_release(ctx);
    free(boxes);

    vsl_camera_stop_capturing(camera);
    vsl_camera_uninit_device(camera);
    vsl_camera_close_device(camera);
    printf("Uninit camera!\n");
#ifdef HEADLESS
    streaming_stop();
#endif
}