/*
TinyFL v0.3_t - Simple AVR (Attiny13a) PWM LED driver
*********************************************************************************
Драйвер светодиода типа CREE на Attiny13a, ШИМ 4.7 KHz, N-ch/P-ch транзистор, кнопка без фиксации
sw version: 19.08.2019 0.4, hw version: v3_t
*/

#ifndef FLASHLIGHT_H_
#define FLASHLIGHT_H_

#define F_CPU 1200000UL  // Attiny13 1.2MHz / PWM 4.6 KHz

#include <util/delay.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#define nop __asm__ __volatile__ ("nop");
typedef uint8_t u8;
typedef uint16_t u16;

// Настройки драйвера
#define RATE_MAX 255 // Макс. яркость (max = 255)
#define RATE_MIN 0 // Мин. яркость (min = 0)
#define RATE_REMEMBER // Запоминать текущую яркость на время сна
#define RATE_NONLINEAR  // Нелинейный режим изменения яркости (гамма-коррекция). RATE_STEP_DEF будет проигнорирован и rate_step расчитан динамически
#define RATE_STEP_DEF 10 // Шаг изменения яркости
#define RATE_STEP_LEN 30 // Время 1 шага в мс
#define RATE_DEFAULT RATE_MIN // Яркость по умолчанию (при первом включении, если не объявлен RATE_REMEMBER - при каждом включении)

#define BAT_CHECK // Проверка заряда батареи
#define BAT_PERIOD 60000 // Интервал проверки заряда батареи (примерное), мс. Раз в минуту оптимально. uint16
//#define BAT_PERIOD 1000 // Для калибровки
#define BAT_WARNING 110 // Напряжение предупреждения о разряде, 135 - 3.0 V 
#define BAT_SHUTDOWN 90 // Напряжение перехода в спящий режим, 115 - 2.7 V
#define BAT_INFO_STEP 3 // Кол-во единиц АЦП на одно мигание при индикации заряда

#define BTN_READ (!(PINB & _BV(1))) // Чтение состояния кнопки на PB1
#define BTN_DBCLICK // Двойной клик, включение доп.светодиода
#define BTN_DBCLICK_DELAY 100 // Задержка клика, мс
#define BTN_DBCLICK_LEN 100 // Длительность клика, мс
#define BTN_ONOFF_DELAY 250 // Время нажатия кнопки до входа в режим управления яркостью, мс. uint16

#define AUXMODES // Доп режимы, маяк и строб
#define AUXMODES_DELAY 10 // Задержка до входа в доп.режимы, x*0.125c
#define STARTSLEEP // Засыпаем после подачи питания на драйвер
#define STARTBLINKS // Aux led мигает раза при подаче питания
#define STARTDELAY 5 // Задержка при включении, мс. Для помехозащиты рекомендуется 5-10

// Настройка пинов и режимов работы транзистора
//#define NCH // N-channel FET (n-канальный полевик)
#define PCH // P-channel FET (p-канальный полевик)
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
#define LED_ON PORTB |= _BV(2); // ВКЛ LED2
#define LED_OFF  PORTB &= ~_BV(2); // ВЫКЛ LED2
#define LED_INV PORTB ^= _BV(2); // Инвертировать LED2

#define PWM_ON TCCR0A |= _BV(COM0A1); // Включаем ШИМ на PB0
#define PWM_OFF	TCCR0A &= ~_BV(COM0A1); // Отключаем ШИМ на PB0


// Прототипы функций
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
