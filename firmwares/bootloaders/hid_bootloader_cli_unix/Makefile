OS ?= LINUX
#OS ?= WINDOWS
#OS ?= MACOSX
#OS ?= BSD

ifeq ($(OS), LINUX)  # also works on FreeBSD
CC ?= gcc
CFLAGS ?= -O2 -Wall
hid_bootloader_cli_unix: hid_bootloader_cli_unix.c
	$(CC) $(CFLAGS) -s -DUSE_LIBUSB -o hid_bootloader_cli_unix hid_bootloader_cli_unix.c -lusb


else ifeq ($(OS), WINDOWS)
CC = i586-mingw32msvc-gcc
CFLAGS ?= -O2 -Wall
LDLIB = -lsetupapi -lhid
hid_bootloader_cli_unix.exe: hid_bootloader_cli_unix.c
	$(CC) $(CFLAGS) -s -DUSE_WIN32 -o hid_bootloader_cli_unix.exe hid_bootloader_cli_unix.c $(LDLIB)


else ifeq ($(OS), MACOSX)
CC ?= gcc
SDK ?= /Developer/SDKs/MacOSX10.5.sdk
CFLAGS ?= -O2 -Wall
hid_bootloader_cli_unix: hid_bootloader_cli_unix.c
	$(CC) $(CFLAGS) -DUSE_APPLE_IOKIT -isysroot $(SDK) -o hid_bootloader_cli_unix hid_bootloader_cli_unix.c -Wl,-syslibroot,$(SDK) -framework IOKit -framework CoreFoundation


else ifeq ($(OS), BSD)  # works on NetBSD and OpenBSD
CC ?= gcct
CFLAGS ?= -O2 -Wall
hid_bootloader_cli_unix: hid_bootloader_cli_unix.c
	$(CC) $(CFLAGS) -s -DUSE_UHID -o hid_bootloader_cli_unix hid_bootloader_cli_unix.c


endif


clean:
	rm -f hid_bootloader_cli_unix hid_bootloader_cli_unix.exe
