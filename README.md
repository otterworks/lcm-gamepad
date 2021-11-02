`lcm-gamepad`
=============

A basic LCM gamepad/joystick publisher.

I'm tinkering with this to simplify the pilot controller for MESOBOT, but
hope to produce something fairly generic. Later I'll probably port it to ROS
or something with flatbuffers & nanomsg/0mq or similar.

My initial development will focus on the PS4 DualShock controller, because
that's what I have handy. MESOBOT has used the Xbox One controller in the
past, so I'll probably tackle that second.

building
--------
```
autoreconf -fi && ./configure && make clean all
```

running the program
-------------------

On the NUC with the DualSense controller:

```
export LCM_DEFAULT_URL=udpm://239.255.76.67:7667?ttl=1 ./lcm-gamepad -v -d /dev/input/event28

```

```
./lcm-gamepad -d /dev/input/by-id/usb-Sony_Computer_Entertainment_Wireless_Controller-event-joystick
```

or

```
./lcm-gamepad -d /dev/input/event28
```

connecting via USB
------------------

For me, this just works.

connecting via Bluetooth
------------------------

The machines I've developed this on use `bluetoothctl` from the command
line. Your desktop manager may also have Bluetooth widgets.

```
bluetoothctl
power on
agent on
scan on
```

...look for something labeled `Wireless Controller` and note the MAC address
(12-character hexadecimal).

```
connect 01:23:45:67:89:AB
trust 01:23:45:67:89:AB
```

More recently, I've seen a prompt:

```
Authorize service
[agent] Authorize service 00001124-0000-1000-8000-00805f9b34fb (yes/no):
```

It works when I type `yes`.


finding the proper device
-------------------------

Sometimes (at least on my computers) you will not see a symlink for your
gamepad in `/dev/input/by-id/`. To figure out which of the numbered options
in the `/dev/input/event*` list is your gamepad:

- look in `/proc/bus/input/devices`, usually at the end (e.g.,
  `tail -n 13 /proc/bus/input/devices`)

- look at `/sys/class/input/event*/device/name` (e.g.
  ```
  for e in /sys/class/input/event*; do 
    echo ${e};
    cat ${e}/device/name;
  done;
  ```

formatting
----------

```
clang-format -i --style=mozilla src/*.c
```

see also
--------

- https://wiki.archlinux.org/index.php/Gamepad#evdev_API

- https://gamepad-tester.com/


