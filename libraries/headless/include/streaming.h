#ifndef STREAMING_INCLUDED
#define STREAMING_INCLUDED 1

#include <gbm.h>

#ifdef __cplusplus
extern "C" {
#endif

void
streaming_start(unsigned int width, unsigned int height);
void
streaming_stop();
void
streaming_send_frame(struct gbm_bo* gbm_buffer);

#ifdef __cplusplus
}
#endif

#endif