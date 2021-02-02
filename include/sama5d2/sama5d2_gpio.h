// GPIO driver for SAMA5D2 chips (kernel driver)

#ifndef SAMA5D2_GPIO_H
#define SAMA5D2_GPIO_H

#include <chaos/types.h>
#include <sama5d2/regmap.h>

enum gpio_func {
    GPIO_FUNC_A = 1,
    GPIO_FUNC_B,
    GPIO_FUNC_C,
    GPIO_FUNC_D,
    GPIO_FUNC_E,
    GPIO_FUNC_F,
    GPIO_FUNC_G
};

void sama5d2_gpio_set_func(struct gpio_reg* hw, u32 pin, enum gpio_func func);

#endif
