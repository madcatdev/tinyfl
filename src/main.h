/*
 * TinyFL - Simple AVR (Attiny13) PWM LED driver
 * MCU: Attiny13(a)
 * Last version: 24.07.2022 (0.4)
 * (c) madcatdev https://github.com/madcatdev
 *---------
 * Драйвер светодиода типа CREE на Attiny13a, ШИМ 4.7 KHz, N-ch/P-ch транзистор, кнопка без фиксации
*/


#ifndef MAIN_H_
#define MAIN_H_

// Frequency definition for gcc. Do not forget to set proper fuses.
// Определение частоты для компилятора. Не забудь выставить частоту фьюзами. 
#define F_CPU 1200000UL  // Attiny13 1.2MHz / PWM 4.6 KHz / CKDIV8 = 0
//#define F_CPU 9600000UL  // Attiny13 9.6MHz / PWM 36.8 KHz / CKDIV8 = 1

#include <util/delay.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <stdbool.h> // bool in C99

#define nop __asm__ __volatile__ ("nop");
typedef uint8_t u8;
typedef uint16_t u16;

//-------------------------------------

// Driver settings 
// Настройки драйвера

// MAX brightness (max = 255) 
// Макс. яркость
#define RATE_MAX 250 

// MIN brightness (min = 0)	
// Мин. яркость
#define RATE_MIN 0

// Restore brightness level when wakin up 
// Запоминать текущую яркость на время сна
#define RATE_REMEMBER 

// Non-linear brightness mode (gamma-correction). RATE_STEP_DEF will be ignores and rate_step will be calculated on the go
// Нелинейный режим изменения яркости (гамма-коррекция). RATE_STEP_DEF будет проигнорирован и rate_step расчитан динамически
#define RATE_NONLINEAR 
 
// Brightness adjustment step
// Шаг изменения яркости
#define RATE_STEP_DEF 10 

// Length of brightness adjustment step (ms)
// Длительность одного шага изменения яркости (мс)
#define RATE_STEP_LEN 30 

// Default brightness (when battery connected, or when each wakeup, if RATE_REMEMBER is not defined)
// Яркость по умолчанию (при первом включении, если не объявлен RATE_REMEMBER - при каждом включении)
#define RATE_DEFAULT RATE_MIN 

// Battery voltage check
// Проверка заряда батареи
#define BAT_CHECK 

// Battery voltage check interval (ms). Once per minute is optimal for me. Maximum is 65s (65535)
// Интервал проверки заряда батареи (примерное), мс. Раз в минуту оптимально. 65с максимально (65535)
#define BAT_CHECK_PERIOD 60000 
//#define BAT_CHECK_PERIOD 1000 // For easy calibration | Для калибровки

// Battery warning voltage, 135 is about 3.0V
// Напряжение предупреждения о разряде, 135 ~ 3.0 V
#define BAT_WARNING 195 

// Shutdown voltage, 115 is about 2.7V
// Напряжение перехода в спящий режим, 115 ~ 2.7 V
#define BAT_SHUTDOWN 180 

// ADC raw units per one warning blink (when battery is low)
// Кол-во единиц АЦП на одно мигание при индикации заряда
#define BAT_INFO_STEP 3 

// Double click turns on LED_BACK 
// Двойной клик, включение доп.светодиода
#define BTN_DBCLICK 

// Double click delay, ms
// Задержка клика, мс
#define BTN_DBCLICK_DELAY 100 

// Double click length, ms
// Длительность клика, мс
#define BTN_DBCLICK_LEN 100 

// Time between button pressed and sleep mode, ms.
// Время нажатия кнопки до входа в режим управления яркостью, мс. uint16
#define BTN_ONOFF_DELAY 250 

// Additional modes, beacon and strobe
// Доп режимы, маяк и строб
#define AUXMODES 

// Delay beetween entering these modes, x*0.125s
// Задержка до входа в доп.режимы, x*0.125c
#define AUXMODES_DELAY 10 

// Sleep after battery connected for the first time
// Засыпаем после подачи питания на драйвер
#define STARTSLEEP 

// Indication when battery connected for the first time (wery useful sometimes)
// Индикация подачи питания при помощи LED_BACK (удобно при прошивке)
#define STARTBLINKS 

// Turn-on delay, ms. For EMP immunity value between 5-10 is recomended 
// Задержка при включении, мс. Для помехозащиты рекомендуется 5-10
#define STARTDELAY 5 


// Peripheral setup
// Настройка пинов, типа транзистора и таймеров
//#define NCH // N-channel FET | N-канальный полевик
#define PCH // P-channel FET | P-канальный полевик)
#ifdef NCH
	#define LED_MAIN_on PORTB |= 1; // PB0
	#define LED_MAIN_off PORTB &= ~1;
	// 10 - установка 0 на выводе OC0A при совпадении с A, установка 1 на выводе OC0A при обнулении счётчика (неинверсный режим)
	#define PWM_on TCCR0A |= _BV(COM0A1);
	#define PWM_off	TCCR0A &= ~_BV(COM0A1); // Отключаем ШИМ на PB0
#endif
#ifdef PCH
	#define LED_MAIN_on PORTB &= ~1;
	#define LED_MAIN_off PORTB |= 1;
	// 11 - установка 1 на выводе OC0A при совпадении с A, установка 0 на выводе OC0A при обнулении счётчика (инверсный режим)
	#define PWM_on TCCR0A |= _BV(COM0A1) | _BV(COM0A0);
	#define PWM_off	TCCR0A &= ~(_BV(COM0A1) | _BV(COM0A0));
#endif
#define LED_BACK_on PORTB |= _BV(2); // ВКЛ LED2
#define LED_BACK_off  PORTB &= ~_BV(2); // ВЫКЛ LED2
#define LED_BACK_inv PORTB ^= _BV(2); // Инвертировать LED2

// Button reading on PB1
// Чтение состояния кнопки на PB1
#define BTN_READ (!(PINB & _BV(1)))

int main(void);
static void init_mcu(void);
static void wakeup(void);
void sleep(void);
static void beacon(void);
static void strobe(void);
void longpress(void);
void shortpress(void);
static u8 bat_getvoltage(void);
static void bat_check(void);

#endif /* MAIN_H_ */
