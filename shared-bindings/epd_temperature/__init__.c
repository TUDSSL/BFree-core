/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016-2017 Scott Shawcroft for Adafruit Industries
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "py/obj.h"
#include "py/runtime.h"
#include "py/reload.h"

#include "supervisor/shared/translate.h"
#include "shared-bindings/epd_temperature/__init__.h"

#include "lib/checkpoint/checkpoint.h"

#include "lib/epd_temperature/epd_temp.h"

STATIC mp_obj_t epd_temp_draw_base(void) {
    epd_draw_base_temp();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(epd_temp_draw_base_obj, epd_temp_draw_base);

STATIC mp_obj_t epd_temp_draw_temp(mp_obj_t temp_in) {
    int t = mp_obj_get_int(temp_in);
    if (t > 99 || t < 0) {
        mp_raise_ValueError(NULL);
    }
    epd_draw_temp(t);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(epd_temp_draw_temp_obj, epd_temp_draw_temp);


STATIC const mp_rom_map_elem_t epd_temperature_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_epd_temperature) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_draw_base),  MP_ROM_PTR(&epd_temp_draw_base_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_draw_temperature),  MP_ROM_PTR(&epd_temp_draw_temp_obj) },

};

STATIC MP_DEFINE_CONST_DICT(epd_temperature_module_globals, epd_temperature_module_globals_table);

const mp_obj_module_t epd_temperature_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&epd_temperature_module_globals,
};
