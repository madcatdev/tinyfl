/*
 * flashlight.cpp
 *
 * Created: 12.09.2018 14:53:32
*/

#include "flashlight.h"

volatile u8 rate = RATE_DEFAULT; // Яркость нагрузки, 0 - выключен, 255 - максимальная (ШИМ)
volatile u8 rate_dir = 0; // Направление изменения яркости нагрузки, 1 - прибавить ШИМ, 0 - убавить ШИМ
u8 rate_step_array[12] = {10,25,50,60,75,80,90,120,130,140,150,RATE_MAX}; // Массив точек кривой гамма-коррекции, в конце обязательно maxrate
bool led_state = false; // Состояние заднего светодиода LED2

int main(void)	{
	setup();
	wakeup();
	
	u16 bat_time = 0; 
	
	while(1)    {
		
		if (BTN_READ){ 
			u16 x = 0;
			while ( (BTN_READ) && (x++ < BTN_ONOFF_DELAY) )  // Посчитаем, сколько времени нажата кнопка. 
				_delay_ms(1);
			
			if  (x < BTN_ONOFF_DELAY)  // Короткое нажатие - обрабатываем двойной клик и(или) выключаем МК
				shortpress();
			
			if (x >= BTN_ONOFF_DELAY)  // Длинное нажатие - управление яркостью и доп.режимы
				longpress();
			
			while (BTN_READ); // Антидребезг (иначе будут самопроизвольные выключения)
		}

		#ifdef BAT_CHECK
		if (bat_time >= BAT_PERIOD)  { // Проверка заряда батареи через определенный интервал
			bat_time = 0; 
			bat_check();
		}
		bat_time++;
		#endif
		
		_delay_ms(1);
	}
}

ISR(INT0_vect) {}	// Прерывание ничего не делает, кроме пробуждения МК
 
void wakeup(void) {
	// Инициализация МК после сна или при старте
	rate_dir = 0;
	if (led_state) LED_ON;
	LOAD_CONNECT;
	PWM_ON;
	#ifndef RATE_REMEMBER // Если нет памяти яркости
	rate = RATE_DEFAULT; // включается сразу на мин. яркость
	#endif
	OCR0A = rate;
	
	ADCSRA |= _BV(ADPS1) | _BV(ADPS0) | _BV(ADEN); // Предделитель частоты, включение АЦП
	bat_getvoltage(); // Пробный запуск АЦП (первый блин комом, где-то в даташите про это было)
	
	while (BTN_READ); // Антидребезг, иначе при включении может сразу выключиться, актуально с некоторыми кнопками
	_delay_ms(100);
}

void setup(void) {
	// Настройка МК после подачи питания на драйвер

	// PB0 - Led CREE (OC0A шим-канал, транзистор), изначально ВЫКЛ
	//LOAD_CONNECT; 
	LOAD_OFF; 
	
	// PB1 - Кнопка POWER/ADJ, оно же INT0, Flip-Flop. Внутр.подтяжка к питанию
	PORTB |= _BV(1);
	
	// PB2 - LED2,  задний светодиод (опционально)
	LED_SETPIN;  
	LED_OFF;
	
	//PB3 - не исп. Внутр.подтяжка к питанию
	PORTB |= _BV(3);
	
	//PB4 // ADC2 (контроль заряда батареи)
	// Hi-z

	// PB5 - не исп, RESET
	
	// Настройка АЦП. Attiny13 datasheet, page 92
	ADMUX |= _BV(MUX1); // [1:0] = 1:0 (ADC2)
	ADMUX |= _BV(ADLAR); // Отсекаем 2 младших бита, получаем 8-битное разрешение
	ADMUX |= _BV(REFS0); // Внутренний ИОН 1.1V

	ACSR |= _BV(ACD); // Отключаем компаратор (по умолчанию включен)
	
	
	pwm_setup(); // Настройка таймера
	PWM_OFF;
	
	#ifdef STARTBLINKS
	LED_ON;
	_delay_ms(50);
	LED_OFF;
	#endif
	
	#ifdef STARTSLEEP
	sleep(); 
	#endif	
	
}

void sleep(void){
	// Сон в выключенном состоянии
	PWM_OFF;
	LOAD_OFF;
	LOAD_DISCONNECT; // Hi-z
	LED_OFF;
	
	ADCSRA &= ~_BV(ADEN); // Отключаем АЦП
	
	

	while (true) {
		// Настройка параметров сна
		GIMSK |= _BV(INT0); // ВКЛ прерывание INT0
		MCUCR &= ~(_BV(ISC01) | _BV(ISC00)); // INT0 по низкому уровню
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
		
		// Основной цикл сна, последовательность не менять!
		sleep_enable();
		cli();
		sei(); // Включаем прерывания, иначе не проснемся
		sleep_cpu(); // Собственно сон
		cli(); // Проснулись, чифирнулись, отключили прерывания
		GIMSK = 0x00; // Отключаем INT0
		sleep_disable();
		sei();                         
		// Пробуждаемся
		
		// Проверка на длительность импульса включения (защита от наносекундных помех)
		_delay_ms(STARTDELAY);
		if (BTN_READ) break;
	}

	wakeup(); // Настроим МК и включим периферию
}

void beacon(void) {
	// Доп. режим - маячок 1 Гц. В этом режиме нет проверки заряда батареи.
	PWM_ON; // Minrate уже установлен
	while (!BTN_READ) {	// Пока не нажата кнопка, работаем в режиме маяка	
		//LOAD_ON; // С частотой 1Гц включаемся на 1/20c (50мс)
		OCR0A = RATE_MAX; 
		PWM_ON;
		_delay_ms(50);
		//LOAD_OFF;
		PWM_OFF;
		_delay_ms(950);		
	}			
}

void strobe(void) {
	// Доп. режим - стробоскоп 8 Гц. В этом режиме нет проверки заряда батареи.		
	while (!BTN_READ) { // Пока не нажата кнопка, работаем в режиме стробоскопа
		LOAD_ON;
		_delay_ms(25);
		LOAD_OFF;
		_delay_ms(100);
	}		
}

void pwm_setup(void) {
	// Настройка таймера и выдача % ШИМ согласно rate
	// http://osboy.ru/blog/microcontrollers/attiny13-pwm.html Таймер в attiny13 всего один - это T0, не разгуляешься
	// При этом на OC0B висит INT0, поэтому использовать второй канал (например для AUX_LED) не получится.
	
	TCCR0B |= _BV(CS00 ); // 001 - тактовый генератор CLK/1 (скорость шим = CLK/256 = 4687.5 Гц)
	TCCR0A |= _BV(WGM01) | _BV(WGM00); // 11 - Режим Fast PWM
	//TIMSK0 |= _BV(TOIE0); // Прерывание по переполнению
	
	#ifdef NCH 
	TCCR0A |= _BV(COM0A1); // 10 - установка 0 на выводе OC0A при совпадении с A, установка 1 на выводе OC0A при обнулении счётчика (неинверсный режим)
	#endif
	
	#ifdef PCH 
	TCCR0A |= _BV(COM0A1) | _BV(COM0A0); // 11 - установка 1 на выводе OC0A при совпадении с A, установка 0 на выводе OC0A при обнулении счётчика (инверсный режим)
	#endif
}

void longpress(void) {
	// Длинное нажатие кнопки - управление заполнением ШИМ
	
	u8 rate_step = RATE_STEP_DEF;
	u8 x = 0; // Счетчик циклов для перехода в доп.режим
	rate_dir =~rate_dir; // Инвертируем направление (больше/меньше % ШИМ)

	while (BTN_READ) { // Пока кнопка нажата, изменяем значение rate 
		_delay_ms(RATE_STEP_LEN);
		// Определим rate_step в зависимости от текущего rate
		// Это сделано для компенсации нелинейной зависимости яркости СД от значения ШИМ (гамма-коррекция)
		// Подробнее про это можно глянуть тут - youtube.com/watch?v=9zrobCFeeGw (в tinyfl реализация проще)
		#ifdef RATE_NONLINEAR 
		// Переберем массив значений кривой step_array, пока не будет совпадения с rate
		// Массив обязательно должен заканчиваться на maxrate, иначе будет ошибка
		u8 y = 0;
		while (rate > rate_step_array[y])  
			y++;
		rate_step = y + 1; // Установим rate_step как номер совпавшего элемента массива кривой + 1
		#endif
	
		if (rate_dir) {	// Изменим rate в большую сторону
			if (rate < RATE_MAX) {
				if (255 - rate > rate_step)
					rate += rate_step;
				else
					rate = RATE_MAX;
			}
		}
		else {	// Или в меньшую сторону
			if (rate > RATE_MIN) {
				if (rate > rate_step)
					rate -= rate_step;
				else
					rate = RATE_MIN;
			}
		}
	
		// Мигаем СД-CREE, когда достигнуто крайнее значение, отслеживаем переход в доп.режим
		if ( (rate <= RATE_MIN) || (rate >= RATE_MAX) ) {
			PWM_OFF;
			_delay_ms(125);
			PWM_ON;
			_delay_ms(125);
			x++;
		}	
	
		// Входим в подрограмму доп.режима, если прошла активация (125+125)*10 = 2500мс удержания
		if (x > AUXMODES_DELAY) {
			#ifdef AUXMODES		
			while (BTN_READ); 
			_delay_ms(250); 
			PWM_OFF;
			LED_OFF;
				
			if (rate <= RATE_MIN)
				beacon();
			if (rate >= RATE_MAX)
				strobe();
				
			if (led_state) LED_ON;
			PWM_ON;
			x = 0;
			//break;	
			#endif			
		}
		OCR0A = rate; // В итоге настроим ШИМ с новым значением rate	
	} // END OF while (BTN_READ)
	
}

void shortpress(void) {
	// Короткое нажатие кнопки
	#ifdef BTN_DBCLICK
	PWM_OFF;
	LOAD_OFF;
	LOAD_DISCONNECT;
	LED_OFF;
	_delay_ms(BTN_DBCLICK_DELAY);
	
	u8 y = 0;
	while ((y++ < BTN_DBCLICK_LEN) && (!BTN_READ))  // Ждем клик в течении определенного интервала
		_delay_ms(1);
			
	if 	(BTN_READ) { // Если он был, включаем доп.СД (или выключаем)
		led_state = !led_state;
		if (led_state) LED_ON;
		LOAD_CONNECT;
		PWM_ON;
		_delay_ms(100);
	} else sleep();
	#else
	sleep(); // Иначе просто засыпаем
	#endif

}

u8 bat_getvoltage(void) {
	/*
	Получение напряжения батареи в единицах АЦП
	Внутренний ИОН 1.1V (1.0-1.2V, page 121). Поэтому значение Vref для расчета делителя стоит принять за 1.0В
	Макс. напряжение, которое будет в схеме - 4.35В, поэтому Kdiv должен быть в районе 4.4-5.0
	Для этого подходит пара 750K:220K, Kdel получается 4.41, Vmax = 4.41 В, Vraw = 17.22 mv/adc
	
	Формула расчета напряжения из ацп: ADC = (Vin/Kdiv) * 256
	Коэфициент Vraw mv/raw = (Vion/256)*Kdiv
	*/
	PWM_OFF;
	LED_OFF;
	_delay_us(500); // Стабилизируем напряжение после отключения мощной нагрузки, Занимает 12 байт
	ADCSRA |= _BV(ADSC); // Начинаем преобразование
	while (ADCSRA & _BV(ADSC)); // ждем окончания преобразования
	PWM_ON;
	if (led_state) LED_ON;
	return ADCH;
}

void bat_check(void) {
	// Проверка заряда батареи
	u8 adc_raw = bat_getvoltage();

	if (adc_raw < BAT_WARNING) { // Предупреждение (мигаем обеими диодами). 
		u8 x = adc_raw; // Копия значения для отработки мигания
		while ( (x < BAT_WARNING) ) { // Каждое мигание означает 5 позиций АЦП (1 мигание - слабо разряжено, 1+n - сильнее)
			PWM_OFF;
			_delay_ms(75);
			PWM_ON;
			_delay_ms(75);
			x = x + BAT_INFO_STEP;
		}
	}
	
	if (adc_raw < BAT_SHUTDOWN) // Отключение
		sleep();	
}
