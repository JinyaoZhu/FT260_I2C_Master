
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ft260_driver.h>

typedef struct
{
    uint16_t m1;
    uint16_t m2;
    uint16_t m3;
    uint16_t m4;
    uint16_t m5;
    uint16_t m6;
} Motors_T;

#define M1_ADDR (0x29)
#define M2_ADDR (0x2A)
#define M3_ADDR (0x2B)
#define M4_ADDR (0x2C)
#define M5_ADDR (0x2D)
#define M6_ADDR (0x2E)

bool MotorWrite(FT260_DEVICE_T *i2c_master, Motors_T *motors)
{
    uint8_t send_buf[2];
    FT260_STATUS ftStatus;
    int32_t writeLength = 0;
    int writeLength_sum = 0;

    send_buf[0] = (motors->m1) & (0xff); // HB
    send_buf[1] = 0;                     // LB(no use)
    ftStatus = i2c_master->I2C_Write(i2c_master, M1_ADDR, FT260_I2C_REPEATED_START, send_buf, 2, &writeLength);
    writeLength_sum += writeLength;

    send_buf[0] = (motors->m2) & (0xff); // HB
    send_buf[1] = 0;                     // LB(no use)
    ftStatus = i2c_master->I2C_Write(i2c_master, M2_ADDR, FT260_I2C_REPEATED_START, send_buf, 2, &writeLength);
    writeLength_sum += writeLength;

    send_buf[0] = (motors->m3) & (0xff); // HB
    send_buf[1] = 0;                     // LB(no use)
    ftStatus = i2c_master->I2C_Write(i2c_master, M3_ADDR, FT260_I2C_REPEATED_START, send_buf, 2, &writeLength);
    writeLength_sum += writeLength;

    send_buf[0] = (motors->m4) & (0xff); // HB
    send_buf[1] = 0;                     // LB(no use)
    ftStatus = i2c_master->I2C_Write(i2c_master, M4_ADDR, FT260_I2C_REPEATED_START, send_buf, 2, &writeLength);
    writeLength_sum += writeLength;

    send_buf[0] = (motors->m5) & (0xff); // HB
    send_buf[1] = 0;                     // LB(no use)
    ftStatus = i2c_master->I2C_Write(i2c_master, M5_ADDR, FT260_I2C_REPEATED_START, send_buf, 2, &writeLength);
    writeLength_sum += writeLength;

    send_buf[0] = (motors->m6) & (0xff); // HB
    send_buf[1] = 0;                     // LB(no use)
    ftStatus = i2c_master->I2C_Write(i2c_master, M6_ADDR, FT260_I2C_START_AND_STOP, send_buf, 2, &writeLength);
    writeLength_sum += writeLength;

    if (writeLength_sum == 12)
        return true;
    else
        return false;
}

void MotorInit(FT260_DEVICE_T *i2c_master)
{
    Motors_T motors;

    motors.m1 = 0;
    motors.m2 = 0;
    motors.m3 = 0;
    motors.m4 = 0;
    motors.m5 = 0;
    motors.m6 = 0;

    // ~ 1.2 second
    for (int i = 0; i < 200; i++)
        MotorWrite(i2c_master, &motors);
}

int main(int argc, char **argv)
{
    // initialize FT260_DEVICE_T structure,
    // set vendor/product/interface IDs and all function pointers.
    FT260_DEVICE_T* ft260_dev = New_FT260( 0x0403, 0x6030, 0);

    if (FT260_OK != ft260_dev->Open(ft260_dev))
        return -1;

    if (FT260_OK != ft260_dev->I2C_Setup(ft260_dev))
        return -1;

    Motors_T motors;
    MotorInit(ft260_dev);

    for (;;)
    {
        motors.m1 = 50;
        motors.m2 = 50;
        motors.m3 = 50;
        motors.m4 = 50;
        motors.m5 = 50;
        motors.m6 = 50;
        // ~6ms per frame
        if (MotorWrite(ft260_dev, &motors))
            printf("Write Motors success.\n");
        else
            printf("Write Motors error.\n");
    }
    return 0;
}
