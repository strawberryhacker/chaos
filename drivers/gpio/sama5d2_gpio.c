// // GPIO kernel driver for the SAMA5D2 chip

#include <sama5d2/sama5d2_gpio.h>
#include <chaos/assert.h>

void sama5d2_gpio_set_func(struct gpio_reg* hw, u32 pin, enum gpio_func func) {
    assert(pin < 32);

    hw->mskr = (1 << pin);
    hw->cfgr = func;
}
