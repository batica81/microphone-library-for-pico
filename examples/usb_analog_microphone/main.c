/*
 * Copyright (c) 2021 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 * 
 * This examples creates a USB Microphone device using the TinyUSB
 * library and captures data from an ANALOG microphone using a sample
 * rate of 16 kHz, to be sent the to PC.
 * 
 * The USB microphone code is based on the TinyUSB audio_test example.
 * 
 * https://github.com/hathach/tinyusb/tree/master/examples/device/audio_test
 */


#include "usb_microphone.h"

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/analog_microphone.h"
#include "tusb.h"

// configuration
const struct analog_microphone_config config = {
    // GPIO to use for input, must be ADC compatible (GPIO 26 - 28)
    .gpio = 26,

    // bias voltage of microphone in volts
    //.bias_voltage = 1.25,
    .bias_voltage = 0,

    // sample rate in Hz
    //.sample_rate = 16000,
    .sample_rate = 192000,

    // number of samples to buffer
    .sample_buffer_size = SAMPLE_BUFFER_SIZE,
};

// variables
uint16_t sample_buffer[SAMPLE_BUFFER_SIZE];
volatile int samples_read = 0;

void on_analog_samples_ready()
{
    // callback from library when all the samples in the library
    // internal sample buffer are ready for reading
    samples_read = analog_microphone_read(sample_buffer, SAMPLE_BUFFER_SIZE);
}

// callback functions

void on_usb_microphone_tx_ready()
{
  // Callback from TinyUSB library when all data is ready
  // to be transmitted.
  //
  // Write local buffer to the USB microphone
  usb_microphone_write(sample_buffer, sizeof(sample_buffer));
}


int main(void)
{
  // initialize the analog microphone
  if (analog_microphone_init(&config) < 0) {
      while (1) { tight_loop_contents(); }
  }

  // set callback that is called when all the samples in the library
  // internal sample buffer are ready for reading
  analog_microphone_set_samples_ready_handler(on_analog_samples_ready);

  // start capturing data from the analog microphone
  if (analog_microphone_start() < 0) {
      while (1) { tight_loop_contents();  }
  }

  // initialize the USB microphone interface
  usb_microphone_init();
  usb_microphone_set_tx_ready_handler(on_usb_microphone_tx_ready);

  while (1) {
    // run the USB microphone task continuously
    usb_microphone_task();
  }

  return 0;
}
