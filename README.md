# tinyfl_t
This is DIY Attiny13-based driver for flashlights with brigh LEDs, such as CREE XM-L, XP-G and others.
Version "T" - with tact switch.

Benefits:
- ON/OFF with single button click (tact switch button). 
- Stepless brightness adjustment with gamma-correction, from moonlight to turbo. Brightness level is remembered for the time of shutdown.
- Battery voltage control with power-off when battery is discharged.
- Two additional modes - emergency beacon and strobe. Additional led for backlighting
- Reverse polarity protection and ESD protection.

How to made it:

-Firmware.

For firmware modification you will need to install Atmel Studio.
You can set up all working parameters just by editing definitions in .h file.
Or you can flash .hex file with default parameters. 
For firmware upload you will need Arduino or any ISP programmer, such as USBASP.

-Hardware.

You can made pcb at home because it is optimised for DIY technologies, such as photoresist. Pattern for this can be found in .pdf file. In this case bottom layer is not required, you can made single-layer pcb with only 3 wired vias on bottom.
Or You can order those pcbs from China here https://www.pcbway.com/project/shareproject/TinyFL_LED_Driver.html
Also you will need some soldering skills to solder it. 
To firmware uploading you can use SOIC-8 Clamp or you can directly solder thin wires to Attiny13 pads and connect it to programmer.
Don't forget to flash Attiny fuses at first (combinations can be found in .h file).
