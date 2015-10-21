#ICMI Solar Meter v1.1#
![](http://i.creativecommons.org/l/by-nc-sa/3.0/de/88x31.png)
----------

This project is a DIY meter originally intended for my solar energy
installation. It is supposed to add monitoring functionality to my
off-the-shelve solar charger, that lacks this functionality (or at least,
it has no display to report any values).

This first version monitors the voltage of the solar cell panel and
the battery. In a later stage, things like power intake and output will
be measurable and I seek to implement logging features as well.

The circuit consists of two voltage dividers allowing the Arduino to
measures voltages of up to 25 Volts, connected to analog 0 and analog 1.
The dividers are made up as follows: 

> `V+ -> 68K resistor -> analog pin -> 5K potentiometer (set to 3K) -> GND`

I opted for a potentiometer to allow fine tuning without changing the software. 
Also, the reference voltage for the ADC is no longer 5 Volts but 1.1 Volts. This
is a little more precise because the main problem are rounding errors and overflows
when calculating the real voltage. To avoid floating point math for as long as possible,
voltage is calculated in millivolts and a maximum of 1100 millivolts leave a 
little more room than 5000.

The LCD uses the following connections:

 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * a potentiometer on V0 for controlling display contrast.

## A note regarding calibration
If your readings are off, you can adjust the value by slightly turning the potentiometer up or down.
Fine tune your potentiometer using a multimeter and a realistic battery voltage.
There usually is deviation between the real voltage and the one calculated. This deviation
grows the farther the current voltage is apart from the voltage the circuit was fine-tuned for.

## A note regarding the common GND
In some solar installations, the charge controller doesn't work properly when the solar panel and the battery share a common ground connection. Please refer to the manual of your charge controller before connecting the ICMI solar meter. When in doubt, connect only the battery line and connect the panel pin to ground. You will then of course receive only readings for the battery and the panel voltage will display as 0 Volts.

##WARNING and disclaimer##
This circuit can only withstand 25 Volts *at maximum*.

- Do not apply a voltage greater than that! 
- Check you polarity when making connections!
- Make sure you set the potentiometer to  about 3K resistance before connecting anything!

Using a voltage outside of 0-25 Volts and/or mixing up polarity will
almost certainly destroy your Arduino and/or worse. I can and will 
take absolutely no responsiblity for things that may go wrong when you 
build this project.

## License ##
This project is licensed under the [Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Germany License](http://creativecommons.org/licenses/by-nc-sa/3.0/de/).