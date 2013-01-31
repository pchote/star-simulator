//*****************************************************************************
//  Toggles the DTR output on the USB chip to reset into the bootloader
//
//  Copyright 2013 Paul Chote
//  This file is part of lightbox, which is free software. It is made available
//  to you under version 3 (or later) of the GNU General Public License, as
//  published by the Free Software Foundation and included in the LICENSE file.
//*****************************************************************************

#include <stdio.h>
#ifdef _WIN32
#   include <windows.h>
#else
#   include <fcntl.h>
#   include <termios.h>
#   include <unistd.h>
#   include <sys/ioctl.h>
#endif

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: reset <path to port>\n");
        return 1;
    }

#ifdef _WIN32
    HANDLE port = CreateFile(argv[1], GENERIC_READ | GENERIC_WRITE, 0, 0,
                             OPEN_EXISTING, 0, 0);
    if (port == INVALID_HANDLE_VALUE)
    {
        printf("Failed to open port: %s\n", argv[1]);
        return 1;
    }

    EscapeCommFunction(port, SETDTR);
    Sleep(1000);
    EscapeCommFunction(port, CLRDTR);
    CloseHandle(port);
#else
    int port = open(argv[1], O_RDWR | O_NOCTTY | O_NDELAY);
    if (port == -1)
    {
        printf("Failed to open port: %s\n", argv[1]);
        return 1;
    }

    ioctl(port, TIOCMSET, &(int){TIOCM_DTR});
    sleep(1);
    ioctl(port, TIOCMSET, &(int){0});
    close(port);
#endif
    return 0;
}
