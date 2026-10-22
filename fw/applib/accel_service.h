/* SPDX-FileCopyrightText: 2024 Google LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <pbl/services/accel_manager.h>
#include <pbl/services/imu/units.h>

#include <kernel/pebble_tasks.h>

//! @addtogroup Foundation
//! @{
//!   @addtogroup EventService
//!   @{
//!     @addtogroup AccelerometerService
//!
//! \brief Using the Pebble accelerometer
//!
//! The AccelerometerService enables the Pebble accelerometer to detect taps,
//! perform measures at a given frequency, and transmit samples in batches to save CPU time
//! and processing.
//!
//! For available code samples, see the
//! <a href="https://github.com/pebble-examples/feature-accel-discs/">feature-accel-discs</a>
//! example app.
//!     @{

//! Enumerated values defining the three accelerometer axes.
typedef enum {
  //! Accelerometer's X axis. The positive direction along the X axis goes
  //! toward the right of the watch.
  ACCEL_AXIS_X = 0,
  //! Accelerometer's Y axis. The positive direction along the Y axis goes
  //! toward the top of the watch.
  ACCEL_AXIS_Y = 1,
  //! Accelerometer's Z axis. The positive direction along the Z axis goes
  //! vertically out of the watchface.
  ACCEL_AXIS_Z = 2,
} AccelAxisType;

// Make sure the AccelAxisType enum is compatible with the unified
// IMUCoordinateAxis enum.
static_assert(ACCEL_AXIS_X == (int)AXIS_X, "AccelAxisType incompatible with IMUCoordinateAxis");
static_assert(ACCEL_AXIS_Y == (int)AXIS_Y, "AccelAxisType incompatible with IMUCoordinateAxis");
static_assert(ACCEL_AXIS_Z == (int)AXIS_Z, "AccelAxisType incompatible with IMUCoordinateAxis");

#define ACCEL_DEFAULT_SAMPLING_RATE ACCEL_SAMPLING_25HZ
#define ACCEL_MINIMUM_SAMPLING_RATE ACCEL_SAMPLING_10HZ

//! Callback type for accelerometer tap events
//! @param axis the axis on which a tap was registered (x, y, or z)
//! @param direction the direction (-1 or +1) of the tap
typedef void (*AccelTapHandler)(AccelAxisType axis, int32_t direction);

//! Callback type for accelerometer data events
//! @param data Pointer to the collected accelerometer samples.
//! @param num_samples the number of samples stored in data.
typedef void (*AccelDataHandler)(AccelData *data, uint32_t num_samples);

//! Callback type for accelerometer raw data events
//! @param data Pointer to the collected accelerometer samples.
//! @param num_samples the number of samples stored in data.
//! @param timestamp the timestamp, in ms, of the first sample.
typedef void (*AccelRawDataHandler)(AccelRawData *data, uint32_t num_samples, uint64_t timestamp);

//! Subscribe to the accelerometer tap event service. Once subscribed, the handler
//! gets called on every tap event emitted by the accelerometer.
//! @param handler A callback to be executed on tap event
void accel_tap_service_subscribe(AccelTapHandler handler);

//! Unsubscribe from the accelerometer tap event service. Once unsubscribed,
//! the previously registered handler will no longer be called.
void accel_tap_service_unsubscribe(void);

//! @internal
//! Subscribe to the accelerometer double tap event service. Once subscribed, the handler
//! gets called on every double tap event emitted by the accelerometer.
//! @param handler A callback to be executed on double tap event
void accel_double_tap_service_subscribe(AccelTapHandler handler);

//! @internal
//! Unsubscribe from the accelerometer double tap event service. Once unsubscribed,
//! the previously registered handler will no longer be called.
void accel_double_tap_service_unsubscribe(void);

//! Subscribe to the accelerometer data event service. Once subscribed, the handler
//! gets called every time there are new accelerometer samples available.
//! @note Cannot use \ref accel_service_peek() when subscribed to accelerometer data events.
//! @param handler A callback to be executed on accelerometer data events
//! @param samples_per_update the number of samples to buffer, at most the accelerometer's limit.
void accel_data_service_subscribe(uint32_t samples_per_update, AccelDataHandler handler);

//! Subscribe to the accelerometer raw data event service. Once subscribed, the handler
//! gets called every time there are new accelerometer samples available.
//! @note Cannot use \ref accel_service_peek() when subscribed to accelerometer data events.
//! @param handler A callback to be executed on accelerometer data events
//! @param samples_per_update the number of samples to buffer, at most the accelerometer's limit.
void accel_raw_data_service_subscribe(uint32_t samples_per_update, AccelRawDataHandler handler);

//! Subscribe to the accelerometer raw data event service, with the samples in the
//! accelerometer's raw counts instead of whole milli-g, which keeps the sensor's full resolution.
//!
//! Counts are signed and already rotated into the watch frame: the same axes and signs as
//! \ref AccelData. X and Y change sign when the display orientation is set to left-handed, so
//! record that setting with the data. Convert with floating point; dividing two integers
//! gives whole g:
//! \code{.c}
//! int32_t counts_per_g = accel_service_get_counts_per_g(); // 2048 at +/-16 g
//! float g = data[i].x / (float)counts_per_g;
//! float mg = data[i].x * 1000.0f / counts_per_g;
//! \endcode
//! - Range: +/-(32768 / counts_per_g) g. An axis whose count is 32767 or -32767 and below is
//!   saturated. On an axis the watch mounts inverted, the sensor's -32768 reads as +32767.
//! - Resolution: 1 count, 1000 / counts_per_g mg, for a 16-bit sensor such as the LSM6DSO. A
//!   12-bit sensor reports counts in steps of 16.
//! - Accuracy: counts_per_g is the sensor's nominal sensitivity, with no per-watch calibration;
//!   the zero-g offset of the LSM6DSO is typically within +/-20 mg. Noise is about 5.5 mg RMS in
//!   its default ultra-low-power mode, about 0.4 mg RMS at 26 Hz in a firmware built with
//!   CONFIG_ACCEL_LSM6DSO_HIGH_PERFORMANCE.
//! - Time: the handler's timestamp is the wall-clock time, in ms, of the first sample. It comes
//!   from the watch's clock, so it jumps when the time is set, e.g. by the phone. Samples are
//!   timed from when the accelerometer's buffer is read, shortly after the newest one was
//!   taken, so they can run a little late. Where the requested rate isn't one the
//!   accelerometer has, samples are dropped from its faster rate to match, so they are not
//!   evenly spaced; use \ref accel_raw_counts_native_service_subscribe() for evenly spaced
//!   samples.
//!
//! This API is specific to this firmware, at SDK revision 110, and is not in upstream PebbleOS.
//! Apps built with it only run on this firmware, so a study using it should record the exact
//! firmware commit.
//! @note Cannot use \ref accel_service_peek() when subscribed to accelerometer data events.
//! @param handler A callback to be executed on accelerometer data events
//! @param samples_per_update the number of samples to buffer, at most the accelerometer's limit.
void accel_raw_counts_service_subscribe(uint32_t samples_per_update, AccelRawDataHandler handler);

//! Subscribe to accelerometer samples in raw counts, at the accelerometer's own sampling rate.
//! Like \ref accel_raw_counts_service_subscribe(), except that no sample is dropped to match
//! the requested rate: the handler gets every sample, evenly spaced at
//! \ref accel_service_get_sampling_interval_us(), so sample i of a batch was taken at
//! timestamp + i * interval. \ref accel_service_set_sampling_rate() sets the slowest rate to
//! sample at; the accelerometer runs at the next rate it supports, e.g. 26 Hz for 25 Hz on the
//! LSM6DSO, and faster while another app or the system asks for a faster rate.
//! @note Cannot use \ref accel_service_peek() when subscribed to accelerometer data events.
//! @param handler A callback to be executed on accelerometer data events
//! @param samples_per_update the number of samples to buffer, at most the accelerometer's limit.
void accel_raw_counts_native_service_subscribe(uint32_t samples_per_update,
                                               AccelRawDataHandler handler);

//! Get the interval between the samples of the current data subscription.
//! For \ref accel_raw_counts_native_service_subscribe() it is the accelerometer's actual
//! interval, which changes if another subscriber asks for a faster rate, so read it in each
//! handler call. For other subscriptions it is the mean interval of the requested rate.
//! @return The interval in microseconds, or 0 when not subscribed.
uint32_t accel_service_get_sampling_interval_us(void);

//! Get the scale of the samples from \ref accel_raw_counts_service_subscribe().
//! @return The number of counts in 1 g, e.g. 2048 at +/-16 g or 8192 at +/-4 g. Fixed for the
//! watch, so it can be read at any time, before subscribing too.
int32_t accel_service_get_counts_per_g(void);

//! Unsubscribe from the accelerometer data event service. Once unsubscribed,
//! the previously registered handler will no longer be called.
void accel_data_service_unsubscribe(void);

//! Change the accelerometer sampling rate.
//! @param rate The sampling rate in Hz (10Hz, 25Hz, 50Hz, and 100Hz possible)
int accel_service_set_sampling_rate(AccelSamplingRate rate);

//! Change the number of samples buffered between each accelerometer data event
//! @param num_samples the number of samples to buffer, at most the accelerometer's limit.
int accel_service_set_samples_per_update(uint32_t num_samples);

//! Peek at the last recorded reading.
//! @param[out] data a pointer to a pre-allocated AccelData item
//! @note Cannot be used when subscribed to accelerometer data events.
//! @return -1 if the accel is not running
//! @return -2 if subscribed to accelerometer events.
int accel_service_peek(AccelData *data);

//!     @} // end addtogroup AccelerometerService
//!   @} // end addtogroup EventService
//! @} // end addtogroup Foundation

//! @internal
typedef void (*AccelRawDataHandler__deprecated)(AccelRawData *data, uint32_t num_samples);

//! @internal
//! This is used to stay in the jump table where the old accel_data_service_subscribe was located.
//! Allows operation on AccelRawData data, which is the same as the previous version of AccelData.
void accel_data_service_subscribe__deprecated(uint32_t samples_per_update,
                                              AccelRawDataHandler__deprecated handler);
