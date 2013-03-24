#ICMI Solar Meter v1.0#
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
measures voltages of up to 20 Volts, connected to analog 0 and analog 1.
The dividers are made up as follows: 

> `V+ -> 150K resistor -> analog pin -> 50 K resistor -> GND`

Using these values, the measured voltage is exactly (at least mathematically)
0.25 times the real voltage. This means that the real voltage can be calculated
by multiplying the measured voltage by 4.

The LCD uses the following connections:

 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * a potentiometer on V0 for controlling display contrast.

##WARNING and disclaimer##
This circuit can only withstand 20 Volts **at maximum**.
Do not apply a voltage greater than that. Check you polarity
when making connections. 
Using a voltage outside of 0-20 Volts and/or mixing up polarity will
almost certainly destroy your Arduino and/or worse. I can and will 
take absolutely no responsiblity for things that may go wrong when you 
build this project.

## License ##
This project is licensed under the [Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Germany License](http://creativecommons.org/licenses/by-nc-sa/3.0/de/).