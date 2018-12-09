
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

bool MotorWrite(Motors_T *motors)
{
    uint8_t send_buf[2];
    FT260_STATUS ftStatus;
    int32_t writeLength = 0;
    int writeLength_sum = 0;

    send_buf[0] = (motors->m1) & (0xff); // HB
    send_buf[1] = 0;                     // LB(no use)
    ftStatus = FT260_I2C_Write(M1_ADDR, FT260_I2C_REPEATED_START, send_buf, 2, &writeLength);
    writeLength_sum += writeLength;

    send_buf[0] = (motors->m2) & (0xff); // HB
    send_buf[1] = 0;                     // LB(no use)
    ftStatus = FT260_I2C_Write(M2_ADDR, FT260_I2C_REPEATED_START, send_buf, 2, &writeLength);
    writeLength_sum += writeLength;

    send_buf[0] = (motors->m3) & (0xff); // HB
    send_buf[1] = 0;                     // LB(no use)
    ftStatus = FT260_I2C_Write(M3_ADDR, FT260_I2C_REPEATED_START, send_buf, 2, &writeLength);
    writeLength_sum += writeLength;

    send_buf[0] = (motors->m4) & (0xff); // HB
    send_buf[1] = 0;                     // LB(no use)
    ftStatus = FT260_I2C_Write(M4_ADDR, FT260_I2C_REPEATED_START, send_buf, 2, &writeLength);
    writeLength_sum += writeLength;

    send_buf[0] = (motors->m5) & (0xff); // HB
    send_buf[1] = 0;                     // LB(no use)
    ftStatus = FT260_I2C_Write(M5_ADDR, FT260_I2C_REPEATED_START, send_buf, 2, &writeLength);
    writeLength_sum += writeLength;

    send_buf[0] = (motors->m6) & (0xff); // HB
    send_buf[1] = 0;                     // LB(no use)
    ftStatus = FT260_I2C_Write(M6_ADDR, FT260_I2C_START_AND_STOP, send_buf, 2, &writeLength);
    writeLength_sum += writeLength;

    if (writeLength_sum == 12)
        return true;
    else
        return false;
}

void MotorInit()
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
        MotorWrite(&motors);
}

int main(int argc, char **argv)
{
    if (FT260_OK != FT260_Open())
        return -1;

    if (FT260_OK != FT260_I2C_Setup())
        return -1;

    MotorInit();

    Motors_T motors;

    for (;;)
    {
        motors.m1 = 50;
        motors.m2 = 50;
        motors.m3 = 50;
        motors.m4 = 50;
        motors.m5 = 50;
        motors.m6 = 50;
        if (MotorWrite(&motors))
            printf("Write Motors success.\n");
        else
            printf("Write Motors error.\n");
    }
    return 0;
}
