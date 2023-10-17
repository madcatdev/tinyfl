/*
 * TinyFL - Simple AVR (Attiny13) PWM LED driver
 * (c) madcatdev https://github.com/madcatdev
*/

#include "main.h" // < All settings are here

// Яркость нагрузки, 0 - выключен, 255 - максимальная (ШИМ)
u8 rate = RATE_DEFAULT; 
// Направление изменения яркости нагрузки, 1 - прибавить ШИМ, 0 - убавить ШИМ
u8 rate_dir = 0; 
// Массив точек кривой гамма-коррекции, в конце обязательно maxrate
const u8 rate_step_array[12] = {10,25,50,60,75,80,90,120,130,140,150,RATE_MAX}; 
// Состояние заднего светодиода LED2
bool led_state = false;  

// TODO - попробовать bool заменить на u8


int main(void)	{
	init_mcu();
	
	// Main LOOP
	u16 bat_time = 0; 
	u16 x;
	
	while(1)    {
		
		if (BTN_READ){ 
			x = 0;
			while ( (BTN_READ) && (x++ < BTN_ONOFF_DELAY) )  // Посчитаем, сколько времени нажата кнопка. 
				_delay_ms(1);
			
			if  (x < BTN_ONOFF_DELAY)  // Короткое нажатие - обрабатываем двойной клик и(или) выключаем МК
				shortpress();
			
			if (x >= BTN_ONOFF_DELAY)  // Длинное нажатие - управление яркостью и доп.режимы
				longpress();
			
			while (BTN_READ); // Антидребезг (иначе будут самопроизвольные выключения)
		}

		#ifdef BAT_CHECK
			if (bat_time == BAT_CHECK_PERIOD)  { // Проверка заряда батареи через определенный интервал
				bat_time = 0; 
				bat_check();
			}
			bat_time++;
		#endif
		
		_delay_ms(1);
	}
}

// Прерывание INT0 ничего не делает, кроме пробуждения МК
ISR(INT0_vect) {}	
 
// Инициализация МК после подачи питания
void init_mcu(void){
	// Настройка АЦП. Attiny13 datasheet, page 92
	ADMUX = _BV(MUX1) | // [1:0] = 1:0 (ADC2)
	_BV(ADLAR) | // Отсекаем 2 младших бита, получаем 8-битное разрешение
	_BV(REFS0); // Внутренний ИОН 1.1V
	ACSR |= _BV(ACD); // Отключаем компаратор (по умолчанию включен)

	// Настройка таймера. Важно это делать до настройки пина LED_MAIN на выход (иначе видимо не успевает зарядиться C1 после подачи питания)
	// На OC0B висит INT0, поэтому использовать второй канал (например для AUX_LED) не получится.
	TCCR0B = _BV(CS00 ); // 001 - тактовый генератор CLK/1 (скорость шим = CLK/256 = 4687.5 Гц)
	TCCR0A = _BV(WGM01) | _BV(WGM00); // 11 - Режим Fast PWM
	
	//TIMSK0 |= _BV(TOIE0); // Прерывание по переполнению
	
	// PB0 - LED_MAIN (OC0A шим-канал, транзистор), изначально ВЫКЛ
	// PB1 - Кнопка POWER/ADJ, оно же INT0, Flip-Flop. Внутр.подтяжка к питанию
	// PB2 - LED_BACK,  задний светодиод (опционально)
	// PB3 - не исп. Внутр.подтяжка к питанию (чтобы не болталась в воздухе и не мешала сну)
	// PB4 - ADC2 (контроль заряда батареи), Hi-z
	// PB5 - RESET
	PORTB = _BV(1) | _BV(3);
	DDRB = _BV (0) | _BV(2);
	
	#ifdef STARTBLINKS
		LED_BACK_on;
		_delay_ms(50);
		LED_BACK_off;
	#endif
	
	#ifdef STARTSLEEP
		sleep();
	#else
		wakeup();
	#endif
}
 
// Инициализация МК после сна и после init_mcu()
void wakeup(void) {
	//rate_dir = 0;
	if (led_state) 
		LED_BACK_on;
	
	#ifndef RATE_REMEMBER // Если нет памяти яркости
		rate = RATE_DEFAULT; // включается сразу на мин. яркость
	#endif
	OCR0A = rate;
	
	ADCSRA |= _BV(ADPS1) | _BV(ADPS0) | _BV(ADEN) ; // Предделитель частоты, включение АЦП
	ADCSRA |= _BV(ADSC); // Пробная конверсия (необходимо провести первый раз после включения для стабилизации АЦП)
	
	while (BTN_READ); // Антидребезг, иначе при включении может сразу выключиться, актуально с некоторыми кнопками
	PWM_on;
	_delay_ms(100);
}

// Сон в выключенном состоянии
void sleep(void){
	PWM_off;
	LED_MAIN_off;
	LED_BACK_off;
	ADCSRA &= ~_BV(ADEN); // Отключаем АЦП
	
	do {
		// Настройка параметров сна
		GIMSK = _BV(INT0); // ВКЛ прерывание INT0
		MCUCR = _BV(SE) | _BV(SM1); // SE - SLeep_enable = 1, SM - Power-down = 10, ISC = 00 (low level of INT0 generates interrupt)
		// Можно делать иначе (но будет на 10 байт больше) - set_sleep_mode(SLEEP_MODE_PWR_DOWN); sleep_enable();
		
		sei(); // Включаем прерывания, иначе заснем навсегда
		asm ("sleep");  // Собственно сон - sleep_cpu();
		cli(); // Проснулись, чифирнулись, отключили прерывания
		GIMSK = 0x00; // Отключаем INT0
		MCUCR = 0x00; // Убираем бит SE, а заодно все остальное. Можно так же и sleep_disable();, но будет на 4 байта больше
		sei();
		
		// Проверка на длительность импульса включения (защита от наносекундных помех)
		_delay_ms(STARTDELAY);
		
	} while (!BTN_READ);

	wakeup(); // Настроим МК и включим периферию
}

// Доп. режим - маячок 1 Гц. 
void beacon(void) {
	// В этом режиме нет проверки заряда батареи.
	OCR0A = RATE_MAX; 
	while (!BTN_READ) {	// Пока не нажата кнопка, работаем в режиме маяка	
		PWM_on;
		_delay_ms(50);
		PWM_off;
		_delay_ms(950);		
	}			
}

// Доп. режим - стробоскоп 8 Гц. 
void strobe(void) {
	// В этом режиме нет проверки заряда батареи.	
	OCR0A = RATE_MAX; 
	while (!BTN_READ) { // Пока не нажата кнопка, работаем в режиме стробоскопа
		PWM_on;
		_delay_ms(25);
		PWM_off;
		_delay_ms(100);
	}		
}

// Длинное нажатие кнопки - изменение скважности ШИМ
void longpress(void) {
	u8 rate_step = RATE_STEP_DEF;
	u8 x = 0; // Счетчик циклов для перехода в доп.режим
	rate_dir = ~rate_dir; // Инвертируем направление (больше/меньше % ШИМ)

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
			PWM_off;
			_delay_ms(125);
			PWM_on;
			_delay_ms(125);
			x++;
		}	
	
		// Входим в подрограмму доп.режима, если прошла активация (125+125)*10 = 2500мс удержания
		if (x > AUXMODES_DELAY) {
			#ifdef AUXMODES		
			while (BTN_READ); 
			_delay_ms(250); 
			PWM_off;
			LED_BACK_off;
				
			if (rate <= RATE_MIN)
				beacon();
			if (rate >= RATE_MAX)
				strobe();
				
			if (led_state) LED_BACK_on;
			PWM_on;
			x = 0;
			//break;	
			#endif			
		}
		
		OCR0A = rate; // В итоге настроим ШИМ с новым значением rate	
	} // END OF while (BTN_READ)
	
}

// Короткое нажатие кнопки - вкючение/выключение
void shortpress(void) {
	
	#ifdef BTN_DBCLICK
	PWM_off;
	LED_BACK_off;
	_delay_ms(BTN_DBCLICK_DELAY);
	
	u8 y = 0;
	while ((y++ < BTN_DBCLICK_LEN) && (!BTN_READ))  // Ждем клик в течении определенного интервала
		_delay_ms(1);
			
	if 	(BTN_READ) { // Если он был, включаем доп.СД (или выключаем)
		led_state = !led_state;
		if (led_state) 
			LED_BACK_on;
		PWM_on;
		_delay_ms(100);
	} else 
		sleep();
	#else
	sleep(); // Иначе просто засыпаем
	#endif

}

// Измерение напряжения батареи
u8 bat_getvoltage(void) {
	/*
	Значение получается в разрядах ацп (старшие 8 бит)
	Внутренний ИОН 1.1V (1.0-1.2V, page 121). Поэтому значение Vref для расчета делителя стоит принять за 1.0В
	Макс. напряжение, которое будет в схеме - 4.35В, поэтому Kdiv должен быть в районе 4.4-5.0
	Для этого подходит пара 750K:220K, Kdel получается 4.41, Vmax = 4.41 В, Vraw = 17.22 mv/adc
	
	Формула расчета напряжения из ацп: ADC = (Vin/Kdiv) * 256
	Коэфициент Vraw mv/raw = (Vion/256)*Kdiv
	
	Необходимо делать одну конверсию после включения для "прогрева" АЦП
	*/
	//PWM_off;
	//LED_BACK_off;
	_delay_us(500); // Стабилизируем напряжение после отключения мощной нагрузки, Занимает 12 байт
	ADCSRA |= _BV(ADSC); // Начинаем преобразование
	while (ADCSRA & _BV(ADSC)); // ждем окончания преобразования
	//PWM_on;
	//if (led_state) 
	//	LED_BACK_on;
	return ADCH;
}

// Проверка батареи с предупреждением и отключением
void bat_check(void) {
	
	PWM_off;
	u8 adc_raw = bat_getvoltage();
	PWM_on;

	// Предупреждение 
	if (adc_raw < BAT_WARNING) { 
		u8 x = adc_raw; // Копия значения для отработки мигания
		while (x < BAT_WARNING) { // Каждое мигание означает 5 позиций АЦП (1 мигание - слабо разряжено, 1+n - сильнее)
			PWM_off;
			_delay_ms(50);
			PWM_on;
			_delay_ms(50);
			x += BAT_INFO_STEP;
		}
	}
	
	// Отключение
	if (adc_raw < BAT_SHUTDOWN) 
		sleep();	
	
}
 
