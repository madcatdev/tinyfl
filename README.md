# tinyfl_t
This is DIY Attiny13-based PWM driver for flashlights with brigh LEDs, such as CREE XM-L, XP-G and others.
Version "T" - with tact switch.

Benefits:
- ON/OFF with single button click (tact switch button). 
- Working voltage: 2.3V - 5.5V, 4A maximum current (with SI2323 mosfet).
- Extremely low power consumption at moonlight mode: only 5mA with XM-L led. With standart 18650 cell it will beam more than 20 days.
- Stepless brightness adjustment with gamma-correction, from moonlight to turbo. Brightness level is remembered for the time of shutdown.
- Battery voltage control with power-off when battery is discharged (2.7V is the default voltage cut-off).
- Two additional modes - emergency beacon and strobe. Additional led for backlighting.
- Reverse polarity protection and ESD protection.


How to made it:

Firmware

For firmware modification you will need to install Atmel Studio.
You can set up all working parameters just by editing definitions in .h file.
Or you can flash .hex file with default parameters. 
For firmware upload you will need Arduino or any ISP programmer, such as USBASP.
Don't forget to flash Attiny fuses at first (combinations can be found in .h file).

Hardware

You can made pcb at home because it is optimised for DIY technologies, such as photoresist. Pattern for this can be found in .pdf file. In this case bottom layer is not required, you can made single-layer pcb with only 3 wired vias on bottom.
Or You can order those pcbs from China here https://www.pcbway.com/project/shareproject/TinyFL_LED_Driver.html
Also you will need some soldering skills to solder it. 
To firmware uploading you can use SOIC-8 Clamp or you can directly solder thin wires to Attiny pads and connect it to programmer.

Detailed project description and history of creation is here (ru) https://habr.com/ru/post/464673/
