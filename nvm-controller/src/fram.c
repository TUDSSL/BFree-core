#include "fram.h"
#include "spi.h"


void writeByte(uint32_t address, uint8_t data)
{
    uint8_t *ptr = (uint8_t *)address;
    MPUCTL0 = MPUPW | MPUENA_0;
    *ptr = data;
    MPUCTL0 = MPUPW | MPUENA_1;
}

void writeShort(uint32_t address, uint16_t data)
{
    data = readSpiShort(data);
    uint16_t *ptr = (uint16_t *)address;
    MPUCTL0 = MPUPW | MPUENA_0;
    *ptr = data;
    MPUCTL0 = MPUPW | MPUENA_1;
}

void writeLong(uint32_t address, uint32_t data)
{
    data = readSpiLong(data);
    uint32_t *ptr = (uint32_t *)address;
    MPUCTL0 = MPUPW | MPUENA_0;
    *ptr = data;
    MPUCTL0 = MPUPW | MPUENA_1;
}

void readByte(uint8_t *ptr, uint16_t len)
{
    int j = 0;
    for(j = 0; j < len; j++, ptr--)
    {
        writeSpiByte(*ptr);
    }
}
void readShort(uint16_t *ptr, uint16_t len)
{
    int j = 0;
    for(j = 0; j < len; j++, ptr--)
        writeSpiShort(*ptr);
}
void readLong(uint32_t *ptr, uint16_t len)
{
    int j = 0;
    for(j = 0; j < len; j++, ptr--)
        writeSpiLong(*ptr);
}
