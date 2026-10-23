/* SPDX-FileCopyrightText: 2026 Core Devices LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include <stdint.h>
#include <string.h>

#include <pbl/drivers/accel.h>
#include <pbl/drivers/imu/lis2dw12/lis2dw12.h>
#include <pbl/services/new_timer/new_timer.h>

#include <clar.h>
#include <fake_mutex.h>
#include <fake_regular_timer.h>
#include <fake_rtc.h>

#define REG_WHO_AM_I     0x0FU
#define REG_OUT_X_L      0x28U
#define REG_FIFO_SAMPLES 0x2FU

// Fake LIS2DW12: a register file, and a FIFO the driver reads in one burst
static uint8_t s_regs[256];
static uint8_t s_fifo[LIS2DW12_FIFO_SIZE * LIS2DW12_SAMPLE_SIZE_BYTES];

void pbl_i2c_use(const struct pbl_i2c_dev *dev) {
}

void pbl_i2c_release(const struct pbl_i2c_dev *dev) {
}

int pbl_i2c_write_register_block(const struct pbl_i2c_dev *dev, uint8_t reg, uint32_t size,
                                 const uint8_t *data) {
  memcpy(&s_regs[reg], data, size);
  return 0;
}

int pbl_i2c_read_register_block(const struct pbl_i2c_dev *dev, uint8_t reg, uint32_t size,
                                uint8_t *result) {
  memcpy(result, &s_regs[reg], size);
  return 0;
}

int pbl_i2c_read_register_block_dma(const struct pbl_i2c_dev *dev, uint8_t reg, uint32_t size,
                                    uint8_t *result) {
  cl_assert_equal_i(reg, REG_OUT_X_L);
  memcpy(result, s_fifo, size);
  return 0;
}

int pbl_gpio_configure(const struct pbl_gpio *gpio, uint32_t flags) {
  return 0;
}

int pbl_gpio_get(const struct pbl_gpio *gpio) {
  return 0;
}

int pbl_intc_request(const struct pbl_irq_spec *spec, pbl_intc_handler_t handler, void *arg) {
  return 0;
}

int pbl_intc_enable(const struct pbl_irq_spec *spec) {
  return 0;
}

int pbl_device_init(const struct pbl_device *dev) {
  return 0;
}

bool pbl_device_is_ready(const struct pbl_device *dev) {
  return true;
}

void pbl_busy_wait_us(uint32_t us) {
}

void pbl_sleep_ms(uint32_t ms) {
}

uint64_t pbl_uptime_ms(void) {
  return 0;
}

bool new_timer_add_work_callback(NewTimerWorkCallback cb, void *data) {
  return true;
}

void new_timer_add_work_callback_from_isr(NewTimerWorkCallback cb, void *data) {
}

static const struct pbl_i2c_bus s_bus;
static const struct pbl_intc s_intc;

PBL_LIS2DW12_DEFINE(s_lis2dw12, &s_bus, 0x18, .int1 = {.intc = &s_intc}, .axis_map = {0, 1, 2},
                    .axis_dir = {1, 1, 1});

#define ACCEL (&s_lis2dw12.accel)

static AccelRawBatch s_batch;
static int s_batches;

static void prv_samples(const struct pbl_accel *accel, const AccelRawBatch *batch,
                        void *user_data) {
  s_batch = *batch;
  s_batches++;
}

static const struct pbl_accel_listener s_listener = {.samples = prv_samples};

void test_accel_lis2dw12__initialize(void) {
  memset(s_regs, 0, sizeof(s_regs));
  s_regs[REG_WHO_AM_I] = 0x44U;
  memset(s_fifo, 0, sizeof(s_fifo));
  s_batch = (AccelRawBatch){};
  s_batches = 0;
  *s_lis2dw12.state = (LIS2DW12State){};
  fake_rtc_init(0, 1000);

  cl_assert_equal_i(pbl_accel_dev_init(&ACCEL->dev), 0);
  cl_assert_equal_i(pbl_accel_set_listener(ACCEL, &s_listener), 0);
}

void test_accel_lis2dw12__cleanup(void) {
  pbl_accel_set_listener(ACCEL, nullptr);
  fake_mutex_reset(true /* assert_all_unlocked */);
}

//! The FIFO is read once its newest sample is in, so the batch ends at the read time
void test_accel_lis2dw12__a_batch_ends_at_the_fifo_read_time(void) {
  uint32_t interval_us;
  cl_assert_equal_i(pbl_accel_set_sampling_interval(ACCEL, 40000, &interval_us), 0);
  cl_assert_equal_i(interval_us, 40000);
  cl_assert_equal_i(pbl_accel_set_num_samples(ACCEL, 25), 0);

  // Three samples are queued when sampling stops, which drains the FIFO
  s_regs[REG_FIFO_SAMPLES] = 3;
  fake_rtc_increment_time_ms(500);
  cl_assert_equal_i(pbl_accel_set_num_samples(ACCEL, 0), 0);

  const uint64_t read_time_us = 1000500000ULL;
  cl_assert_equal_i(s_batches, 1);
  cl_assert_equal_i(s_batch.num_samples, 3);
  cl_assert_equal_i(s_batch.sampling_interval_us, 40000);
  cl_assert(s_batch.first_timestamp_us == read_time_us - 2 * 40000ULL);
}
