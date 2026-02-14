/*
This file is part of the Gamebuino-AKA library,
Copyright (c) Gamebuino 2026

This is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License (LGPL)
as published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License (LGPL) for more details.

You should have received a copy of the GNU Lesser General Public
License (LGPL) along with the library.
If not, see <http://www.gnu.org/licenses/>.

Authors:
 - Jean-Marie Papillon
*/

#include "gb_core.h"
#include "freertos/FreeRTOS.h"
#include "gb_ll_lcd.h"
#include "gb_ll_i2c.h"
#include "gb_ll_expander.h"
#include "gb_ll_i2c.h"
#include "gb_ll_audio.h"
#include "gb_ll_adc.h"
#include "gb_ll_sdcard.h"
#include "gb_ll_system.h"
#include "gb_ll_adc.h"

// ============================================================
//  ACTIVE / DESACTIVE L'AUDIO I2S + AMPLI
// ============================================================
//  Pour Tetris : laisser commenté
//  Pour jeux avec musique : décommenter
//
// #define USE_I2S_AUDIO
// ============================================================


gb_core::gb_core() {}
gb_core::~gb_core() {}


// -----------------------------------------------------------------------------
//  Timing
// -----------------------------------------------------------------------------
void gb_core::delay_ms(uint32_t u32_ms)
{
    gb_delay_ms(u32_ms);
}

uint32_t gb_core::get_millis()
{
    return gb_get_millis();
}

int64_t gb_core::get_micros()
{
    return gb_get_micros();
}


// -----------------------------------------------------------------------------
//  Initialisation du hardware AKA
// -----------------------------------------------------------------------------
void gb_core::init()
{
    gb_ll_system_init();
    gb_ll_adc_init();
    gb_ll_i2c_init();
    gb_ll_expander_init();

    // IMPORTANT : LCD AVANT SDMMC
    gb_ll_lcd_init();
    gb_ll_sd_init();

#ifdef USE_I2S_AUDIO
    gb_ll_audio_init();
#endif
}


// -----------------------------------------------------------------------------
//  Boutons
// -----------------------------------------------------------------------------
void gb_buttons::update()
{
    u16_buttons_last = u16_buttons;
    u16_buttons = gb_ll_expander_read() & EXPANDER_KEY;

    if (u16_buttons & EXPANDER_KEY_RUN)
        gb_ll_expander_power_off();
}

uint16_t gb_buttons::state()
{
    return u16_buttons;
}

uint16_t gb_buttons::pressed()
{
    return u16_buttons & (~u16_buttons_last);
}

bool gb_buttons::pressed(gb_key key)
{
    return (pressed() & key) ? true : false;
}

uint16_t gb_buttons::released()
{
    return (~u16_buttons) & u16_buttons_last;
}

bool gb_buttons::released(gb_key key)
{
    return (released() & key) ? true : false;
}


// -----------------------------------------------------------------------------
//  Joystick
// -----------------------------------------------------------------------------
int16_t gb_joystick::get_x() { return i16_joy_x; }
int16_t gb_joystick::get_y() { return i16_joy_y; }
float gb_joystick::get_posx() { return f32_joy_pos_x; }
float gb_joystick::get_posy() { return f32_joy_pos_y; }

void gb_joystick::set_posx_range(float fmin, float fmax)
{
    f32_joy_pos_x_min = fmin;
    f32_joy_pos_x_max = fmax;
}

void gb_joystick::set_posy_range(float fmin, float fmax)
{
    f32_joy_pos_y_min = fmin;
    f32_joy_pos_y_max = fmax;
}

void gb_joystick::set_posxy_speed(float fspeed)
{
    f32_joy_speed_xy = fspeed;
}

void gb_joystick::set_posxy(float posx, float posy)
{
    f32_joy_pos_x = BOUND(posx, f32_joy_pos_x_min, f32_joy_pos_x_max);
    f32_joy_pos_y = BOUND(posy, f32_joy_pos_y_min, f32_joy_pos_y_max);
}

uint16_t gb_joystick::state() { return u16_buttons; }
uint16_t gb_joystick::pressed() { return u16_buttons & (~u16_buttons_last); }
bool gb_joystick::pressed(gb_buttons::gb_key key) { return (pressed() & key) ? true : false; }
uint16_t gb_joystick::released() { return (~u16_buttons) & u16_buttons_last; }
bool gb_joystick::released(gb_buttons::gb_key key) { return (released() & key) ? true : false; }

void gb_joystick::calibrate_center()
{
    i32_joy_center_x = 0;
    i32_joy_center_y = 0;

    for (int i = 0; i < 1000; i++)
    {
        i32_joy_center_x += gb_ll_adc_read_joyx();
        i32_joy_center_y += gb_ll_adc_read_joyy();
        gb_delay_us(100);
    }

    i32_joy_center_x /= 1000;
    i32_joy_center_y /= 1000;
}

void gb_joystick::update()
{
    uint32_t u32_now = gb_get_millis();

    i16_joy_x = 2000 * (gb_ll_adc_read_joyx() - i32_joy_center_x) / JOYX_MAX;
    i16_joy_y = 2000 * (gb_ll_adc_read_joyy() - i32_joy_center_y) / JOYX_MAX;

    if (u32_last_update)
    {
        float f32_delta_t_sec = (u32_now - u32_last_update) / 1000.0;

        if ((i16_joy_x > 50) || (i16_joy_x < -50))
            f32_joy_pos_x += f32_delta_t_sec * f32_joy_speed_xy * i16_joy_x / 1000.0;

        if (f32_joy_pos_x_min && f32_joy_pos_x_max)
            f32_joy_pos_x = BOUND(f32_joy_pos_x, f32_joy_pos_x_min, f32_joy_pos_x_max);

        if ((i16_joy_y > 50) || (i16_joy_y < -50))
            f32_joy_pos_y -= f32_delta_t_sec * f32_joy_speed_xy * i16_joy_y / 1000.0;

        if (f32_joy_pos_y_min && f32_joy_pos_y_max)
            f32_joy_pos_y = BOUND(f32_joy_pos_y, f32_joy_pos_y_min, f32_joy_pos_y_max);
    }

    u16_buttons_last = u16_buttons;
    u16_buttons = 0;

    if (i16_joy_y > 500) u16_buttons |= EXPANDER_KEY_UP;
    if (i16_joy_y < -500) u16_buttons |= EXPANDER_KEY_DOWN;
    if (i16_joy_x > 500) u16_buttons |= EXPANDER_KEY_RIGHT;
    if (i16_joy_x < -500) u16_buttons |= EXPANDER_KEY_LEFT;

    u32_last_update = u32_now;
}


// -----------------------------------------------------------------------------
//  Pool
// -----------------------------------------------------------------------------
void gb_core::pool()
{
    buttons.update();
    joystick.update();
}


// -----------------------------------------------------------------------------
//  Mémoire
// -----------------------------------------------------------------------------
size_t gb_core::free_psram()
{
    return heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);
}

size_t gb_core::free_sram()
{
    return heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
}


// -----------------------------------------------------------------------------
//  Power down
// -----------------------------------------------------------------------------
void gb_core::power_down()
{
    gb_ll_expander_power_off();
}
