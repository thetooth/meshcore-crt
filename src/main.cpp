#include <Arduino.h>

#include "pico/stdlib.h"
#include <stdio.h>
#include <stdlib.h>

extern "C"
{
#include "connections.h"
#include "cvideo.h"
#include "renderer.h"
}

#define SCREEN_WIDTH CVIDEO_PIX_PER_LINE
#define SCREEN_HEIGHT CVIDEO_LINES

// bool pong_gametick_callback(struct repeating_timer *t)
// {
//     pong_tick();
//     return true;
// }

uint64_t current_time = 0;
static char text_buffer[128];

int xOffset = 0;
int yOffset = 0;
void draw(void)
{
    xOffset = sin(current_time / 10.0) * 80.0;
    yOffset = cos(current_time / 10.0) * 60.0;

    sprintf(text_buffer, "Lil pico boi");
    renderer_draw_string(SCREEN_WIDTH / 2 + xOffset, SCREEN_HEIGHT / 2 + yOffset, 2, text_buffer, strlen(text_buffer),
                         JUSTIFY_CENTRE);
    current_time++;
    sprintf(text_buffer, "%llu", current_time);
    renderer_draw_string(SCREEN_WIDTH / 2 - xOffset, SCREEN_HEIGHT / 2 + 30 - yOffset, 1, text_buffer,
                         strlen(text_buffer), JUSTIFY_CENTRE);
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Pi Pico Pong\r\n");
    Serial.println("Created by Alan Reed\r\n");

    renderer_init(draw);

    // struct repeating_timer timer;
    // add_repeating_timer_ms(PONG_FRAME_INTERVAL_ms, pong_gametick_callback, NULL, &timer);

    while (1)
    {
        renderer_run();
    }
}

void loop()
{
}