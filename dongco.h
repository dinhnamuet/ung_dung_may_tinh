#ifndef __DONGCO_H__
#define __DONGCO_H__

/* define for device driver */
/* GPIO */
#define DEV_NAME "dongco"
#define GPIO_BASE_ADDR 0xFE200000
#define GPIO_LENGTH 0xF0
#define GPFSEL1_OFFSET 0x04
#define GPSET0_OFFSET 0x1C
#define GPCLR0_OFFSET 0x28
#define GPLEV0_OFFSET 0x34
#define PULLUP1_OFFSET 0xE8

/* define ioctl */
#define FORWARD _IOW('a', '1', uint32_t *)
#define REVERSE _IOW('a', '2', uint32_t *)
#define STOP _IOW('a', '3', uint32_t *)

#define PWM_BASE 0xFE002000
#define PWM_LENGTH 0x24
#define CTL_OFFSET 0x00
#define STA_OFFSET 0x04
#define DMAC_OFFSET 0x08
#define RNG1_OFFSET 0x10
#define DAT1_OFFSET 0x14
#define FIF1_OFFSET 0x18
#define RNG2_OFFSET 0x20
#define DAT2_OFFSET 0x24

#endif
