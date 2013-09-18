
hid - a Pd object for getting data from USB HID devices

[hid] is an object for reading data from USB HID devices like keyboards, mice,
joysticks, gamepads, keypads, and all sorts of other esoteric controllers like
USB knobs, touchscreens, Apple IR Remotes, etc. It represents the data with a
cross-platform message scheme which is then translated to the underlying
native API for input devices (Linux input.h or Mac OS X HID Utilities).

For GNU/Linux, there are varying restrictions on getting the USB HID
data, so you will need to make sure you have permissions to read the
/dev/input/event* devices in order to get data with [hid].  There are
some docs to help you with this:

http://puredata.info/docs/tutorials/HowToReadHIDDevicesInLinuxWithoutBeingRoot/

For those interested in output support, and a cleaner message system,
checkout the alpha [hidio]:

http://puredata.info/community/projects/software/hidio
http://puredata.info/dev/HidIO
