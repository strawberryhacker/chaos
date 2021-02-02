// Clock driver for SAMA5D2 chips (kernel driver)

#include <sama5d2/sama5d2_clk.h>
#include <sama5d2/regmap.h>
#include <chaos/assert.h>

// These functions takes in PID numbers defined in the Periheral chapter in the datasheet
// (SAMA5D2 datasheet page 57)

void sama5d2_per_clk_en(u32 pid) {
    assert(pid >= 2 && pid < 64);

    // Get a pointer to the hardware
    struct pmc_reg* hw = PMC_REG;

    if (pid < 32) {
        hw->pcer0 = (1 << pid);
    } else {
        pid -= 32;
        hw->pcer1 = (1 << pid);
    }
}

void sama5d2_per_clk_dis(u32 pid) {
    assert(pid >= 2 && pid < 64);

    // Get a pointer to the hardware
    struct pmc_reg* hw = PMC_REG;

    if (pid < 32) {
        hw->pcdr0 = (1 << pid);
    } else {
        pid -= 32;
        hw->pcdr1 = (1 << pid);
    }
}

void sama5d2_genricc_clk_en(u32 pid, u8 div, enum gck_clk_source src) {
    assert(pid >= 2 && pid < 64);
    assert(div != 0);

    // Get the hardware
    struct pmc_reg* hw = PMC_REG;

    // Enable read operation
    hw->pcr = pid;
    u32 reg = hw->pcr;
    reg &= ~((0xFF << 20) | (0x07 << 8) | 0x7F);

    // Set the division
    reg |= ((div - 1) << 20);

    // Set the clock source
    reg |= (src << 8);

    // Select the peripheral
    reg |= pid;

    // Enable clock
    reg |= (1 << 29);

    // Write operation
    reg |= (1 << 12);

    // Configure the generic clock
    hw->pcr = reg;
}

void sama5d2_genricc_clk_dis(u32 pid) {
    assert(pid >= 2 && pid < 64);

    // Get the hardware
    struct pmc_reg* hw = PMC_REG;

    // Enable read operation
    hw->pcr = pid;
    hw->pcr = hw->pcr & ~(1 << 29);
}
