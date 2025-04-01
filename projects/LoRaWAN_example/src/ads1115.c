#include "ads1115.h"
#include "tremo_i2c.h"
#include "tremo_delay.h"
#include "stdio.h"

#define ADDR 0x48

#define OS_POS 15
#define MUX_POS 12
#define PGA_POS 9
#define MODE_POS 8
#define DR_POS 5
#define COMP_POS 4
#define CMP_POL_POS 3
#define CMP_LAT_POS 2
#define CMP_QUE_POS 0

void ADS1115_write(uint8_t ptr, uint16_t val)
{   
    while(i2c_get_flag_status(I2C0, I2C_FLAG_UNIT_BUSY));

    i2c_clear_flag_status(I2C0, I2C_FLAG_TRANS_EMPTY);
    i2c_master_send_start(I2C0, ADDR, I2C_WRITE);
    while(!i2c_get_flag_status(I2C0, I2C_FLAG_TRANS_EMPTY));

    i2c_clear_flag_status(I2C0, I2C_FLAG_TRANS_EMPTY);
    i2c_send_data(I2C0, ptr);
    while(!i2c_get_flag_status(I2C0, I2C_FLAG_TRANS_EMPTY));

    i2c_clear_flag_status(I2C0, I2C_FLAG_TRANS_EMPTY);
    i2c_send_data(I2C0, (val>>8) & 0xFF);
    while(!i2c_get_flag_status(I2C0, I2C_FLAG_TRANS_EMPTY));

    i2c_clear_flag_status(I2C0, I2C_FLAG_TRANS_EMPTY);
    i2c_master_send_stop_with_data(I2C0, (val>>0) & 0xFF);
    while(i2c_get_flag_status(I2C0, I2C_FLAG_UNIT_BUSY));
}

void ADS1115_setPtr(uint8_t ptr)
{
    while(i2c_get_flag_status(I2C0, I2C_FLAG_UNIT_BUSY));

    i2c_clear_flag_status(I2C0, I2C_FLAG_TRANS_EMPTY);
    i2c_master_send_start(I2C0, ADDR, I2C_WRITE);
    while(!i2c_get_flag_status(I2C0, I2C_FLAG_TRANS_EMPTY));

    i2c_clear_flag_status(I2C0, I2C_FLAG_TRANS_EMPTY);
    i2c_master_send_stop_with_data(I2C0, ptr);
    while(i2c_get_flag_status(I2C0, I2C_FLAG_UNIT_BUSY));
}

uint16_t ADS1115_read()
{
    while(i2c_get_flag_status(I2C0, I2C_FLAG_UNIT_BUSY));

    uint16_t rx = 0;
    i2c_clear_flag_status(I2C0, I2C_FLAG_TRANS_EMPTY);
    i2c_master_send_start(I2C0, ADDR, I2C_READ);
    while(!i2c_get_flag_status(I2C0, I2C_FLAG_TRANS_EMPTY));

    i2c_clear_flag_status(I2C0, I2C_FLAG_RECV_FULL);
    i2c_set_receive_mode(I2C0, I2C_ACK);
    while(!i2c_get_flag_status(I2C0, I2C_FLAG_RECV_FULL));
    rx |= i2c_receive_data(I2C0) << 8;   

    i2c_clear_flag_status(I2C0, I2C_FLAG_RECV_FULL);
    
    I2C0->CR &= ~(I2C_CR_START_MASK);
    I2C0->CR |= I2C_CR_TRANS_BYTE_MASK | I2C_CR_STOP_MASK | I2C_CR_ACKNAK_MASK;

    while(!i2c_get_flag_status(I2C0, I2C_FLAG_RECV_FULL));
    rx |= i2c_receive_data(I2C0);

    while(i2c_get_flag_status(I2C0, I2C_FLAG_UNIT_BUSY));
    i2c_clear_flag_status(I2C0, I2C_FLAG_TRANS_EMPTY);
    i2c_clear_flag_status(I2C0, I2C_FLAG_RECV_FULL);
    return rx;
}

uint16_t ads1115_readCh(uint8_t ch)
{
    ADS1115_write(0x01, 
        (0b1 << OS_POS) |
        ((0b100+ch) << MUX_POS) |
        (0b000 << PGA_POS) |
        (0b1 << MODE_POS) |
        (0b111 << DR_POS) |
        (0b0 << COMP_POS) |
        (0b0 << CMP_POL_POS) |
        (0b0 << CMP_LAT_POS) |
        (0b11 << CMP_QUE_POS)
    );
    
    while (!(ADS1115_read() & (0b1 << OS_POS)));
    ADS1115_setPtr(0x00);
    return ADS1115_read();
}