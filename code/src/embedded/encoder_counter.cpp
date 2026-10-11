#ifndef NATIVE

#include "embedded/encoder_counter.h"

#include <Arduino.h>
#include <driver/gpio.h>
#include <driver/pcnt.h>

#include "constants.h"

#define TAG "ENCODER"

#define UNIT PCNT_UNIT_0
// in APB cycles at 80 MHz, the most the filter takes: pulses shorter than 12.8 µs are ignored
#define FILTER 1023

bool EncoderCounter::begin()
{
    // Each channel counts the edges of one pin, the other pin's level gives the
    // direction. Together they count all 4 edges of a detent. The signs match
    // the RotaryEncoder library used before, so clockwise stays positive.
    pcnt_config_t a = {};
    a.pulse_gpio_num = PIN_ENC_A;
    a.ctrl_gpio_num = PIN_ENC_B;
    a.channel = PCNT_CHANNEL_0;
    a.unit = UNIT;
    a.pos_mode = PCNT_COUNT_INC;
    a.neg_mode = PCNT_COUNT_DEC;
    a.lctrl_mode = PCNT_MODE_REVERSE;
    a.hctrl_mode = PCNT_MODE_KEEP;
    a.counter_h_lim = INT16_MAX;
    a.counter_l_lim = INT16_MIN;

    pcnt_config_t b = a;
    b.pulse_gpio_num = PIN_ENC_B;
    b.ctrl_gpio_num = PIN_ENC_A;
    b.channel = PCNT_CHANNEL_1;
    b.pos_mode = PCNT_COUNT_DEC;
    b.neg_mode = PCNT_COUNT_INC;

    if (pcnt_unit_config(&a) != ESP_OK || pcnt_unit_config(&b) != ESP_OK)
    {
        ESP_LOGE(TAG, "Cannot set up the pulse counter");
        return false;
    }

    gpio_set_pull_mode(static_cast<gpio_num_t>(PIN_ENC_A), GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(static_cast<gpio_num_t>(PIN_ENC_B), GPIO_PULLUP_ONLY);

    pcnt_set_filter_value(UNIT, FILTER);
    pcnt_filter_enable(UNIT);
    pcnt_counter_pause(UNIT);
    pcnt_counter_clear(UNIT);
    pcnt_counter_resume(UNIT);

    running = true;
    return true;
}

int32_t EncoderCounter::read()
{
    if (!running)
    {
        return 0;
    }

    int16_t count = 0;
    pcnt_get_counter_value(UNIT, &count);
    if (count >= CLEAR_AT || count <= -CLEAR_AT)
    {
        // An edge between reading and clearing is lost, at most once per 4096
        // detents. The DetentCounter's hysteresis absorbs a single count.
        base += count;
        pcnt_counter_clear(UNIT);
        return base;
    }
    return base + count;
}

#endif
