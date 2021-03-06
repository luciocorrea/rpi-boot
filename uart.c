/* Copyright (C) 2013 by John Cronin <jncronin@tysos.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdint.h>
#include <stdio.h>
#include "mmio.h"
#include "uart.h"
#include "timer.h"

extern uint32_t base_adjust;

#define GPIO_BASE 			0x20200000
#define GPPUD 				0x94
#define GPPUDCLK0 			0x98
#define UART0_BASE			0x20201000
#define UART0_DR			0x00
#define UART0_RSRECR			0x04
#define UART0_FR			0x18
#define UART0_ILPR			0x20
#define UART0_IBRD			0x24
#define UART0_FBRD			0x28
#define UART0_LCRH			0x2C
#define UART0_CR			0x30
#define UART0_IFLS			0x34
#define UART0_IMSC			0x38
#define UART0_RIS			0x3C
#define UART0_MIS			0x40
#define UART0_ICR			0x44
#define UART0_DMACR			0x48
#define UART0_ITCR			0x80
#define UART0_ITIP			0x84
#define UART0_ITOP			0x88
#define UART0_TDR			0x8C

static uint32_t uart_base = UART0_BASE;
static uint32_t gpio_base = GPIO_BASE;

void uart_set_base(uint32_t base)
{
	uart_base = base;
}

void gpio_set_base(uint32_t base)
{
	gpio_base = base;
}

void uart_init()
{
	// disable UART
	mmio_write(uart_base + UART0_CR, 0x0);

	// the following disables pullup/down for GPIO pins 14 and 15
	mmio_write(gpio_base + GPPUD, 0x0);
	usleep(150000);

	mmio_write(gpio_base + GPPUDCLK0, (1 << 14) | (1 << 15));
	usleep(150000);

	mmio_write(gpio_base + GPPUDCLK0, 0x0);

	// clear interrupts
	mmio_write(uart_base + UART0_ICR, 0x7ff);

	// set baud rate - commented out therefore use default
	//mmio_write(uart_base + UART0_IBRD, 1);
	//mmio_write(uart_base + UART0_FBRD, 40);

	// 8 bit, no parity, 1 stop bit - commented out therefore use default
	//mmio_write(uart_base + UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));

	// interrupt mask
	mmio_write(uart_base + UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) |
				(1 << 8) | (1 << 9) | (1 << 10));

	// enable device, transmit and receive
	mmio_write(uart_base + UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
}

int uart_putc(int byte)
{
	while(mmio_read(uart_base + UART0_FR) & (1 << 5))
        usleep(2000);

	mmio_write(uart_base + UART0_DR, (uint8_t)(byte & 0xff));

	if(byte == '\n')
		uart_putc('\r');

	return byte;
}

int uart_getc()
{
    while(mmio_read(uart_base + UART0_FR) & (1 << 4))
        usleep(2000);
    return mmio_read(uart_base + UART0_DR) & 0xff;
}

int uart_getc_timeout(useconds_t timeout)
{
    TIMEOUT_WAIT((mmio_read(uart_base + UART0_FR) & (1 << 4)) == 0, timeout);
    if((mmio_read(uart_base + UART0_FR) & (1 << 4)) == 0)
        return mmio_read(uart_base + UART0_DR) & 0xff;
    else
        return -1;
}

void uart_puts(const char *str)
{
	while(*str)
		uart_putc(*str++);
}

