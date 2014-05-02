/*
 * Copyright 2010-2012 Paul Chote
 * This file is part of Puoko-nui, which is free software. It is made available
 * to you under the terms of version 3 of the GNU General Public License, as
 * published by the Free Software Foundation. For more information, see LICENSE.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "serial.h"

#ifdef _WIN32
#   include <windows.h>
#else
#   include <sys/time.h>
#endif


// Force gcc ABI for packed structs under windows
#ifdef _WIN32
#   define PACKED_STRUCT __attribute__((gcc_struct, __packed__))
#else
#   define PACKED_STRUCT __attribute__((__packed__))
#endif

// Message protocol definitions
#define MAX_DATA_LENGTH 200

// Must be less than MAX_DATA_LENGTH - 5
#define MAX_SIMULATION_NAME_LENGTH 40
#define MAX_SIMULATION_DESC_LENGTH 150

enum packet_state {HEADERA = 0, HEADERB, TYPE, LENGTH, DATA, CHECKSUM, FOOTERA, FOOTERB};
enum packet_type
{
    REQUEST_MODES = 'A',
    SET_MODE = 'B',
    MESSAGE = 'C',
    SIMULATION_TYPE = 'D',
    SIMULATION_COUNT = 'E',
};

struct PACKED_STRUCT packet_message
{
    uint8_t length;
    char str[MAX_DATA_LENGTH-1];
};

struct PACKED_STRUCT packet_simulation
{
    uint8_t id;
    uint16_t exptime;

    char name[MAX_SIMULATION_NAME_LENGTH + 1];
    uint8_t name_length;

    char desc[MAX_SIMULATION_DESC_LENGTH + 1];
    uint8_t desc_length;
};

struct PACKED_STRUCT packet_simulation_count
{
    uint8_t total;
    uint8_t active;
};

struct PACKED_STRUCT packet_set_mode
{
    uint8_t id;
};

struct timer_packet
{
    enum packet_state state;
    enum packet_type type;
    uint8_t length;
    uint8_t progress;
    uint8_t checksum;

    union
    {
        // Extra byte allows us to always null-terminate strings for display
        uint8_t bytes[MAX_DATA_LENGTH+1];
        struct packet_message message;
        struct packet_simulation simulation;
        struct packet_simulation_count count;
        struct packet_set_mode set_simulation;
    } data;
};

static void millisleep(int ms)
{
#ifdef _WIN32
    Sleep(ms);
#else
    nanosleep(&(struct timespec){ms / 1000, (ms % 1000)*1e6}, NULL);
#endif
}

struct packet_simulation_count config;
static void parse_packet(struct timer_packet *p)
{
    // Handle packet
    switch (p->type)
    {
        case MESSAGE:
        {
            // TODO: only display if debug mode is active
            p->data.message.str[p->data.message.length] = '\0';
            //printf("Debug message: %s\n", p->data.message.str);
            break;
        }
        case SIMULATION_TYPE:
        {
            struct packet_simulation *sim = &p->data.simulation;
            printf("    %2hhu     %s\n", sim->id, sim->name);
            printf(" %s  %s\n", sim->id == config.active ? "(active)" : "        ", sim->desc);
            printf("           Recommended exposure time: ~%gs\n", sim->exptime / 1000.0f);
            printf("\n");
            break;
        }
        case SET_MODE:
            printf("Changed simulation type to %hhu\n", p->data.set_simulation.id);
            break;
        case SIMULATION_COUNT:
            config = p->data.count;
            break;
        default:
            printf("Unknown packet type: %c\n", p->type);
    }
}

static int send_data(struct serial_port *port, uint8_t type, const void *data, uint8_t length)
{
    uint8_t *packet = calloc(length + 7, sizeof(uint8_t));
    if (!packet)
    {
        printf("Allocation failure\n");
        return 1;
    }

    // Header
    size_t i = 0;
    packet[i++] = '$';
    packet[i++] = '$';
    packet[i++] = type;
    packet[i++] = length;

    // Data
    uint8_t checksum = 0;
    for (uint8_t j = 0; j < length; j++)
    {
        uint8_t b = ((uint8_t *)data)[j];
        packet[i++] = b;
        checksum ^= b;
    }

    // Footer
    packet[i++] = checksum;
    packet[i++] = '\r';
    packet[i++] = '\n';

    ssize_t error = serial_write(port, packet, length + 7);
    if (error < 0)
        printf("Connection error %zd: %s\n", error, serial_error_string(error));

    free(packet);
    return error < 0 ? 1 : 0;
}

int query_response(struct serial_port *port)
{
    struct timer_packet p = (struct timer_packet){.state = HEADERA};
    uint8_t b;
    ssize_t status;
    size_t timeout = 0;

    for (;;)
    {
        status = serial_read(port, &b, 1);

        // Wait for some more data
        if (status == 0)
        {
            // Assume transmission is complete after 500ms without new data
            if (timeout >= 500)
                break;

            timeout += 10;
            millisleep(10);
            continue;
        }

        // New data; reset timeout
        timeout = 0;

        if (status < 0)
        {
            printf("Read error (%zd): %s\n", status, serial_error_string(status));
            goto error;
        }

        switch (p.state)
        {
            case HEADERA:
            case HEADERB:
                if (b == '$')
                    p.state++;
                else
                    p.state = HEADERA;
                break;
            case TYPE:
                p.type = b;
                p.state++;
                break;
            case LENGTH:
                p.length = b;
                p.progress = 0;
                p.checksum = 0;
                if (p.length == 0)
                    p.state = CHECKSUM;
                else if (p.length <= sizeof(p.data))
                    p.state++;
                else
                {
                    printf("Warning: ignoring long packet: %c (length %u)\n", p.type, p.length);
                    p.state = HEADERA;
                }
                break;
            case DATA:
                p.checksum ^= b;
                p.data.bytes[p.progress++] = b;
                if (p.progress == p.length)
                    p.state++;
                break;
            case CHECKSUM:
                if (p.checksum == b)
                    p.state++;
                else
                {
                    printf("Warning: Packet checksum failed. Got 0x%02x, expected 0x%02x.\n", b, p.checksum);
                    p.state = HEADERA;
                }
                break;
            case FOOTERA:
                if (b == '\r')
                    p.state++;
                else
                {
                    printf("Warning: Invalid packet end byte. Got 0x%02x, expected 0x%02x.\n", b, '\r');
                    p.state = HEADERA;
                }
                break;
            case FOOTERB:
                if (b == '\n')
                    parse_packet(&p);
                else
                    printf("Warning: Invalid packet end byte. Got 0x%02x, expected 0x%02x.\n", b, '\n');

                p.state = HEADERA;
                break;
        }
    }

    return 0;

error:
    return 1;
}

int main(int argc, char *argv[])
{
    char *device = "COM6";

    if (argc >= 2)
        device = argv[1];

    printf("Connecting to star simulator on %s\n", device);
    
    ssize_t error;
    struct serial_port *port = serial_new(device, 9600, &error);
    if (!port)
    {
        printf("Connection error %zd: %s\n", error, serial_error_string(error));
		printf("\n[Press enter to exit]\n");
		getchar();
		return 1;
    }

    if (send_data(port, REQUEST_MODES, NULL, 0))
        goto error;

    printf("Querying simulation types...\n\n");

    if (query_response(port) != 0)
        goto error;
    
    printf("Enter simulation number, then press enter to continue: ");
    
    char inputbuf[10];
    int sim = atoi(fgets(inputbuf, 10, stdin));
    if (sim <= 0 || sim > config.total)
    {
        printf("Invalid option selected\n");
        goto error;
    }
    
    struct packet_set_mode mode = { .id = sim };
    if (send_data(port, SET_MODE, &mode, sizeof(struct packet_set_mode)))
        goto error;

    printf("Waiting for response...\n\n");
    if (query_response(port) != 0)
        goto error;

error:
    printf("\n[Press enter to exit]\n");
    getchar();
    
    serial_free(port);
    return 1;
}
