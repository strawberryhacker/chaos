// Timer driver for SAMA5D2 chips (kernel driver)

#include <chaos/timer.h>
#include <chaos/kprint.h>

#include <sama5d2/regmap.h>
#include <sama5d2/sama5d2_clk.h>

// Initialize on of the timers. The timer instance we use doesn't matter since we will 
// disable it before the kernel start the device servers
void sama5d2_init() {
    struct timer_reg* const timer_reg = TIMER0_REG;

    // Configure the clocks for the peripheral. We use the generic clock generator to
    // provide a prescaled SLOWCLK signal at 1kHz
    sama5d2_per_clk_en(35);
    sama5d2_genricc_clk_en(35, 32, GCK_CLK_SLOW);

    // Disable the clock first
    timer_reg->channel[0].ccr = (1 << 1);

    // Setup capture mode with SLOWCLK
    timer_reg->channel[0].cmr = 0;
}

void sama5d2_restart() {
    struct timer_reg* const timer_reg = TIMER0_REG;

    // Call in the software trigger
    timer_reg->channel[0].ccr = (1 << 2) | (1 << 0);
}

u32 sama5d2_get_time() {
    struct timer_reg* const timer_reg = TIMER0_REG;
    return timer_reg->channel[0].cv;
}

// Make a new driver
const struct timer_iface sama5d2_timer_iface = {
    .init = sama5d2_init,
    .restart = sama5d2_restart,
    .get_time = sama5d2_get_time
};

const struct timer_iface* get_timer() {
    return &sama5d2_timer_iface;
}
