

#ifndef __LED_H
#define __LED_H


enum {
	LED_SYS = 0,
    LED_ERR,
    LED_NET,
    LED_POWERDOWN,
	LED_MAX,
};


void ledInit(void);
void ledOn(unsigned int _no);
void ledOff(unsigned int _no);
void ledToggle(unsigned int _no);
void ledToggleAll(void);


#endif

