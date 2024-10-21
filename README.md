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

## Design

A command line terminal is provided to allow a user to create a list of instructions to produce images to be drawn on the monitor and the headset. There are three main types of instructions.

- **Layer instructions** provide a new image to be manipulated. At the moment, layers can only be generated from the headset's front-facing camera and PNG images stored in the Images folder. Layers are given a user-defined name to allow access for processing and drawing.
- **Process instructions** tell the program how to change provided layers. As the list of instructions is process sequentially, only layers that were defined above the instruction can be processed by it. For example, a process instruction at the third spot on the list can't process a layer defined on the fourth.
- **Draw instructions** draw the selected layer to the selected framebuffers, which can be changed using the `display` command. Ideally, only one draw instruction is ever on the list at any point, and only as the last instruction.

Users can manipulate the instruction list using the `push`, `edit`, and `delete` commands. The terminal's `help` message is as follows.

```
*************************************************************
* HELP                                                      *
*************************************************************
* help -> Display this message                              *
* print instructions -> Print the current instruction list  *
* clear -> Delete all instructions in the instruction list  *
* save [NAME] -> Save the current instruction list in       *
*                file [NAME].inli                           *
* load [NAME] -> Load an instruction list from file         *
*                [NAME].inli                                *
* push [INSTRUCTION] -> Add a new instruction at the end of *
*                       the list                            *
* push [#] [INSTRUCTION] -> Push a new instruction directly *
*                          at Instruction [#]               *
* edit [#] [INSTRUCTION] -> Edit Instruction [#]            *
* delete [#] -> Delete Instruction [#]                      *
*                                                           *
* display [vive | monitor] [true | false] -> set the video  *
*                                            output         *
* exit -> Exit the program                                  *
*************************************************************
* INSTRUCTIONS                                              *
*************************************************************
* layer [NAME] camera -> Grab the most recent camera frame  *
* layer [NAME] image [FILE] -> Load a .PNG file             *
*                                                           *
* process [NAME] resize [ dimensions [INT] [INT] | scale    *
*				[FLT] ] -> Resize a layer   *
* process [NAME] rotate [FLT] -> Rotate a layer to an angle *
* process [NAME] alpha [ flat [INT] | circular [INT]        *
*               [INT] ] -> Set a flat alpha, or a circular  *
*               patterned alpha (inner, outer)              *
* process [NAME] text [STR] -> Print text on a layer        *
* process [NAME] overlay [NAME] -> Overlay a layer onto     *
*                                    another layer          *
*                                                           *
* draw [NAME] -> Draw a layer to the selected outputs       *
*************************************************************
```

In a constantly running `while` loop, the program will try to process its instruction list as many times per second as possible, each loop generating a single frame. Naturally, this means that as the length of the instruction list goes up and the number of instructions it needs to generate for each loop goes up, the visible framerate of any changes goes down.

The program is also designed to be as compartmentalized as possible. `terminal.h` and `canvas.h` are completely seperate, and are connected only by `terminal_function.h`. Keeping the runnable file as simple as possible and the headers as compartmentalized as possible is one focus of the design, and an important part of any future development.

The terminal verifies commands through a tree structure generated from a flat-text file, allowing valid commands to be easily added without recompilation. Naturally, however, actual functionality does need to be added in `terminal_functions.h`. Commands are sent to `terminal_functions.h`, along with flags they find in the tree structure (also from the flat-text file), allowing various aspects of the command functionality to be handled with `switch` statements.

## TODO

### Major Features

- Optimize the functions `Canvas::draw`, `Layer::overlay`, `Layer::resize` and `Layer::copy`. These functions are computationally expensive and performed frequently, and consequently drag down the framerate.
  - O'Reilly's *Optimized C++* by Kurt Guntheroth
- Add a 3D Image Manager, allowing a loaded `.OBJ` file to be loaded and turned into a transformable wireframe image.
- Add an Audio Manager, allowing `.MP3` files to be loaded an played through the Vive's audio output.
- Experiment with the Vive's Bluetooth motion tracking equipment.

### Minor Features

- Refactor `public`/`private` variables and methods.
- Refactor `terminl_functions.h` as an interface, easing reuse of `terminal.h` in other programs.
- Make instruction list persistantly display on screen, removing the need to use the `print instructions` command.
- Add flat-text file containing programs default settings, such as `Text` settings and default display outputs.
- Add command to change `Text` styling, editing font size, color, etc.
