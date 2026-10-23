# SPDX-FileCopyrightText: 2026 Core Devices LLC
# SPDX-License-Identifier: Apache-2.0

pbl_clar_test(test_flash_api
  SOURCES
    drivers/flash/flash_api.c
  OVERRIDES dummy_board
)

pbl_clar_test(test_flash_erase
  SOURCES
    drivers/flash/flash_erase.c
  PLATFORMS obelix
  OVERRIDES dummy_board
)

pbl_clar_test(test_accel_lsm6dso
  SOURCES
    drivers/imu/accel.c
    drivers/imu/lsm6dso/lsm6dso.c
    tests/fakes/fake_rtc.c
  DEFINES
    CONFIG_DRIVER_IMU_LOG_LEVEL=0
    CONFIG_ACCEL_LSM6DSO_SCALE_MG=16000
    CONFIG_ACCEL_LSM6DSO_WK_DUR_DEFAULT=1
    CONFIG_ACCEL_LSM6DSO_WK_THS_MIN=1
    CONFIG_ACCEL_LSM6DSO_WK_THS_MAX=40
    CONFIG_ACCEL_LSM6DSO_WK_THS_DEFAULT=16
    PBL_NO_LINKER_SCRIPT
  OVERRIDES dummy_board
)

pbl_clar_test(test_accel_lsm6dso_high_performance
  SOURCE test_accel_lsm6dso.c
  SOURCES
    drivers/imu/accel.c
    drivers/imu/lsm6dso/lsm6dso.c
    tests/fakes/fake_rtc.c
  DEFINES
    CONFIG_DRIVER_IMU_LOG_LEVEL=0
    CONFIG_ACCEL_LSM6DSO_SCALE_MG=16000
    CONFIG_ACCEL_LSM6DSO_WK_DUR_DEFAULT=1
    CONFIG_ACCEL_LSM6DSO_WK_THS_MIN=1
    CONFIG_ACCEL_LSM6DSO_WK_THS_MAX=40
    CONFIG_ACCEL_LSM6DSO_WK_THS_DEFAULT=16
    CONFIG_ACCEL_LSM6DSO_HIGH_PERFORMANCE=1
    PBL_NO_LINKER_SCRIPT
  OVERRIDES dummy_board
)

pbl_clar_test(test_accel_qemu
  SOURCES
    drivers/base/device.c
    drivers/imu/accel.c
    drivers/imu/qemu.c
    tests/fakes/fake_rtc.c
  DEFINES
    CONFIG_DEVICE_LOG_LEVEL=0
    PBL_NO_LINKER_SCRIPT
  OVERRIDES dummy_board
)

pbl_clar_test(test_accel_lis2dw12
  SOURCES
    drivers/imu/accel.c
    drivers/imu/lis2dw12/lis2dw12.c
    tests/fakes/fake_rtc.c
  DEFINES
    CONFIG_DRIVER_IMU_LOG_LEVEL=0
    CONFIG_ACCEL_LIS2DW12_SCALE_MG=4000
    CONFIG_ACCEL_LIS2DW12_WK_DUR_DEFAULT=1
    CONFIG_ACCEL_LIS2DW12_WK_THS_MIN=1
    CONFIG_ACCEL_LIS2DW12_WK_THS_MAX=28
    CONFIG_ACCEL_LIS2DW12_WK_THS_DEFAULT=14
    PBL_NO_LINKER_SCRIPT
  OVERRIDES dummy_board
)
