/*
 * Gumstix Platforms
 *
 * Copyright (c) 2007 by Thorsten Zitterell <info@bitmux.org>
 *
 * Code based on spitz platform by Andrzej Zaborowski <balrog@zabor.org>
 *
 * This code is licensed under the GNU GPL v2.
 */
 
/*
 * WinQEMU GPL Disclaimer: For the avoidance of doubt, except that if any license choice
 * other than GPL is available it will apply instead, WinQEMU elects to use only the 
 * General Public License version 3 (GPLv3) at this time for any software where a choice of 
 * GPL license versions is made available with the language indicating that GPLv3 or any later
 * version may be used, or where a choice of which version of the GPL is applied is otherwise unspecified.
 * 
 * Please contact Yan Wen (celestialwy@gmail.com) if you need additional information or have any questions.
 */
 
/* 
 * Example usage:
 * 
 * connex:
 * =======
 * create image:
 * # dd of=flash bs=1k count=16k if=/dev/zero
 * # dd of=flash bs=1k conv=notrunc if=u-boot.bin
 * # dd of=flash bs=1k conv=notrunc seek=256 if=rootfs.arm_nofpu.jffs2
 * start it:
 * # qemu-system-arm -M connex -pflash flash -monitor null -nographic
 *
 * verdex:
 * =======
 * create image:
 * # dd of=flash bs=1k count=32k if=/dev/zero
 * # dd of=flash bs=1k conv=notrunc if=u-boot.bin
 * # dd of=flash bs=1k conv=notrunc seek=256 if=rootfs.arm_nofpu.jffs2
 * # dd of=flash bs=1k conv=notrunc seek=31744 if=uImage
 * start it:
 * # qemu-system-arm -M verdex -pflash flash -monitor null -nographic -m 289
 */

#include "hw.h"
#include "pxa.h"
#include "net.h"
#include "flash.h"
#include "sysemu.h"
#include "devices.h"
#include "boards.h"

static const int sector_len = 128 * 1024;

static void connex_init(ram_addr_t ram_size, int vga_ram_size,
                const char *boot_device,
                const char *kernel_filename, const char *kernel_cmdline,
                const char *initrd_filename, const char *cpu_model)
{
    struct pxa2xx_state_s *cpu;
    int index;

    uint32_t connex_rom = 0x01000000;
    uint32_t connex_ram = 0x04000000;

    if (ram_size < (connex_ram + connex_rom + PXA2XX_INTERNAL_SIZE)) {
        fprintf(stderr, "This platform requires %i bytes of memory\n",
                connex_ram + connex_rom + PXA2XX_INTERNAL_SIZE);
        exit(1);
    }

    cpu = pxa255_init(connex_ram);

    index = drive_get_index(IF_PFLASH, 0, 0);
    if (index == -1) {
        fprintf(stderr, "A flash image must be given with the "
                "'pflash' parameter\n");
        exit(1);
    }

    if (!pflash_cfi01_register(0x00000000, qemu_ram_alloc(connex_rom),
            drives_table[index].bdrv, sector_len, connex_rom / sector_len,
            2, 0, 0, 0, 0)) {
        fprintf(stderr, "qemu: Error registering flash memory.\n");
        exit(1);
    }

    cpu->env->regs[15] = 0x00000000;

    /* Interrupt line of NIC is connected to GPIO line 36 */
    smc91c111_init(&nd_table[0], 0x04000300,
                    pxa2xx_gpio_in_get(cpu->gpio)[36]);
}

static void verdex_init(ram_addr_t ram_size, int vga_ram_size,
                const char *boot_device,
                const char *kernel_filename, const char *kernel_cmdline,
                const char *initrd_filename, const char *cpu_model)
{
    struct pxa2xx_state_s *cpu;
    int index;

    uint32_t verdex_rom = 0x02000000;
    uint32_t verdex_ram = 0x10000000;

    if (ram_size < (verdex_ram + verdex_rom + PXA2XX_INTERNAL_SIZE)) {
        fprintf(stderr, "This platform requires %i bytes of memory\n",
                verdex_ram + verdex_rom + PXA2XX_INTERNAL_SIZE);
        exit(1);
    }
#ifndef _MSC_VER
	cpu = pxa270_init(verdex_ram, cpu_model ?: "pxa270-c0");
#else
    cpu = pxa270_init(verdex_ram, cpu_model ?cpu_model: "pxa270-c0");
#endif
    index = drive_get_index(IF_PFLASH, 0, 0);
    if (index == -1) {
        fprintf(stderr, "A flash image must be given with the "
                "'pflash' parameter\n");
        exit(1);
    }

    if (!pflash_cfi01_register(0x00000000, qemu_ram_alloc(verdex_rom),
            drives_table[index].bdrv, sector_len, verdex_rom / sector_len,
            2, 0, 0, 0, 0)) {
        fprintf(stderr, "qemu: Error registering flash memory.\n");
        exit(1);
    }

    cpu->env->regs[15] = 0x00000000;

    /* Interrupt line of NIC is connected to GPIO line 99 */
    smc91c111_init(&nd_table[0], 0x04000300,
                    pxa2xx_gpio_in_get(cpu->gpio)[99]);
}

#ifndef _MSC_VER
QEMUMachine connex_machine = {
    .name = "connex",
    .desc = "Gumstix Connex (PXA255)",
    .init = connex_init,
    .ram_require = (0x05000000 + PXA2XX_INTERNAL_SIZE) | RAMSIZE_FIXED,
};

QEMUMachine verdex_machine = {
    .name = "verdex",
    .desc = "Gumstix Verdex (PXA270)",
    .init = verdex_init,
    .ram_require = (0x12000000 + PXA2XX_INTERNAL_SIZE) | RAMSIZE_FIXED,
};
#else

QEMUMachine connex_machine = {
	"connex",
	"Gumstix Connex (PXA255)",
	connex_init,
	(0x05000000 + PXA2XX_INTERNAL_SIZE) | RAMSIZE_FIXED
};

QEMUMachine verdex_machine = {
	"verdex",
	"Gumstix Verdex (PXA270)",
	verdex_init,
	(0x12000000 + PXA2XX_INTERNAL_SIZE) | RAMSIZE_FIXED
};
#endif