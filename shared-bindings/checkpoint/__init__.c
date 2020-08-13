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
#include "shared-bindings/checkpoint/__init__.h"

#include "lib/checkpoint/checkpoint.h"

STATIC mp_obj_t checkpointruntime_disable(void) {
    checkpoint_disable();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(checkpointruntime_disable_obj, checkpointruntime_disable);

STATIC mp_obj_t checkpointruntime_enable(void) {
    checkpoint_enable();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(checkpointruntime_enable_obj, checkpointruntime_enable);


extern uint32_t checkpoint_performed;
STATIC mp_obj_t checkpointruntime_checkpoint_count(void) {
    return mp_obj_new_int(checkpoint_performed);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(checkpointruntime_checkpoint_count_obj, checkpointruntime_checkpoint_count);

extern uint32_t checkpoint_restore_performed;
STATIC mp_obj_t checkpointruntime_restore_count(void) {
    return mp_obj_new_int(checkpoint_restore_performed);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(checkpointruntime_restore_count_obj, checkpointruntime_restore_count);


//
// Checkpoint strategy configuration
//
extern struct checkpoint_config checkpoint_cfg;

STATIC mp_obj_t checkpointruntime_set_schedule(mp_obj_t cp_schedule) {
    int sched = mp_obj_get_int(cp_schedule);

    switch (sched) {
        case CHECKPOINT_SCHEDULE_TIME:
        case CHECKPOINT_SCHEDULE_TRIGGER:
        case CHECKPOINT_SCHEDULE_HYBRID:
            checkpoint_cfg.checkpoint_schedule = sched;
            break;
        default:
            mp_raise_ValueError(NULL);
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(checkpointruntime_set_schedule_obj, checkpointruntime_set_schedule);

STATIC mp_obj_t checkpointruntime_set_period(mp_obj_t period_ms) {
    int p = mp_obj_get_int(period_ms);
    if (p < 0) {
        mp_raise_ValueError(NULL);
    }
    checkpoint_cfg.cps_period_ms = p;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(checkpointruntime_set_period_obj, checkpointruntime_set_period);

STATIC mp_obj_t checkpointruntime_set_hybrid_period(mp_obj_t hybrid_period_ms) {
    int p = mp_obj_get_int(hybrid_period_ms);
    if (p < 0) {
        mp_raise_ValueError(NULL);
    }
    checkpoint_cfg.cps_hybrid_period_ms = p;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(checkpointruntime_set_hybrid_period_obj, checkpointruntime_set_hybrid_period);


STATIC const mp_rom_map_elem_t checkpoint_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_checkpoint) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_disable),  MP_ROM_PTR(&checkpointruntime_disable_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_enable),  MP_ROM_PTR(&checkpointruntime_enable_obj) },

    { MP_OBJ_NEW_QSTR(MP_QSTR_checkpoint_count),  MP_ROM_PTR(&checkpointruntime_checkpoint_count_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_restore_count),  MP_ROM_PTR(&checkpointruntime_restore_count_obj) },

    { MP_OBJ_NEW_QSTR(MP_QSTR_set_schedule),  MP_ROM_PTR(&checkpointruntime_set_schedule_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_period),  MP_ROM_PTR(&checkpointruntime_set_period_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_hybrid_period),  MP_ROM_PTR(&checkpointruntime_set_hybrid_period_obj) },

};

STATIC MP_DEFINE_CONST_DICT(checkpoint_module_globals, checkpoint_module_globals_table);

const mp_obj_module_t checkpoint_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&checkpoint_module_globals,
};
