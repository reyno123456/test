#/bin/sh

export USB_DEV_CLASS_HID_ENABLE=1
make -f Makefile.cpu0 $1
