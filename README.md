# ViveToPi

## Summary

ViveToPi is an ongoing personal project, meant to practice various programming techniques and disciplines. Functionally, the program allows a Raspberry Pi to communicate with an HTC Vive virtual reality headset. A command line terminal reads in user input to control a list of instructions, which it then uses to load, process and draw images to the headset.

## Setup

Changes to the boot configuration were necessary to draw to the Vive properly. The following lines were added to /boot/config.txt to allow the Pi to properly set up the framebuffer.

```
max_framebuffer_width = 2160
max_framebuffer_height = 1200

[EDID=HVR-HTC-VIVE]
hdmi_drive = 2
hdmi_group = 2
hdmi_mode = 87
hdmi_timings=2160 1 40 20 46 1200 1 28 2 234 0 0 0 60 0 297000000 5

[pi4]
max_framebuffers = 2
```

The following line also had to be commented out, to disable vc4-fkms-v3d.

```
[all]
#dtoverlay = vc4-fkms-v3d
```

X11 had to be disabled, disabling the GUI and converting the Pi into a purely termianl-based console. It can be disabled on boot using the `raspi-config` tool.

The main monitor is plugged into the HDMI0 port, while the Vive is connected through an HDMI-Micro HDMI converter plugged into the HDMI1 port.

The libraries SIMD and OpenCV are used for image processing.
