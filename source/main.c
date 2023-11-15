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

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
#include "task.h"
#include "timers.h"

/* Library includes. */
#if ( mainRUN_ON_CORE == 1 )
#include "pico/multicore.h"
#endif


#include <stdlib.h>
#include <stdio.h>
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

// static timer & task
#if configSUPPORT_STATIC_ALLOCATION
StaticTimer_t blinky_tmdef;

StackType_t  usb_device_stack[USBD_STACK_SIZE];
StaticTask_t usb_device_taskdef;

StackType_t  app_stack[APP_STACK_SIZE];
StaticTask_t app_taskdef;

#endif



TimerHandle_t blinky_tm;

static void led_blinky_cb(TimerHandle_t xTimer);
static void usb_device_task(void *param);
static void maintain_registers_task(void *param);

// TODO this is currently defined in the PSL
/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};


/*------------- MAIN -------------*/
int main(void)
{
  board_init();

  scpi_instrument_init();



#if configSUPPORT_STATIC_ALLOCATION
  // soft timer for blinky
  blinky_tm = xTimerCreateStatic(NULL, pdMS_TO_TICKS(BLINK_NOT_MOUNTED), true, NULL, led_blinky_cb, &blinky_tmdef);

  // Create a task for tinyusb device stack
  xTaskCreateStatic(usb_device_task, "usbd", USBD_STACK_SIZE, NULL, configMAX_PRIORITIES-1, usb_device_stack, &usb_device_taskdef);

  // create task to maintain SCPI registers
  xTaskCreateStatic(maintain_registers_task, "regs", APP_STACK_SIZE, NULL, configMAX_PRIORITIES-2, app_stack, &app_taskdef);


#else
  blinky_tm = xTimerCreate(NULL, pdMS_TO_TICKS(BLINK_NOT_MOUNTED), true, NULL, led_blinky_cb);
  xTaskCreate(usb_device_task, "usbd", USBD_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);
  xTaskCreate(maintain_registers_task, "regs", APP_STACK_SIZE, NULL, configMAX_PRIORITIES - 2, NULL);
#endif

  xTimerStart(blinky_tm, 0);

  vTaskStartScheduler();


/*
  while (1)
  {
    tud_task(); // tinyusb device task
    led_blinking_task();
    maintainInstrumentRegs();
    usbtmc_app_task_iter();
  }
*/

  return 0;
}

void vApplicationTickHook( void )
{
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

  // RTOS forever loop
  while (1) {
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
