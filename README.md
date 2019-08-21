# tinyfl_t
This is DIY Attiny13-based driver for flashlights with brigh LEDs, such as CREE XM-L, XP-G and others.
Version "T" - with tact switch.

Benefits:
- ON/OFF with single button click (tact switch button). This was a main reason why i made this project.
- Stepless brightness adjustment with gamma-correction, from moonlight to turbo. Brightness level is remembered for the time of shutdown.
- Battery voltage control with power-off when battery is discharged.
- Two additional modes - emergency beacon and strobe. Additional led for backlighting
- Reverse polarity protection and ESD protection.

For firmware modification you will need to install Atmel Studio.
You can set up all working parameters just by editing definitions in .h file.
Or you can flash .hex file with default parameters. 
For firmware upload you will need Arduino or any ISP programmer, such as USBASP.

