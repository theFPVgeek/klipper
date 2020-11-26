// GPIO functions on PRU
//
// Copyright (C) 2017-2020  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "board/io.h" // readl
#include "command.h" // shutdown
#include "compiler.h" // ARRAY_SIZE
#include "gpio.h" // gpio_out_setup
#include "sched.h" // sched_shutdown


/****************************************************************
 * Pin mappings
 ****************************************************************/

#define GPIO(PORT, NUM) ((PORT) * 32 + (NUM))
#define GPIO2PORT(PIN) ((PIN) / 32)
#define GPIO2BIT(PIN) (1<<((PIN) % 32))

struct gpio_regs {
    uint32_t pad_0[77];
    volatile uint32_t oe;
    volatile uint32_t datain;
    volatile uint32_t dataout;
    uint32_t pad_140[20];
    volatile uint32_t cleardataout;
    volatile uint32_t setdataout;
};

DECL_ENUMERATION_RANGE("pin", "gpio0_0", GPIO(0, 0), 32);
DECL_ENUMERATION_RANGE("pin", "gpio1_0", GPIO(1, 0), 32);
DECL_ENUMERATION_RANGE("pin", "gpio2_0", GPIO(2, 0), 32);
DECL_ENUMERATION_RANGE("pin", "gpio3_0", GPIO(3, 0), 32);

static struct gpio_regs *digital_regs[] = {
    (void*)0x44e07000, (void*)0x4804c000, (void*)0x481ac000, (void*)0x481ae000
};


/****************************************************************
 * General Purpose Input Output (GPIO) pins
 ****************************************************************/

struct gpio_out
gpio_out_setup(uint8_t pin, uint8_t val)
{
    if (GPIO2PORT(pin) >= ARRAY_SIZE(digital_regs))
        goto fail;
    struct gpio_regs *regs = digital_regs[GPIO2PORT(pin)];
    uint32_t bit = GPIO2BIT(pin);
    struct gpio_out rv = (struct gpio_out){.reg=&regs->cleardataout, .bit=bit};
    gpio_out_write(rv, val);
    return rv;
fail:
    shutdown("Not an output pin");
}

void
gpio_out_reset(struct gpio_out g, uint8_t val)
{
    shutdown("PRU does not support push/pull pins");
}

void
gpio_out_toggle_noirq(struct gpio_out g)
{
    gpio_out_write(g, !(*g.reg & g.bit));
}

void
gpio_out_toggle(struct gpio_out g)
{
    gpio_out_toggle_noirq(g);
}

void
gpio_out_write(struct gpio_out g, uint8_t val)
{
    volatile uint32_t *reg = g.reg;
    if (val)
        reg++;
    *reg = g.bit;
}


struct gpio_in
gpio_in_setup(uint8_t pin, int8_t pull_up)
{
    if (GPIO2PORT(pin) >= ARRAY_SIZE(digital_regs))
        goto fail;
    struct gpio_regs *regs = digital_regs[GPIO2PORT(pin)];
    uint32_t bit = GPIO2BIT(pin);
    return (struct gpio_in){ .reg=&regs->datain, .bit=bit };
fail:
    shutdown("Not an input pin");
}

void
gpio_in_reset(struct gpio_in g, int8_t pull_up)
{
    shutdown("PRU does not support push/pull pins");
}

uint8_t
gpio_in_read(struct gpio_in g)
{
    return !!(*g.reg & g.bit);
}
