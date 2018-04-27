
#include "stm32f10x.h"


unsigned char sum_verify(unsigned char *s_data, int len)
{
    unsigned char sum_data = 0;

    while(len--)
    {
        sum_data += *s_data++;
    }

    return sum_data;
}


unsigned char xor_verify(unsigned char *s_data, int len)
{
    unsigned char sum_data = 0;

    while(len--)
    {
        sum_data ^= *s_data++;
    }

    return sum_data;
}

void soft_reset(void)
{
    __set_FAULTMASK(1);
    NVIC_SystemReset();
}


int hex_2_ascii(char *s_data, char *o_data, int len)
{
    const char ascii_table[17] = {"0123456789ABCDEF"};

    char *p_temp = o_data;
    int i, pos;

    pos = 0;
    for(i = len-1; i >= 0 ; i--)
    {
        p_temp[pos++] = ascii_table[*(s_data + i) >> 4];
        p_temp[pos++] = ascii_table[*(s_data + i) & 0x0F];
    }

    p_temp[pos] = '\0';
    
    return pos;
}
