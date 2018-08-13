Arduino Car Tachometer
======================

I recently bought a Nissan Micra K11 with the aim to slap a turbo on and use it
for some budget track day smiles. The platform was chosen for its availablilty 
here in the UK, the low price of parts and its handling and performace 
potential (yes really!). Small fast cars are great fun to throw around on the 
track.

Anyway, being a crappy old micra it didn't come with a tachometer. I could have
sourced some clocks from a better equiped model, or purchased an aftermarket 
tacho, but being a tight-arse and already having all the parts I need I decided 
to make one.


Will it only work on your crap car?
-----------------------------------

No. It should work on most crap cars. You just need a 5v tacho signal.


How it works
------------

In my case, a square wave tacho signal from the ECU via a voltage divider is fed
into pin 3 of an Ardiuno Nano. Two TPIC6C595 shift registers wired in series are
connected on the SPI pins (13 11 & 10 for latch). The shift registers sink 14 
leds - 8 green, 3 orange and 3 red. Pin 2 controls blue shift light led.

As the engine revolutions climb, the leds are illuminated in order with each led
representing an increment of 500rpm.

When the 6500rpm rev limiter is hit all the leds flash on and off.

When 4000rpm is reached the blue shift light is illuminated.

Everything is easily configured to suit your application.


Is there a demo?
----------------

Not yet, but I'll add once it's fully installed.


Schema?
-------

To follow. I need to clean it up and correct a mistake or two first.


Thanks
------

Thanks to [deepsyx](https://github.com/deepsyx) whose 
[arduino-tachometer](https://github.com/deepsyx/arduino-tachometer) project
provided inspiration and a little maths.


