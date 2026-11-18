#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2s_std.h"
#include "driver/gpio.h"

#define SAMPLE_RATE    48000
#define SINE_FREQ      5000
#define PI             3.14159265358979f
#define AMPLITUDE      28000

static i2s_chan_handle_t tx_chan;

static void sine_task(void *arg)
{
    const int samples_per_cycle = SAMPLE_RATE / SINE_FREQ;
    const int channels = 2;

    int16_t *buffer =
        malloc(samples_per_cycle * channels * sizeof(int16_t));

    if (buffer == NULL)
    {
        printf("Buffer allocation failed!\n");
        vTaskDelete(NULL);
    }

    /* Generate one complete sine cycle */
    for (int n = 0; n < samples_per_cycle; n++)
    {
        float phase =
            (2.0f * PI * n) / samples_per_cycle;

        int16_t sample =
            (int16_t)(AMPLITUDE * sinf(phase));

        buffer[2 * n]     = sample;   /* Left */
        buffer[2 * n + 1] = sample;   /* Right */
    }

    size_t bytes_per_cycle =
        samples_per_cycle * channels * sizeof(int16_t);

    while (1)
    {
        size_t bytes_written;

        i2s_channel_write(
            tx_chan,
            buffer,
            bytes_per_cycle,
            &bytes_written,
            portMAX_DELAY);
    }
}

void app_main(void)
{
    i2s_chan_config_t chan_cfg =
        I2S_CHANNEL_DEFAULT_CONFIG(
            I2S_NUM_0,
            I2S_ROLE_MASTER);

    i2s_new_channel(
        &chan_cfg,
        &tx_chan,
        NULL);

    i2s_std_config_t std_cfg =
    {
        .clk_cfg =
            I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),

        .slot_cfg =
            I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
                I2S_DATA_BIT_WIDTH_16BIT,
                I2S_SLOT_MODE_STEREO),

        .gpio_cfg =
        {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = GPIO_NUM_4,
            .ws   = GPIO_NUM_5,
            .dout = GPIO_NUM_18,
            .din  = I2S_GPIO_UNUSED,

            .invert_flags =
            {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false
            }
        }
    };

    i2s_channel_init_std_mode(
        tx_chan,
        &std_cfg);

    i2s_channel_enable(tx_chan);

    xTaskCreate(
        sine_task,
        "sine_task",
        4096,
        NULL,
        5,
        NULL);

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}