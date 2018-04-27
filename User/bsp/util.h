

#ifndef __UTIL_H
#define __UTIL_H


#ifndef TRUE
	#define TRUE  1
#endif

#ifndef FALSE
	#define FALSE 0
#endif


#ifndef true
	#define true   1
#endif

#ifndef false
	#define false  0
#endif



unsigned char sum_verify(unsigned char *s_data, int len);
unsigned char xor_verify(unsigned char *s_data, int len);
void soft_reset(void);
int hex_2_ascii(char *s_data, char *o_data, int len);

#endif

