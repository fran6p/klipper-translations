// ADC functions on STM32
//
// Copyright (C) 2019  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "board/irq.h" // irq_save
#include "board/misc.h" // timer_from_us
#include "command.h" // shutdown
#include "compiler.h" // ARRAY_SIZE
#include "generic/armcm_timer.h" // udelay
#include "gpio.h" // gpio_adc_setup
#include "internal.h" // GPIO
#include "sched.h" // sched_shutdown

DECL_CONSTANT("ADC_MAX", 4095);

static const uint8_t adc_pins[] = {
    GPIO('A', 0), GPIO('A', 1), GPIO('A', 2), GPIO('A', 3),
    GPIO('A', 4), GPIO('A', 5), GPIO('A', 6), GPIO('A', 7),
    GPIO('B', 0), GPIO('B', 1), GPIO('C', 0), GPIO('C', 1),
    GPIO('C', 2), GPIO('C', 3), GPIO('C', 4), GPIO('C', 5),
#if CONFIG_MACH_STM32F4
    0x00, 0x00, 0x00, 0x00,
    GPIO('F', 6), GPIO('F', 7), GPIO('F', 8), GPIO('F', 9),
    GPIO('F', 10), GPIO('F', 3), 0x00, 0x00,
    0x00, 0x00, GPIO('F', 4), GPIO('F', 5),
#endif
};

#if CONFIG_MACH_STM32F1
#define CR2_FLAGS (ADC_CR2_ADON | (7 << ADC_CR2_EXTSEL_Pos) | ADC_CR2_EXTTRIG)
#else
#define CR2_FLAGS ADC_CR2_ADON
#endif

struct gpio_adc
gpio_adc_setup(uint32_t pin)
{
    // Find pin in adc_pins table
    int chan;
    for (chan=0; ; chan++) {
        if (chan >= ARRAY_SIZE(adc_pins))
            shutdown("Not a valid ADC pin");
        if (adc_pins[chan] == pin)
            break;
    }

    // Determine which ADC block to use
    ADC_TypeDef *adc = ADC1;
    uint32_t adc_base = ADC1_BASE;
#if CONFIG_MACH_STM32F4
    if (chan >= 16) {
        // On the STM32F4, some ADC channels are only available from ADC3
        adc = ADC3;
        adc_base += 0x800;
        chan -= 16;
    }
#endif

    // Enable the ADC
    if (!is_enabled_pclock(adc_base)) {
        enable_pclock(adc_base);
        uint32_t aticks = 3; // 2.5-3.2us (depending on stm32 chip)
        adc->SMPR1 = (aticks | (aticks << 3) | (aticks << 6) | (aticks << 9)
                      | (aticks << 12) | (aticks << 15) | (aticks << 18)
                      | (aticks << 21) | (aticks << 24));
        adc->SMPR2 = (aticks | (aticks << 3) | (aticks << 6) | (aticks << 9)
                      | (aticks << 12) | (aticks << 15) | (aticks << 18)
                      | (aticks << 21) | (aticks << 24) | (aticks << 27));
        adc->CR2 = CR2_FLAGS;

#if CONFIG_MACH_STM32F1
        // Perform calibration
        udelay(timer_from_us(1));
        adc->CR2 = ADC_CR2_CAL | CR2_FLAGS;
        while (adc->CR2 & ADC_CR2_CAL)
            ;
#endif
    }

    gpio_peripheral(pin, GPIO_ANALOG, 0);

    return (struct gpio_adc){ .adc = adc, .chan = chan };
}

// Try to sample a value. Returns zero if sample ready, otherwise
// returns the number of clock ticks the caller should wait before
// retrying this function.
uint32_t
gpio_adc_sample(struct gpio_adc g)
{
    ADC_TypeDef *adc = g.adc;
    uint32_t sr = adc->SR;
    if (sr & ADC_SR_STRT) {
        if (!(sr & ADC_SR_EOC) || adc->SQR3 != g.chan)
            // Conversion still in progress or busy on another channel
            goto need_delay;
        // Conversion ready
        return 0;
    }
    // Start sample
    adc->SQR3 = g.chan;
    adc->CR2 = ADC_CR2_SWSTART | CR2_FLAGS;

need_delay:
    return timer_from_us(10);
}

// Read a value; use only after gpio_adc_sample() returns zero
uint16_t
gpio_adc_read(struct gpio_adc g)
{
    ADC_TypeDef *adc = g.adc;
    adc->SR = ~ADC_SR_STRT;
    return adc->DR;
}

// Cancel a sample that may have been started with gpio_adc_sample()
void
gpio_adc_cancel_sample(struct gpio_adc g)
{
    ADC_TypeDef *adc = g.adc;
    irqstatus_t flag = irq_save();
    if (adc->SR & ADC_SR_STRT && adc->SQR3 == g.chan)
        gpio_adc_read(g);
    irq_restore(flag);
}