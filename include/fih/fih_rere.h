#ifdef CONFIG_FIH_APR

#ifndef __FIH_RERE_H
#define __FIH_RERE_H

#include <linux/input/qpnp-power-on.h>

#define FIH_RERE_EMERGENCY_DLOAD      0xFF

#define FIH_RERE_KERNEL_BUG           0x20
#define FIH_RERE_KERNEL_PANIC         0x21
#define FIH_RERE_KERNEL_RESTART       0x22
#define FIH_RERE_KERNEL_SHUTDOWN      0x23
#define FIH_RERE_KERNEL_WDOG          0x24
#define FIH_RERE_MODEM_FATAL          0x25
#define FIH_RERE_SYSTEM_CRASH         0x26
#define FIH_RERE_UNKNOWN_RESET        0x27
#define FIH_RERE_OVER_TAMPERATURE     0x28
#define FIH_RERE_MEMORY_TEST          0x29

#endif

#endif
