# A multi functional digital clock based on 8051
## Basic functions & User interface 
This electronic clock mainly has the following functions:
- [ ] The DS1302 clock chip is used to accurately display time, date and day of the week.
- [ ] The real-time temperature is collected by the DS18B20 temperature sensor chip, and it has a
temperature alarm function.
- [ ] Buzzer sounds every hour, and the buzzer can be turned on in real time by the control of the
remote controller.
- [ ] The time can be stored by EEPROM, and the power-down storage of the time can be controlled
by the remote controller.
- [ ] The time can be adjusted and the clock state can be changed by the infrared remote controller.
- [ ] Communicate with the computer via the UART to transfer real-time time or time stored in the
EEPROM.

The clock interface consists of 5 parts, the date display part, the time display part, the working mode
display part, the day of the week display part, and the temperature display part. Next, we will focus on
our clock working mode. The working mode of the clock can be seen in the upper right corner of the
LCD in real time: MUT, DIS, ERD, EWR, UAR, SET mode. In the  state machine of the
clock, we have two states: time setting state and normal walking state. Among them (MUT, DIS,
ERD, EWR, UAR) are completed in the normal walking state, only the SET mode is in the time setting
state. Refer the doc for further explanation.
![ui](https://github.com/lirui-shanghaitech/A-multi-functions-digital-clock-based-on-8051/blob/master/fig/a.PNG)

## Usage
![ui](https://github.com/lirui-shanghaitech/A-multi-functions-digital-clock-based-on-8051/blob/master/fig/f.PNG)
Note that the Mode button here can convert the state of the clock. The default is the DIS state. When
pressed once, it changes to the time setting state. The mute button has two functions: when the clock is
in the DIS state, press the Mute button to turn on the music playback function. When the clock is in the
SET state, press the Mute button to abandon the current setting and the clock remains at the original
time. The RPT button also has two functions. The first time you press the ERD state, you can transfer
data to the computer if you turn on the UAR at the same time. The second time you press the RPT
button, the EWR state is entered. At this time, the clock continuously writes data to the EEPROM. .
U/SD can turn on UAR or turn on UAR. Press UAR for the first time open the UAR, press it second
time you turn on UAR. The (0,1,2,3,4,5,6,7,8,9) button is the clock setting button. When in the SET
state, the corresponding button is pressed and the corresponding bit becomes the pressed value. The left
or right shift key is responsible for adjusting the position of the cursor, they are only valid in the SET
state.

## Circuit design
This electronic clock consists of seven modules. They
are: LCD display module, DS1302 clock module, DS18B20 temperature measurement module, passive
buzzer sound playback module, EEPROM 24C02C power-down storage module, MCU control module,
infrared wireless communication module.The schematic of the circuit is as follows.
![circuit](https://github.com/lirui-shanghaitech/A-multi-functions-digital-clock-based-on-8051/blob/master/fig/b.PNG)

## Software APIs
Due to the many functions of this program and many low-level driver modules, this project adopts mod-
ular programming. The program consists mainly of five parts and a lot of header files. The five parts
are: DS1302 driver module, DS18B20 driver module, EEPROM driver module (I2C module), LCD1602
driver module, and the final clock function module. We mainly use function calls and finite state ma-
chine programming. The first four modules are mainly responsible for the hardware driver, generating
the timing required by the corresponding chip. The last module is mainly responsible for the interaction
with the user, the clock state conversion, the infrared button Detection, LCD display mode.Below are all the functions used in the program, which are listed in the table below. On the left are the basic driver functions, the right interrupt service function and the user key service function, and the state machine conversion function.
![apis](https://github.com/lirui-shanghaitech/A-multi-functions-digital-clock-based-on-8051/blob/master/fig/c.PNG)

## Simulation & Real circuit

![apis](https://github.com/lirui-shanghaitech/A-multi-functions-digital-clock-based-on-8051/blob/master/fig/d.PNG)
![apis](https://github.com/lirui-shanghaitech/A-multi-functions-digital-clock-based-on-8051/blob/master/fig/e.PNG)
