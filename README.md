# os
operating system

## introduction
this is a personal operating system, it looks like this:
![screenshot](attachment/screenshot.png)

it supports:
1. multiple tasks, semaphore, message-based communication, memory management;
2. PCI bus, USB host controller (OHCI, EHCI);
3. USB mice (HID), USB flash disk (mass storage);
4. FAT32 file system;
5. BMP image file;
6. graphical user interface (GUI), SVGA;
7. wifi dongle (tp-link WN725N);

## usage

### step 1. setup BIOS
enter BIOS setup menu, select USB driver as boot device.
this is an example: https://www.asus.com/support/FAQ/1013017

### step 2. format bootable USB flash disk
use "flashboot" to format USB flash disk with bootable FAT32.
#### a. install flashboot
master/attachment/flashboot-2.3a-setup.exe
#### b. run flashboot and choose "MiniOS"
![step](attachment/flashboot%20-%202.png)
#### c. choose "Minimal DOS (FreeDOS)"
![step](attachment/flashboot%20-%203.png)
#### d. choose target USB disk
![step](attachment/flashboot%20-%204.png)
#### e. choose Filesystem "FAT32"
![step](attachment/flashboot%20-%205.png)

#### restart your PC to make sure the USB flash disk works well.

### step 3. update file system
run master/fs.exe **as administrator**. select "HDD" and then "start".

![step](attachment/fs%20-%201.png)
#### if the USB flash disk CANNOT boot, run fs.exe again and select "FDD".

### step 4. boot PC from USB flash disk

### step 5. some shell commands
help (list commands)

svesa (change the resolution)

desktop (use picture as background)
