/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
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
 *
 */

/*
FreeRTOS port based on
https://github.com/hathach/tinyusb/tree/0f187b4d1c4d1cb74e453fbbabf6fdcdbf8a5745/examples/device/cdc_msc_freertos/src
*/

#include "FreeRTOS.h" /* Must come first. */
#include "task.h"     /* RTOS task related API prototypes. */
#include "timers.h"   /* Software timer related API prototypes. */

#include <stdio.h>
#include "pico/stdlib.h"

/* Library includes. */
#if ( mainRUN_ON_CORE == 1 )
#include "pico/multicore.h"
#endif


#include <string.h>

#include "bsp/board.h"
#include "tusb.h"
#include "usb/usbtmc_app.h"

#include "usb/usb_utils.h"

#include "scpi-def.h"
#include "scpi/scpi_base.h"

void vApplicationTickHook( void );

#define USBD_STACK_SIZE    (3*configMINIMAL_STACK_SIZE/2)
#define APP_STACK_SIZE    (configMINIMAL_STACK_SIZE)

#define mainLED_CHECK_FREQUENCY_MS 250
#define mainREG_CHECK_FREQUENCY_MS 100

TimerHandle_t blinky_tm;

static void led_blinky_cb(TimerHandle_t xTimer);
static void usb_device_task(void *param);
static void maintain_registers_task(void *param);

/*------------- MAIN -------------*/
int main(void) {
	TaskHandle_t xHandle;

  board_init();
  scpi_instrument_init();

  blinky_tm = xTimerCreate(NULL, pdMS_TO_TICKS(mainLED_CHECK_FREQUENCY_MS), true, NULL, led_blinky_cb);
  xTaskCreate(usb_device_task, "usbd", USBD_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, &(xHandle));
  vTaskCoreAffinitySet(xHandle, (1 << 0));

  xTaskCreate(maintain_registers_task, "regs", APP_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, &(xHandle));
  vTaskCoreAffinitySet(xHandle, (1 << 0));

  xTimerStart(blinky_tm, 0);
  vTaskStartScheduler();

  return 0;
}


// USB Device Driver task
// This top level thread process all usb events and invoke callbacks
static void usb_device_task(void *param) {
  (void) param;

  // init device stack on configured roothub port
  // This should be called after scheduler/kernel is started.
  // Otherwise it could cause kernel issue since USB IRQ handler does use RTOS queue API.
  tud_init(BOARD_TUD_RHPORT);

  // RTOS forever loop
  while (1) {
    // put this thread to waiting state until there is new events
    tud_task();
    usbtmc_app_task_iter();
  }
}

static void maintain_registers_task(void *param) {
  (void) param;
	TickType_t xNextWakeTime;
	xNextWakeTime = xTaskGetTickCount();
  // RTOS forever loop
	for( ;; ) {
    vTaskDelayUntil( &xNextWakeTime, mainREG_CHECK_FREQUENCY_MS );
    maintainInstrumentRegs();
  }
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
static void led_blinky_cb(TimerHandle_t xTimer) {
  (void) xTimer;
  led_blinking_task();
}
