//*****************************************************************************
//  Copyright 2012 - 2014 Paul Chote
//  This file is part of lightbox, which is free software. It is made available
//  to you under version 3 (or later) of the GNU General Public License, as
//  published by the Free Software Foundation and included in the LICENSE file.
//*****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>

#include "usb.h"
#include "main.h"
#include "simulation.h"

#define MAX_DATA_LENGTH 200

// Must be less than MAX_DATA_LENGTH - 5
#define MAX_SIMULATION_NAME_LENGTH 40
#define MAX_SIMULATION_DESC_LENGTH 150

enum packet_state { HEADERA = 0, HEADERB, TYPE, LENGTH, DATA, CHECKSUM, FOOTERA, FOOTERB };
enum packet_type
{
    REQUEST_MODES = 'A',
    SET_MODE = 'B',
    MESSAGE = 'C',
    SIMULATION_TYPE = 'D',
    SIMULATION_COUNT = 'E',
};

struct packet_message
{
    uint8_t length;
    char str[MAX_DATA_LENGTH-1];
};

struct packet_simulation
{
    uint8_t id;
    uint16_t exptime;

    // Add an extra byte for a null terminator
    char name[MAX_SIMULATION_NAME_LENGTH + 1];
    uint8_t name_length;

    char desc[MAX_SIMULATION_DESC_LENGTH + 1];
    uint8_t desc_length;
};

struct packet_simulation_count
{
    uint8_t total;
    uint8_t active;
};

struct packet_set_mode
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
        struct packet_set_mode mode;
    } data;
};

const char unknown_packet_fmt[]  PROGMEM = "Unknown packet type '%c' - ignoring";
const char long_packet_fmt[]     PROGMEM = "Ignoring long packet: %c (length %u)";
const char checksum_failed_fmt[] PROGMEM = "Packet checksum failed. Got 0x%02x, expected 0x%02x";
const char invalid_packet_fmt[]  PROGMEM = "Invalid packet end byte. Got 0x%02x, expected 0x%02x";
const char got_packet_fmt[]      PROGMEM = "Got packet type '%c'";

static uint8_t input_buffer[256];
static uint8_t input_read = 0;
static volatile uint8_t input_write = 0;

static uint8_t output_buffer[256];
static volatile uint8_t output_read = 0;
static volatile uint8_t output_write = 0;

// Add a byte to the send buffer.
// Will block if the buffer is full
void queue_byte(uint8_t b)
{
    // Don't overwrite data that hasn't been sent yet
    while (output_write == (uint8_t)(output_read - 1));

    output_buffer[output_write++] = b;

    // Enable transmit if necessary
    UCSR0B |= _BV(UDRIE0);
}

// Send data from RAM
static void queue_data(uint8_t type, const void *data, uint8_t length)
{
    // Header
    queue_byte('$');
    queue_byte('$');
    queue_byte(type);
    queue_byte(length);

    // Data
    uint8_t checksum = 0;
    for (uint8_t i = 0; i < length; i++)
    {
        uint8_t b = ((uint8_t *)data)[i];
        queue_byte(b);
        checksum ^= b;
    }

    // Footer
    queue_byte(checksum);
    queue_byte('\r');
    queue_byte('\n');
}

static bool byte_available()
{
    return input_write != input_read;
}

static uint8_t read_byte()
{
    // Loop until data is available
    while (input_read == input_write);
    return input_buffer[input_read++];
}

ISR(USART_UDRE_vect)
{
    if (output_write != output_read)
        UDR0 = output_buffer[output_read++];

    // Ran out of data to send - disable the interrupt
    if (output_write == output_read)
        UCSR0B &= ~_BV(UDRIE0);
}

ISR(USART_RX_vect)
{
    input_buffer[(uint8_t)(input_write++)] = UDR0;
}

void usb_initialize()
{
#define BAUD 9600
#include <util/setbaud.h>
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;
#if USE_2X
    UCSR0A = _BV(U2X0);
#endif

    // Enable receive, transmit, data received interrupt
    UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0);

    input_read = input_write = 0;
    output_read = output_write = 0;
}

static void parse_packet(struct timer_packet *p)
{
    usb_send_message_fmt_P(got_packet_fmt, p->type);
    switch (p->type)
    {
        case REQUEST_MODES:
        {
            // Simulation numbering starts at 1
            usb_send_simulation_count(simulation_count, active_simulation);
            for (uint8_t i = 0; i < simulation_count; i++)
                usb_send_simulation_type(i+1, &simulation[i]);
            break;
        }
        case SET_MODE:
            // Simulation numbering starts at 1
            select_simulation(p->data.mode.id);
            break;
        default:
            usb_send_message_fmt_P(unknown_packet_fmt, p->type);
            break;
    }
}

void usb_tick()
{
    static struct timer_packet p = {.state = HEADERA};
    while (byte_available())
    {
        uint8_t b = read_byte();
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
                    usb_send_message_fmt_P(long_packet_fmt, p.type, p.length);
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
                    usb_send_message_fmt_P(checksum_failed_fmt, b, p.checksum);
                    p.state = HEADERA;
                }
                break;
            case FOOTERA:
                if (b == '\r')
                    p.state++;
                else
                {
                    usb_send_message_fmt_P(invalid_packet_fmt, b, '\r');
                    p.state = HEADERA;
                }
                break;
            case FOOTERB:
                if (b == '\n')
                    parse_packet(&p);
                else
                    usb_send_message_fmt_P(invalid_packet_fmt, b, '\n');
    
                p.state = HEADERA;
                break;
        }
    }
}

void usb_send_message_P(const char *string)
{
    struct packet_message msg;
    msg.length = strlen_P(string);
    if (msg.length > MAX_DATA_LENGTH-1)
        msg.length = MAX_DATA_LENGTH-1;

    strncpy_P(msg.str, string, msg.length);
    queue_data(MESSAGE, &msg, msg.length + 1);
}

void usb_send_message_fmt_P(const char *fmt, ...)
{
    va_list args;
    struct packet_message msg;

    va_start(args, fmt);
    int len = vsnprintf_P(msg.str, MAX_DATA_LENGTH, fmt, args);
    va_end(args);

    if (len > MAX_DATA_LENGTH-1)
        len = MAX_DATA_LENGTH-1;

    msg.length = (uint8_t)len;
    queue_data(MESSAGE, &msg, msg.length + 1);
}

#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
void usb_send_simulation_type(uint8_t index, struct simulation_parameters *params)
{
    struct packet_simulation sim;
    sim.id = index;
    sim.exptime = params->exptime;
    sim.name_length = MAX(strlen_P(params->name), MAX_SIMULATION_NAME_LENGTH);
    sim.desc_length = MAX(strlen_P(params->desc), MAX_SIMULATION_DESC_LENGTH);

    strncpy_P(sim.name, params->name, sim.name_length);
    sim.name[sim.name_length] = 0;
    strncpy_P(sim.desc, params->desc, sim.desc_length);
    sim.desc[sim.desc_length] = 0;

    queue_data(SIMULATION_TYPE, &sim, sizeof(struct packet_simulation));
}

void usb_send_simulation_count(uint8_t total, uint8_t active)
{
    struct packet_simulation_count count;
    count.total = total;
    count.active = active;
    queue_data(SIMULATION_COUNT, &count, sizeof(struct packet_simulation_count));
}

void usb_send_simulation_changed(uint8_t index)
{
    struct packet_set_mode sim;
    sim.id = index;
    queue_data(SET_MODE, &sim, sizeof(struct packet_set_mode));
}