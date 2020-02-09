# tinyfl_t
This is a DIY Attiny13-based PWM driver for flashlights with bright LEDs, such as CREE XM-L, XP-G and others. Version "T" - with tact switch.

Capabilities and benefits:

-ON/OFF with a single button click (tact switch button).
-Working voltage: 2.3V - 5.5V, 4A maximum current (with SI2323 mosfet).
-Extremely low power consumption at moonlight mode: only 5mA with XM-L led. With standart 18650 cell it will shine more than 20 days.
-Stepless brightness adjustment with gamma-correction, from moonlight to turbo. Brightness level is remembered until shutdown, and is preserved in sleep mode.
-Battery voltage control with auto power-off when battery is discharged (2.7V is the default voltage cut-off).
-Two additional modes - emergency beacon and strobe. Additional LED for backlighting.
-Reverse polarity protection and ESD protection.

How to make it:

Firmware

For firmware modification you will need to install Arduino IDE with Attiny13 support or Atmel Studio. You can set up all working parameters just by editing definitions in .h file. 
Or you can flash .hex file with default parameters. For firmware upload you will need Arduino or any ISP programmer, such as USBASP. 
Don't forget to flash Attiny fuses at first (combinations can be found in .h file).

Hardware


You can make the PCB at home because it its layout is based off DIY technologies, such as photoresist. The Pattern for this can be found in the .pdf file. 
In this case, bottom layer is not required, you can make a single-layer PCB with only 3 copper-wired vias on the bottom. 
Or You can order those PCBs from China here https://www.pcbway.com/project/shareproject/TinyFL_LED_Driver.html 
You will also need some soldering skills in order to solder it. For firmware uploading you can use an SOIC-8 Test Clip or you can directly solder thin wires to Attiny pads and connect them to a programmer.

Detailed project description and history of creation is here (in Russian) https://habr.com/ru/post/464673/
