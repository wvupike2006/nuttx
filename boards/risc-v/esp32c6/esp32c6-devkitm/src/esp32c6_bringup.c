/****************************************************************************
 * boards/risc-v/esp32c6/esp32c6-devkitm/src/esp32c6_bringup.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <debug.h>
#include <fcntl.h>
#include <syslog.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <nuttx/fs/fs.h>

#include "esp_board_ledc.h"
#include "esp_board_spiflash.h"

#ifdef CONFIG_WATCHDOG
#  include "espressif/esp_wdt.h"
#endif

#ifdef CONFIG_TIMER
#  include "espressif/esp_timer.h"
#endif

#ifdef CONFIG_ONESHOT
#  include "espressif/esp_oneshot.h"
#endif

#ifdef CONFIG_RTC_DRIVER
#  include "espressif/esp_rtc.h"
#endif

#ifdef CONFIG_DEV_GPIO
#  include "espressif/esp_gpio.h"
#endif

#ifdef CONFIG_INPUT_BUTTONS
#  include <nuttx/input/buttons.h>
#endif

#ifdef CONFIG_ESP_RMT
#  include "esp_board_rmt.h"
#endif

#ifdef CONFIG_ESPRESSIF_SPI
#  include "espressif/esp_spi.h"
#  include "esp_board_spidev.h"
#endif

#ifdef CONFIG_ESPRESSIF_WIFI_BT_COEXIST
#  include "esp_coexist_internal.h"
#endif

#ifdef CONFIG_ESPRESSIF_WIFI
#  include "esp_board_wlan.h"
#endif

#include "esp32c6-devkitm.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: esp_bringup
 *
 * Description:
 *   Perform architecture-specific initialization.
 *
 *   CONFIG_BOARD_LATE_INITIALIZE=y :
 *     Called from board_late_initialize().
 *
 *   CONFIG_BOARD_LATE_INITIALIZE=y && CONFIG_BOARDCTL=y :
 *     Called from the NSH library via board_app_initialize().
 *
 * Input Parameters:
 *   None.
 *
 * Returned Value:
 *   Zero (OK) is returned on success; A negated errno value is returned on
 *   any failure.
 *
 ****************************************************************************/

int esp_bringup(void)
{
  int ret = OK;

#ifdef CONFIG_FS_PROCFS
  /* Mount the procfs file system */

  ret = nx_mount(NULL, "/proc", "procfs", 0, NULL);
  if (ret < 0)
    {
      _err("Failed to mount procfs at /proc: %d\n", ret);
    }
#endif

#ifdef CONFIG_FS_TMPFS
  /* Mount the tmpfs file system */

  ret = nx_mount(NULL, CONFIG_LIBC_TMPDIR, "tmpfs", 0, NULL);
  if (ret < 0)
    {
      _err("Failed to mount tmpfs at %s: %d\n", CONFIG_LIBC_TMPDIR, ret);
    }
#endif

#ifdef CONFIG_ESPRESSIF_MWDT0
  ret = esp_wdt_initialize("/dev/watchdog0", ESP_WDT_MWDT0);
  if (ret < 0)
    {
      _err("Failed to initialize WDT: %d\n", ret);
    }
#endif

#ifdef CONFIG_ESPRESSIF_MWDT1
  ret = esp_wdt_initialize("/dev/watchdog1", ESP_WDT_MWDT1);
  if (ret < 0)
    {
      _err("Failed to initialize WDT: %d\n", ret);
    }
#endif

#ifdef CONFIG_ESPRESSIF_RWDT
  ret = esp_wdt_initialize("/dev/watchdog2", ESP_WDT_RWDT);
  if (ret < 0)
    {
      _err("Failed to initialize WDT: %d\n", ret);
    }
#endif

#ifdef CONFIG_TIMER
  ret = esp_timer_initialize(0);
  if (ret < 0)
    {
      _err("Failed to initialize Timer 0: %d\n", ret);
    }

#ifndef CONFIG_ONESHOT
  ret = esp_timer_initialize(1);
  if (ret < 0)
    {
      _err("Failed to initialize Timer 1: %d\n", ret);
    }
#endif
#endif

#ifdef CONFIG_ONESHOT
  ret = esp_oneshot_initialize();
  if (ret < 0)
    {
      _err("Failed to initialize Oneshot Timer: %d\n", ret);
    }
#endif

#ifdef CONFIG_ESP_RMT
  ret = board_rmt_txinitialize(RMT_TXCHANNEL, RMT_OUTPUT_PIN);
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: board_rmt_txinitialize() failed: %d\n", ret);
    }

  ret = board_rmt_rxinitialize(RMT_RXCHANNEL, RMT_INPUT_PIN);
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: board_rmt_txinitialize() failed: %d\n", ret);
    }
#endif

#ifdef CONFIG_RTC_DRIVER
  /* Initialize the RTC driver */

  ret = esp_rtc_driverinit();
  if (ret < 0)
    {
      _err("Failed to initialize the RTC driver: %d\n", ret);
    }
#endif

#if defined(CONFIG_ESPRESSIF_SPI) && defined(CONFIG_SPI_DRIVER)
  ret = board_spidev_initialize(ESPRESSIF_SPI2);
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: Failed to init spidev 2: %d\n", ret);
    }
#endif

#ifdef CONFIG_ESPRESSIF_SPIFLASH
  ret = board_spiflash_init();
  if (ret)
    {
      syslog(LOG_ERR, "ERROR: Failed to initialize SPI Flash\n");
    }
#endif

#ifdef CONFIG_ESPRESSIF_TWAI0

  /* Initialize TWAI and register the TWAI driver. */

  ret = board_twai_setup(0);
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: board_twai_setup failed: %d\n", ret);
    }
#endif

#ifdef CONFIG_ESPRESSIF_TWAI1

  /* Initialize TWAI and register the TWAI driver. */

  ret = board_twai_setup(1);
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: board_twai_setup failed: %d\n", ret);
    }
#endif

#ifdef CONFIG_ESPRESSIF_WIFI_BT_COEXIST
  esp_coex_adapter_register(&g_coex_adapter_funcs);
  coex_pre_init();
#endif

#ifdef CONFIG_ESPRESSIF_WIFI
  ret = board_wlan_init();
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: Failed to initialize wireless subsystem=%d\n",
             ret);
    }
#endif

#ifdef CONFIG_DEV_GPIO
  ret = esp_gpio_init();
  if (ret < 0)
    {
      ierr("Failed to initialize GPIO Driver: %d\n", ret);
    }
#endif

#if defined(CONFIG_INPUT_BUTTONS) && defined(CONFIG_INPUT_BUTTONS_LOWER)
  /* Register the BUTTON driver */

  ret = btn_lower_initialize("/dev/buttons");
  if (ret < 0)
    {
      ierr("ERROR: btn_lower_initialize() failed: %d\n", ret);
    }
#endif

#ifdef CONFIG_ESPRESSIF_LEDC
  ret = board_ledc_setup();
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: board_ledc_setup() failed: %d\n", ret);
    }
#endif /* CONFIG_ESPRESSIF_LEDC */

  /* If we got here then perhaps not all initialization was successful, but
   * at least enough succeeded to bring-up NSH with perhaps reduced
   * capabilities.
   */

  return ret;
}
