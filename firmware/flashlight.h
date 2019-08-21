/*
TinyFL v0.3_t - Simple AVR (Attiny13a) PWM LED driver
*********************************************************************************
Äðàéâåð ñâåòîäèîäà òèïà CREE íà Attiny13a, ØÈÌ 4.7 KHz, N-ch/P-ch òðàíçèñòîð, êíîïêà áåç ôèêñàöèè
sw version: 19.08.2019 0.3_t, hw version: v3_t
 
FUSES [NON-INVERTED, 0 = programmed, 1 - unprogrammed (attiny13a datasheet, page 104)]
(E:FF, H:FD, L:6A)
BODLEVEL1:0 = 10 (BODLEVEL =  1.8V)
CKSEL1:0 = 10 (internal rc 9.6MHz)
CKDIV8 = 0 (9.6 / 8 = 1.2MHz)  
EESAVE = 0 (EEPROM will NOT be erased during programming)
*/

#ifndef FLASHLIGHT_H_
#define FLASHLIGHT_H_

#define F_CPU 1200000UL  // Attiny13 1.2MHz / PWM 4.6 KHz

#include <util/delay.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#define nop __asm__ __volatile__ ("nop");
#define u8 uint8_t
#define u16 uint16_t

// Íàñòðîéêè äðàéâåðà
#define RATE_MAX 255 // Ìàêñ. ÿðêîñòü (max = 255)
#define RATE_MIN 0 // Ìèí. ÿðêîñòü (min = 0)
#define RATE_REMEMBER // Çàïîìèíàòü òåêóùóþ ÿðêîñòü íà âðåìÿ ñíà
#define RATE_NONLINEAR  // Íåëèíåéíûé ðåæèì èçìåíåíèÿ ÿðêîñòè (ãàììà-êîððåêöèÿ). RATE_STEP_DEF áóäåò ïðîèãíîðèðîâàí è rate_step ðàñ÷èòàí äèíàìè÷åñêè
#define RATE_STEP_DEF 10 // Øàã èçìåíåíèÿ ÿðêîñòè
#define RATE_STEP_LEN 30 // Âðåìÿ 1 øàãà â ìñ

#define BAT_CHECK // Ïðîâåðêà çàðÿäà áàòàðåè
#define BAT_PERIOD 60000 // Èíòåðâàë ïðîâåðêè çàðÿäà áàòàðåè (ïðèìåðíîå), ìñ. Ðàç â ìèíóòó îïòèìàëüíî. uint16
//#define BAT_PERIOD 1000 // Äëÿ êàëèáðîâêè
#define BAT_WARNING 135 // Íàïðÿæåíèå ïðåäóïðåæäåíèÿ î ðàçðÿäå, 3.0 V 
#define BAT_SHUTDOWN 115 // Íàïðÿæåíèå ïåðåõîäà â ñïÿùèé ðåæèì, 2.7 V
#define BAT_INFO_STEP 3 // Êîë-âî åäèíèö ÀÖÏ íà îäíî ìèãàíèå ïðè èíäèêàöèè çàðÿäà

#define BTN_READ (!(PINB & _BV(1))) // Чтение состояния кнопки на PB1
#define BTN_DBCLICK // Äâîéíîé êëèê, âêëþ÷åíèå äîï.ñâåòîäèîäà
#define BTN_DBCLICK_DELAY 100 // Çàäåðæêà êëèêà, ìñ
#define BTN_DBCLICK_LEN 100 // Äëèòåëüíîñòü êëèêà, ìñ
#define BTN_ONOFF_DELAY 250 // Âðåìÿ íàæàòèÿ êíîïêè äî âõîäà â ðåæèì óïðàâëåíèÿ ÿðêîñòüþ, ìñ. uint16

#define AUXMODES // Äîï ðåæèìû, ìàÿê è ñòðîá
#define AUXMODES_DELAY 10 // Çàäåðæêà äî âõîäà â äîï.ðåæèìû, x*0.125c
#define STARTSLEEP // Çàñûïàåì ïîñëå ïîäà÷è ïèòàíèÿ íà äðàéâåð
#define STARTBLINKS // Aux led ìèãàåò ðàçà ïðè ïîäà÷å ïèòàíèÿ
 
// Íàñòðîéêà ïèíîâ è ðåæèìîâ ðàáîòû òðàíçèñòîðà
//#define NCH // N-channel FET (n-êàíàëüíûé ïîëåâèê)
#define PCH // P-channel FET (p-êàíàëüíûé ïîëåâèê)
#define LOAD_CONNECT DDRB |= 1; // LOAD Pin PB0
#define LOAD_DISCONNECT DDRB &= ~1;
#ifdef NCH 
	#define LOAD_ON PORTB |= 1; // PB0
	#define LOAD_OFF PORTB &= ~1;
#endif
#ifdef PCH 
	#define LOAD_ON PORTB &= ~1;
	#define LOAD_OFF PORTB |= 1;
	#endif
#define LED_SETPIN DDRB |= _BV(2); // LED2 Pin
#define LED_ON PORTB |= _BV(2); // ÂÊË LED2
#define LED_OFF  PORTB &= ~_BV(2); // ÂÛÊË LED2
#define LED_INV PORTB ^= _BV(2); // Èíâåðòèðîâàòü LED2

#define PWM_ON TCCR0A |= _BV(COM0A1); // Âêëþ÷àåì ØÈÌ íà PB0
#define PWM_OFF	TCCR0A &= ~_BV(COM0A1); // Îòêëþ÷àåì ØÈÌ íà PB0


// Ïðîòîòèïû ôóíêöèé
int main(void);
void wakeup(void);
void setup(void);
void sleep(void);
u8 bat_getvoltage(void);
void beacon(void);
void strobe(void);
void pwm_setup(void);
void longpress(void);
void shortpress(void);
void bat_check(void);


#endif /* FLASHLIGHT_H_ */
