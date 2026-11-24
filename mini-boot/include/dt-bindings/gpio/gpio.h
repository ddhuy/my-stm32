#ifndef MINIBOOT_DT_BINDINGS_GPIO_GPIO_H
#define MINIBOOT_DT_BINDINGS_GPIO_GPIO_H

/*
 * Flags in the last cell of a GPIO specifier (cs-gpios = <&gpioc 1 FLAGS>),
 * shared by C and the device tree; defines only. Values as in Linux.
 */
#define GPIO_ACTIVE_HIGH  0
#define GPIO_ACTIVE_LOW   1

#endif /* MINIBOOT_DT_BINDINGS_GPIO_GPIO_H */
