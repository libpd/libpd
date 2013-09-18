
This package includes a Pd object and a matching Arduino firmware.  It allows
you to control the Arduino board from Pd without having to program in
Arduino's C++.

To use it, first download and install Arduino 0018 or newer from
http://arduino.cc. When you run the Arduino program, then you'll be able to
choose firmware from Arduino application, in the menu: 

  File -> Open -> Examples -> Library-Firmata

Once you have uploaded a firmware to your Arduino board, open up
arduino-test.pd in Pd, then choose your serial port with the [open( message.

* for more on Arduino, go to: http://arduino.cc
* for more information about this firmware here: http://firmata.org

WARNING!  This version of the arduino object for Pd will only work with 2.1
versions of Firmata or newer!  It will not work with older versions of the firmware!

(You can also optionally unzip the included Firmata-2.2 firmware and follow
the README included in there.)

Hans-Christoph Steiner <hans@eds.org>
