/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Scott Shawcroft for Adafruit Industries
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
#include "background.h"

#include "audio_dma.h"
#include "tick.h"
#include "supervisor/filesystem.h"
#include "supervisor/usb.h"

#include "py/runtime.h"
#include "shared-module/network/__init__.h"
#include "supervisor/shared/stack.h"

#ifdef CIRCUITPY_DISPLAYIO
#include "shared-module/displayio/__init__.h"
#endif

#include "lib/checkpoint/checkpoint.h"

volatile uint64_t last_finished_tick = 0;

bool stack_ok_so_far = true;

static bool running_background_tasks = false;

void background_tasks_reset(void) {
    running_background_tasks = false;
}

void run_background_tasks(void) {
    // Don't call ourselves recursively.
    if (running_background_tasks) {
        return;
    }
    assert_heap_ok();
    running_background_tasks = true;

    #if (defined(SAMD21) && defined(PIN_PA02)) || defined(SAMD51)
    audio_dma_background();
    #endif
    #if CIRCUITPY_DISPLAYIO
    displayio_refresh_displays();
    #endif

    #if CIRCUITPY_NETWORK
    network_module_background();
    #endif
    filesystem_background();
    usb_background();
    running_background_tasks = false;
    assert_heap_ok();

    #if CHECKPOINT_SCHEDULE_UPDATE
    checkpoint_schedule_update();
    #endif

    last_finished_tick = ticks_ms;
}

bool background_tasks_ok(void) {
    return ticks_ms - last_finished_tick < 1000;
}
