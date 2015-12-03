README.md {#README}
=========

[TOC]

# Android Debug Bridge via Netcat File System (adbncfs) {#section1}

## Abstract {#abstract}

This is a [FUSE] based file system client to mount the file system of your
android device on your linux host.

The idea of adbncfs was taken from [ADBFS] file system.

[ADBFS] uses `adb shell` commands to communicate with the device.
This involves creation of an `adb` process for every command to execute on the
device and that's why it is very very slow, especially if using a  file
manager.

To overcome this limitation I use [NETCAT] instead of `adb shell` to
communicate with the device.

At startup a [NETCAT] process is started on both the android device and on
the local host. Additional a `adb` port forwarding is setup. Now the shell
commands are sent to the local [NETCAT], then via port forwarding to the
[NETCAT] on the android device which forwards the shell command to the bash
shell. The output is sent the reverse way back.

[ADB] shell is still used to set up port forwarding and to start and kill the
[NETCAT] process on the android device. [ADB] push and pull are used to move
files to and from the device.


### Features of this implementation are:

- Based on [FUSE] (the best userspace file system framework for linux ;-)
- Multithreading: more than one request can be on it's way to the device
- Caching of file attributes and resolved links
- well-behaved filesystem, allowing tools like find(1) df(1) and rsync(1) to work as expected

## How to mount a android file system {#howtomount}

Once `adbncfs` is installed (see @ref installing) running it is very simple:

Make sure that
- [BUSYBOX] is installed on your device [how-to-install-busybox]
- USB Debugging is enabled [enable-usb-debug]
- your device is connected to the USB port

### To mount the file system:

Create a mount point if needed (e.g. in your home directory)

    > mkdir ~/mountpoint

For regular use, simply specifying a mountpoint will have the first available
android device mounted under it.

    > adbncfs mountpoint

Note, that it's recommended to run it as user, not as root.

### To unmount the filesystem:

    > fusermount -u mountpoint

## Requirements {#requirements}

- libfuse-dev
  * on debian execute `sudo apt-get install libfuse-dev`
- pkg-config
  * on debian execute `sudo apt-get install pkg-config`
- An install of gcc that supports the C++11 standard.
- [ADB]
  * on debian execute `sudo apt-get install android-tools-adb`
- [BUSYBOX] installed on your android device
  * [how-to-install-busybox]
- optional
  + cppunit
    * on debian execute `sudo apt-get install libcppunit-1.13-0 libcppunit-dev`
  + cppcheck
    * on debian execute `sudo apt-get install cppcheck`
  + doxygen
    * on debian execute `sudo apt-get install doxygen graphviz`

## Installing {#installing}

Compile `adbncfs` the usual way:

    >  make
    >  make install (as root)

And you are ready to go.

## Known bugs {#knownbugs}

- problems with files with ? in filenames (adb bug?)

## Not yet implemented {#notyetimplemented}

The following [FUSE] callback functions are not yet implemented:
- fgetattr
- symlink
- link
- chmod
- chown
- ftruncate
- fsyncdir
- lock
- bmap
- setxattr
- getxattr
- listxattr
- ioctl
- poll


[ADBFS]: http://collectskin.com/adbfs/ "ADBFS"
[FUSE]: http://fuse.sourceforge.net/ "FUSE"
[ADB]: http://developer.android.com/tools/help/adb.html "ADB"
[NETCAT]: https://en.wikipedia.org/wiki/Netcat "netcat"
[BUSYBOX]: http://www.busybox.net/downloads/BusyBox.html "busybox"
[enable-usb-debug]: http://www.droidviews.com/how-to-enable-developer-optionsusb-debugging-mode-on-devices-with-android-4-2-jelly-bean/ "How to enable usb debugging"
[how-to-install-busybox]: http://forums.androidcentral.com/software-development-hacking/4446-how-installing-busybox-hand.html "How to install busybox"