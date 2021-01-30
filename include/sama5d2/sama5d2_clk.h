// Clock kernel driver for the SAMA5D2 chip

#ifndef SAMA5D2_CLK_H
#define SAMA5D2_CLK_H

#include <chaos/types.h>

enum gck_clk_source {
    GCK_CLK_SLOW,
    GCK_CLK_MAIN,
    GCK_CLK_PLLA,
    GCK_CLK_UPLL,
    GCK_CLK_MCK,
    GCK_CLK_AUDIO
};

void sama5d2_per_clk_en(u32 pid);
void sama5d2_per_clk_dis(u32 pid);
void sama5d2_genricc_clk_en(u32 pid, u8 div, enum gck_clk_source src);
void sama5d2_genricc_clk_dis(u32 pid);

#endif
