// Register map definitions for SAMA5D2 chips

#ifndef SAMA5D2_REGMAP_H
#define SAMA5D2_REGMAP_H    

#include <chaos/types.h>

struct wdt_reg {
    __w u32 cr;
    _rw u32 mr;
    __r u32 sr;
};

#define WDT_REG ((struct wdt_reg *)0xf8048040)

struct gpio_reg {
    _rw u32 mskr;
    _rw u32 cfgr;
    __r u32 pdsr;
    __r u32 locksr;
    __w u32 sodr;
    __w u32 codr;
    _rw u32 odsr;
    __r u32 reserved1;
    __w u32 ier;
    __w u32 idr;
    __r u32 imr;
    __r u32 isr;
    __r u32 reserved2[3];
    __w u32 iofr;
};

#define GPIOA_REG ((struct gpio_reg *)0xfc038000)
#define GPIOB_REG ((struct gpio_reg *)0xfc038040)
#define GPIOC_REG ((struct gpio_reg *)0xfc038080)
#define GPIOD_REG ((struct gpio_reg *)0xfc0380c0)

struct uart_reg {
    __w u32 cr;
    _rw u32 mr;  
    __w u32 ier;
    __w u32 idr;
    __r u32 imr;
    __r u32 sr;
    __r u32 rhr;
    __w u32 thr;
    _rw u32 brgr;
    _rw u32 cmpr;
    _rw u32 rtor;
    __r u32 reserved[46];
    _rw u32 wpmr;
};

#define UART0_REG ((struct uart_reg *)0xf801c000)
#define UART1_REG ((struct uart_reg *)0xf8020000)
#define UART2_REG ((struct uart_reg *)0xf8024000)
#define UART3_REG ((struct uart_reg *)0xfc008000)
#define UART4_REG ((struct uart_reg *)0xfc00c000)

struct pmc_reg {
    __w u32 scer;
    __w u32 scdr;
    __r u32 scsr;
    __r u32 reserved1;
    __w u32 pcer0;
    __w u32 pcdr0;
    __r u32 pcsr0;
    _rw u32 uckr;
    _rw u32 mor;
    _rw u32 mcfr;
    _rw u32 pllar;
    __r u32 reserved2;
    _rw u32 mckr;
    __r u32 reserved3;
    _rw u32 usb;
    __r u32 reserved4;
    _rw u32 pck0;
    _rw u32 pck1;
    _rw u32 pck2;
    __r u32 reserved5[5];
    __w u32 ier;
    __w u32 idr;
    __r u32 sr;
    __r u32 imr;
    _rw u32 fsmr;
    _rw u32 fspr;
    _rw u32 focr;
    __r u32 reserved6;
    _rw u32 pllicpr;
    __r u32 reserved7[24];
    _rw u32 wpmr;
    __r u32 wpsr;
    __r u32 reserved8[5];
    __w u32 pcer1;
    __w u32 pcdr1;
    __r u32 pcsr1;
    _rw u32 pcr;
    _rw u32 ocr;
    __w u32 slpwk_er0;
    __w u32 slpwk_dr0;
    __r u32 slpwk_sr0;
    __r u32 slpwk_asr0;
    __r u32 reserved9[4];
    __w u32 slpwk_er1;
    __w u32 slpwk_dr1;
    __r u32 slpwk_sr1;
    __r u32 slpwk_asr1;
    __r u32 slpwk_aipr;
    _rw u32 slpwkcr;
    _rw u32 audio_pll0;
    _rw u32 audio_pll1;
};

#define PMC_REG ((struct pmc_reg *)0xf0014000)

struct pit_reg {
    _rw u32 mr;
    __r u32 sr;
    __r u32 pivr;
    __r u32 piir;
};

#define PIT_REG ((struct pit_reg *)0xf8048030)

struct apic_reg {
    _rw u32 ssr;
    _rw u32 smr;
    _rw u32 svr;
    __r u32 reserved0;
    _rw u32 ivr;        // warning: dont write unless you know what ur doing 
    __r u32 fvr;
    __r u32 isr;
    __r u32 reserved1;
    __r u32 ipr0;
    __r u32 ipr1;
    __r u32 ipr2;
    __r u32 ipr3;
    __r u32 imr;
    __r u32 cisr;
    __w u32 eoicr;
    _rw u32 spu;
    __w u32 iecr;
    __w u32 idcr;
    __w u32 iccr;
    __w u32 iscr;
    __r u32 reserved2[7];
    _rw u32 dcr;
    __r u32 reserved3[29];
    _rw u32 wpmr;
    __r u32 wpsr;
};

#define APIC_REG ((struct apic_reg *)0xfc020000)
#define SAPIC_REG ((struct apic_reg *)0xf803c000)

struct mmc_reg {
    _rw u32 ssar;
    _rw u16 bsr;
    _rw u16 bcr;
    _rw u32 arg1r;
    _rw u16 tmr;
    _rw u16 cr;
    __r u32 rr[4];
    _rw u32 bdpr;
    __r u32 psr;
    _rw u8  hc1r;
    _rw u8  pcr;
    _rw u8  bgcr;
    _rw u8  wcr;
    _rw u16 ccr;
    _rw u8  tcr;
    _rw u8  srr;
    _rw u16 nistr;
    _rw u16 eistr;
    _rw u16 nister;
    _rw u16 eister;
    _rw u16 nisier;
    _rw u16 eisier;
    __r u16 acesr;
    _rw u16 hc2r;
    __r u32 ca0r;
    _rw u32 ca1r;
    __r u32 mccar;
    __r u16 reserved0;
    __w u16 feraces;
    __w u32 fereis;
    __r u8  aesr;
    __r u8  reserved1[3];
    _rw u32 asar;
    __r u32 reserved2;
    _rw u16 pvr[8];
    __r u32 reserved3[35];
    __r u16 sisr;
    __r u16 hcvr;
    __r u32 reserved4[64];
    __r u32 apsr;
    _rw u8  mc1r;
    __w u8  mc2r;
    __r u16 reserved5;
    _rw u32 acr;
    _rw u32 cc2r;
    _rw u8  rtc1r;
    __w u8  rtc2r;
    __r u16 reserved6;
    _rw u32 rtcvr;
    _rw u8  rtister;
    _rw u8  rtisier;
    __r u16 reserved7;
    _rw u8  rtistr;
    __r u8  rtssr;
    __r u16 reserved8;
    _rw u32 tuncr;
    __r u32 reserved9[3];
    _rw u32 cacr;
    __r u32 reserved10[3];
    _rw u32 calcr;
};

#define MMC0_REG ((struct mmc_reg *)0xa0000000)
#define MMC1_REG ((struct mmc_reg *)0xb0000000)

struct ddr_reg {
    _rw u32 mr;
    _rw u32 rtr;
    _rw u32 cr;
    _rw u32 tpr0;
    _rw u32 tpr1;
    _rw u32 tpr2;
    __r u32 reserved0;
    _rw u32 lpr;
    _rw u32 md;
    __r u32 reserved1;
    _rw u32 lpddr2_lpddr3_lpr;
    _rw u32 lpddr2_lpddr3_ddr3_mr4;
    _rw u32 lpddr2_lpddr3_ddr3_cal;
    _rw u32 io_calibr;
    _rw u32 ocms;
    __w u32 ocms_key1;
    __w u32 ocms_key2;
    _rw u32 conf_arbiter;
    _rw u32 timeout;
    _rw u32 req_port_0123;
    _rw u32 req_port_4567;
    __r u32 bdw_port_0123;
    __r u32 bdw_port_4567;
    _rw u32 rd_data_path;
    _rw u32 mcfgr;
    _rw u32 maddr[8];
    __r u32 minfo[8];
    __r u32 reserved2[16];
    _rw u32 wpmr;
    __r u32 wpsr;
};

#define DDR_REG ((struct ddr_reg *)0xf000c000)

struct matrix_pri {
    _rw u32 a;
    _rw u32 b;
};

struct matrix_reg {
    _rw u32 mcfg[12];
    __r u32 reserved0[4];
    _rw u32 scfg[15];
    __r u32 reserved1;
    struct matrix_pri pri[15];
    __r u32 reserved2[21];
    __w u32 meier;
    __w u32 meidr;
    __r u32 meimr;
    __r u32 mesr;
    __r u32 mear[12];
    __r u32 reserved3[21];
    _rw u32 wpmr;
    __r u32 wpsr;
    __r u32 reserved4[5];
    _rw u32 ssr[15];
    __r u32 reserved5;
    _rw u32 sassr[15];
    __r u32 reserved6[2];
    _rw u32 srtsr[15];
    __r u32 reserved7;
    _rw u32 spselr1;
    _rw u32 spselr2;
    _rw u32 spselr3;
};

#define H32MX_REG ((struct matrix_reg *)0xfc03c000)
#define H64MX_REG ((struct matrix_reg *)0xf0018000)

struct sfr_reg {
    __r u32 reserved0;
    _rw u32 ddrcfg;
    __r u32 reserved1[2];
    _rw u32 ohciicr;
    __r u32 ohciisr;
    __r u32 reserved2[4];
    _rw u32 secure;
    __r u32 reserved3;
    _rw u32 utmicktrim;
    _rw u32 utmihstrim;
    _rw u32 utmifstrim;
    _rw u32 utmiswap;
    __r u32 reserved4[2];
    _rw u32 can;
    __r u32 sn0;
    __r u32 sn1;
    _rw u32 aicredir;
    _rw u32 l2cc_hramc;
    __r u32 reserved5[13];
    _rw u32 i2sclksel;
    _rw u32 qspiclk;
};

#define SFR_REG ((struct sfr_reg *)0xf8030000)

struct reset_reg {
    __w u32 cr;
    __r u32 sr;
    _rw u32 mr;
};

#define RST_REG ((struct reset_reg *)0xf8048000)

struct l2cache_reg {
    __r u32 idr;
    __r u32 typr;
    __r u32 reserved1[62];
    _rw u32 cr;
    _rw u32 acr;
    _rw u32 trcr;
    _rw u32 drcr;
    __r u32 reserved2[60];
    _rw u32 ecr;
    _rw u32 ecfgr1;
    _rw u32 ecfgr0;
    _rw u32 evr1;
    _rw u32 evr0;
    __r u32 imr;
    __r u32 misr;
    __r u32 risr;
    __r u32 icr;
    __r u32 reserved3[323];
    _rw u32 csr;
    __r u32 reserved4[15];
    _rw u32 ipalr;
    __r u32 reserved5[2];
    _rw u32 iwr;
    __r u32 reserved6[12];
    _rw u32 cpalr;
    __r u32 reserved7;
    _rw u32 cir;
    _rw u32 cwr;
    __r u32 reserved8[12];
    _rw u32 cipalr;
    __r u32 reserved;
    _rw u32 ciir;
    _rw u32 ciwr;
    __r u32 reserved9[64];
    __r u32 dlkr;
    __r u32 ilkr;
    __r u32 reserved10[398];
    _rw u32 dcr;
    __r u32 reserved11[7];
    _rw u32 pcr;
    __r u32 reserved12[7];
    _rw u32 powcr;
};

#define L2CAHCE_REG ((struct l2cache_reg *)0x00a00000)

struct trng_reg {
    __w u32 cr;
    __r u32 reserved0[3];
    __w u32 ier;
    __w u32 idr;
    __r u32 imr;
    __r u32 isr;
    __r u32 reserved1[12];
    __r u32 odata;
};

#define TRNG_REG ((struct trng_reg *)0xfc01c000)

struct timer_channel_reg {
    __w u32 ccr;
    _rw u32 cmr;
    _rw u32 smmr;
    __r u32 rab;
    __r u32 cv;
    _rw u32 ra;
    _rw u32 rb;
    _rw u32 rc;
    __r u32 sr;
    __w u32 ier;
    __w u32 idr;
    __r u32 imr;
    _rw u32 eemr;
    __r u32 reserved[3];
};

struct timer_reg {
    struct timer_channel_reg channel[3];

    __w u32 bcr;
    _rw u32 bmr;
    __w u32 qier;
    __w u32 qidr;
    __r u32 qimr;
    __r u32 qisr;
    _rw u32 fmr;
    __r u32 reserved[2];
    _rw u32 wpmr;
};

#define TIMER0_REG ((struct timer_reg *)0xf800c000)
#define TIMER1_REG ((struct timer_reg *)0xf8010000)

#define DMA_CHANNELS 16

struct dma_reg {
    __r u32 gtype;
    _rw u32 gcfg;
    _rw u32 gwac;
    __w u32 gie;
    __w u32 gid;
    __r u32 gim;
    __r u32 gis;
    __w u32 ge;
    __w u32 gd;
    __r u32 gs;
    _rw u32 grs;
    _rw u32 gws;
    __w u32 grws;
    __w u32 grwr;
    __w u32 gswr;
    __r u32 gsws;
    __w u32 gswf;
    __r u32 reserved0[3];

    struct {
        __w u32 cie;
        __w u32 cid;
        __r u32 cim;
        __r u32 cis;
        _rw u32 csa;
        _rw u32 cda;
        _rw u32 cnda;
        _rw u32 cndc;
        _rw u32 cubc;
        _rw u32 cbc;
        _rw u32 cc;
        _rw u32 cds_msp;
        _rw u32 csus;
        _rw u32 cdus;
        __r u32 reserved1[2];
    } channel[DMA_CHANNELS];
};

#define DMA0_REG ((struct dma_reg *)0xf0010000)
#define DMA1_REG ((struct dma_reg *)0xf0004000)

struct spi_reg {
    __w u32 cr;
    _rw u32 mr;
    __r u32 rdr;
    __w u32 tdr;
    __r u32 sr;
    __w u32 ier;
    __w u32 idr;
    __r u32 imr; 
    __r u32 reserved0[4];

    struct {
        _rw u32 csr;
    } chip_select[4];

    _rw u32 fmr;
    __r u32 flr;
    __r u32 cmpr;
    __r u32 reserved1[38];
    _rw u32 wpmr;
    __r u32 wpsr;
};

#define SPI0_REG ((struct spi_reg *)0xf8000000)
#define SPI1_REG ((struct spi_reg *)0xfc000000)

struct flexcom_reg {
    _rw u32 flex_mr;
    __r u32 reserved0[3];
    __r u32 flex_rhr;
    __r u32 reserved1[3];
    _rw u32 flex_thr;
    _rw u32 reserved2[119];
    __w u32 u_cr;
    _rw u32 u_mr;
    __w u32 u_ier;
    __w u32 u_idr;
    __r u32 u_imr;
    __r u32 u_sr;
    __r u32 u_rhr;
    __w u32 u_thr;
    _rw u32 u_brgr;
    _rw u32 u_rtor;
    _rw u32 u_ttgr;
};

#define FLEX0_REG ((struct flexcom_reg *)0xf8034000)
#define FLEX1_REG ((struct flexcom_reg *)0xf8038000)
#define FLEX2_REG ((struct flexcom_reg *)0xfc010000)
#define FLEX3_REG ((struct flexcom_reg *)0xfc014000)
#define FLEX4_REG ((struct flexcom_reg *)0xfc018000)

struct lcd_ctrl {
    __w u32 cher;
    __w u32 chdr;
    __r u32 chsr;
    __w u32 ier;
    __w u32 idr;
    __r u32 imr;
    __r u32 isr;
};

struct lcd_dma {
    _rw u32 head;
    _rw u32 addr;
    _rw u32 ctrl;
    _rw u32 next;
};

struct lcd_reg {
    _rw u32 lcdcfg0;
    _rw u32 lcdcfg1;
    _rw u32 lcdcfg2;
    _rw u32 lcdcfg3;
    _rw u32 lcdcfg4;
    _rw u32 lcdcfg5;
    _rw u32 lcdcfg6;
    __r u32 reserved0;
    __w u32 lcden;
    __w u32 lcddis;
    __r u32 lcdsr;
    __w u32 lcdier;
    __w u32 lcdidr;
    __r u32 lcdimr;
    __r u32 lcdisr;
    __w u32 attr;

    struct lcd_ctrl base_ctrl;
    struct lcd_dma base_dma;
    _rw u32 basecfg[7];
    __r u32 reserved1[46];

    struct lcd_ctrl ov1_ctrl;
    struct lcd_dma ov1_dma;
    _rw u32 ov1cfg[10];
    __r u32 reserved2[43];

    struct lcd_ctrl ov2_ctrl;
    struct lcd_dma ov2_dma;
    _rw u32 ov2cfg[10];
    __r u32 reserved3[43];

    struct lcd_ctrl heo_ctrl;
    struct lcd_dma heo_dma;
    struct lcd_dma heo_udma;
    struct lcd_dma heo_vdma;
    _rw u32 heocfg[42];
    __r u32 reserved4[67];

    struct lcd_ctrl pp_ctrl;
    struct lcd_dma pp_dma;
    _rw u32 ppcfg[6];
    __r u32 reserved5[31];

    _rw u32 baseclut[256];
    _rw u32 ov1clut[256];
    _rw u32 ov2clut[256];
    _rw u32 heoclut[256];
};

#define LCD_REG ((struct lcd_reg *)0xf0000000)

struct nic_reg {
    _rw u32 ncr;
    _rw u32 ncfgr;
    __r u32 nsr;
    _rw u32 ur;
    _rw u32 dcfgr;
    _rw u32 tsr;
    _rw u32 rbqb;
    _rw u32 tbqb;
    _rw u32 rsr;
    __r u32 isr;
    __w u32 ier;
    __w u32 idr;
    _rw u32 imr;
    _rw u32 man;
    __r u32 rpq;
    _rw u32 tpq;
    _rw u32 tpsf;
    _rw u32 rpsf;
    _rw u32 rjfml;
    __r u32 reserved0[13];
    _rw u32 hrb;
    _rw u32 hrt;
    struct {
        _rw u32 btm;
        _rw u32 top;
    } sa[4];
    _rw u32 tidm[4];
    _rw u32 wol;
    _rw u32 ipgs;
    _rw u32 svlan;
    _rw u32 tpfcp;
    _rw u32 samb1;
    _rw u32 samt1;
    __r u32 reserved1[3];
    _rw u32 nsc;
    _rw u32 scl;
    _rw u32 sch;
    __r u32 eftsh;
    __r u32 efrsh;
    __r u32 peftsh;
    __r u32 pefrsh;
    __r u32 reserved2[2];
    __r u32 otlo;
    __r u32 othi;
    __r u32 ft;
    __r u32 bcft;
    __r u32 mft;
    __r u32 pft;
    __r u32 bft64;
    __r u32 tbft127;
    __r u32 tbft255;
    __r u32 tbft511;
    __r u32 tbft1023;
    __r u32 tbft1518;
    __r u32 gtbft1518;
    __r u32 tur;
    __r u32 scf;
    __r u32 mcf;
    __r u32 ec;
    __r u32 lc;
    __r u32 dtf;
    __r u32 cse;
    __r u32 orlo;
    __r u32 orhi;
    __r u32 fr;
    __r u32 bcfr;
    __r u32 mfr;
    __r u32 pfr;
    __r u32 bfr64;
    __r u32 tbfr127;
    __r u32 tbfr255;
    __r u32 tbfr511;
    __r u32 tbfr1023;
    __r u32 tbfr1518;
    __r u32 tmxbfr;
    __r u32 ufr;
    __r u32 ofr;
    __r u32 jr;
    __r u32 fcse;
    __r u32 lffe;
    __r u32 rse;
    __r u32 ae;
    __r u32 rre;
    __r u32 roe;
    __r u32 ihce;
    __r u32 tce;
    __r u32 uce;
    __r u32 reserved3[2];
    _rw u32 tisubn;
    _rw u32 tsh;
    __r u32 reserved4[3];
    _rw u32 tsl;
    _rw u32 tn;
    __w u32 ta;
    _rw u32 ti;
    __r u32 eftsl;
    __r u32 eftn;
    __r u32 efrsl;
    __r u32 efrn;
    __r u32 peftsl;
    __r u32 peftn;
    __r u32 pefrsl;
    __r u32 pefrn;
    __r u32 reserved5[28];
    __r u32 rxlpi;
    __r u32 rxlpitime;
    __r u32 txlpi;
    __r u32 txlpitime;
    __r u32 reserved6[95];
    __r u32 isrpq[2];
    __r u32 reserved7[14];
    _rw u32 tbqbapq[2];
    __r u32 reserve8[14];
    _rw u32 rbqbapq[2];
    __r u32 reserved9[6];
    _rw u32 rbsrpq[2];
    __r u32 reserved10[5];
    _rw u32 cbscr;
    _rw u32 cbsisqa;
    _rw u32 cbsisqb;
    __r u32 reserved17[14];
    _rw u32 st1rpq[4];
    __r u32 reserved11[12];
    _rw u32 st2rpq[8];
    __r u32 reserved12[40];
    __w u32 ierpq[2];
    __r u32 reserved13[6];
    __w u32 idrpq[2];
    __r u32 reserved14[6];
    _rw u32 imrpq[2];
    __r u32 reserved15[38];
    _rw u32 st2er[4];
    __r u32 reserved16[4];
    struct {
        _rw u32 cw0;
        _rw u32 cw1;
    } st2[24];
};

#define NIC_REG ((struct nic_reg *)0xf8008000)

#endif
