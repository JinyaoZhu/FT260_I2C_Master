/**
 * @file ft260_driver.h
 * @author J.Zhu
 * @date 2018-12-09
 * 
 * FT260 I2C-Master driver for Linux
 * I2C clock speed: 100kHz
 * 
 * TODO: Add I2C read function.
 */

#ifndef __FT260_DRIVER_H__
#define __FT260_DRIVER_H__

#include <linux/types.h>
#include <linux/input.h>
#include <linux/hidraw.h>

#include <libudev.h>
/*
* For the systems that don't have the new version of hidraw.h in userspace.
*/
#ifndef HIDIOCSFEATURE
#warning please have your distro update the userspace kernel headers
#define HIDIOCSFEATURE(len) _IOC(_IOC_WRITE | _IOC_READ, 'H', 0x06, len)
#define HIDIOCGFEATURE(len) _IOC(_IOC_WRITE | _IOC_READ, 'H', 0x07, len)
#endif

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <errno.h>

#include <stdint.h>
#include <stdbool.h>

// clk speed range 60k~800kbps
#define FT260_I2C_CLK_SPEED (100) 

// show debug infos
#define FT260_INFO_VERBOSE (false)

// colorize printf()
#define PRED "\x1B[31m" // red
#define PGRN "\x1B[32m" // green
#define PYEL "\x1B[33m" // yellow
#define PBLU "\x1B[34m" // blue
#define PMAG "\x1B[35m" // magenta
#define PCYN "\x1B[36m" // cyan
#define PWHT "\x1B[37m" // white
#define PRST "\x1B[0m"  // reset/normal

// Device status
typedef enum
{
    FT260_OK = 0,
    FT260_FAIL
} FT260_STATUS;

// I2C Flags
typedef enum
{
    FT260_I2C_NONE = 0,
    FT260_I2C_START = 0x02,
    FT260_I2C_REPEATED_START = 0x03,
    FT260_I2C_STOP = 0x04,
    FT260_I2C_START_AND_STOP = 0x06
} FT260_I2C_FLAG;


typedef struct FT260_DEVICE_T
{
    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t interface_id;
    uint16_t i2c_clock_speed;
    int fd; //file descriptor
    FT260_STATUS (*Open)(FT260_DEVICE_T *self);
    FT260_STATUS (*I2C_Setup)(FT260_DEVICE_T *self);
    FT260_STATUS (*I2C_Write)(FT260_DEVICE_T *self,
                                uint8_t addr,
                                FT260_I2C_FLAG i2c_flag,
                                uint8_t *data_buf_ptr,
                                uint16_t data_buf_len,
                                int32_t *written_length);
} FT260_DEVICE_T;

/**
 * @name New_FT260
 * @brief constructor for FT260_DEVICE_T
 * @return pointer to the FT260_DEVICE_T structure
 */
FT260_DEVICE_T* New_FT260(uint16_t vendor_id,
                          uint16_t product_id,
                          uint16_t interface_id);

/**
 * @name Open
 * @brief open FT260 and show device infos
 */
FT260_STATUS Open(FT260_DEVICE_T *self);

/**
 * @name I2C_Setup
 * @brief initialize I2C master controller
 */
FT260_STATUS I2C_Setup(FT260_DEVICE_T *self);

/**
 * @name I2C_Write
 * @brief write data with I2C bus
 * @param addr: I2C slave address
 *        i2c_flag: write method(FT260_I2C_NONE, FT260_I2C_START,
 *         FT260_I2C_REPEATED_START, FT260_I2C_STOP, FT260_I2C_START_AND_STOP)
 *        data_buf_ptr: pointer to the data buffer
 *        data_buf_len: data length
 * @return written_length: how many bytes was written to I2C bus
 *         status: FT260_OK/FT260_FAIL
 */
FT260_STATUS I2C_Write(FT260_DEVICE_T *self,
                       uint8_t addr,
                       FT260_I2C_FLAG i2c_flag,
                       uint8_t *data_buf_ptr,
                       uint16_t data_buf_len,
                       int32_t *written_length);

#endif /* __FT260_DRIVER_H__ */