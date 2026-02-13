#ifndef __CVIDEO_H__
#define __CVIDEO_H__

#include "hardware/pio.h"

#ifndef CVIDEO_INTERLACED
#define CVIDEO_INTERLACED 0
#endif

// Pixels per line should be a multiple of 32
// For very high or very low pixel counts, DATA_DELAY within cvideo.pio may need adjustment
#if CVIDEO_INTERLACED
#define CVIDEO_LINES        576
#define CVIDEO_PIX_PER_LINE 768
#else
#define CVIDEO_LINES        288
#define CVIDEO_PIX_PER_LINE 384
#endif

typedef uint32_t (*cvideo_data_callback_t)(void);

void cvideo_init(PIO pio, uint data_pin, uint sync_pin, cvideo_data_callback_t callback);

#endif