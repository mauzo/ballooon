/* ubx_pkt.h
 * UBX packet definitions
 */

#ifndef __UBX_PKT_H
#define __UBX_PKT_H

#define UBX_HEAD \
    uint16_t    type; \
    uint16_t    len;

#define UBX_HEADSIZ 4

/* The length field should not count the header */
#define ubx_len(t) (sizeof(t) - UBX_HEADSIZ)

typedef struct {
    UBX_HEAD
    uint8_t     dat[1];
} ubx_pkt;

/* The type values are all backwards (0x id class), because the AVR is
 * little-endian. (I should probably be using htons.)
 */

#define UBX_TYP_ACK 0x0105
typedef struct {
    UBX_HEAD
    uint16_t    ack_type;
} ubx_ack;

#define UBX_TYP_CFG_PRT 0x0006
typedef struct {
    UBX_HEAD
    uint8_t     port;
#define UBX_CFGPRT_PORT_DDC     0
#define UBX_CFGPRT_PORT_UART    1
#define UBX_CFGPRT_PORT_USB     3
#define UBX_CFGPRT_PORT_SPI     4
    uint8_t     reserved0;
    uint16_t    tx_ready;
#define UBX_CFGPRT_TX_EN        0x01
#define UBX_CFGPRT_TX_POL       0x02
#define UBX_CFGPRT_TX_PIN(x)    ((x) << 2)
#define UBX_CFGPRT_TX_THRES(x)  ((x) << 7)
    uint32_t    mode;
#define UBX_CFGPRT_UART_7BIT    (2 << 6)
#define UBX_CFGPRT_UART_8BIT    (3 << 6)
#define UBX_CFGPRT_UART_EPAR    (0 << 9)
#define UBX_CFGPRT_UART_OPAR    (1 << 9)
#define UBX_CFGPRT_UART_NPAR    (8 << 9)
#define UBX_CFGPRT_UART_1ST     (0 << 12)
#define UBX_CFGPRT_UART_15ST    (1 << 12)
#define UBX_CFGPRT_UART_2ST     (2 << 12)
#define UBX_CFGPRT_UART_05ST    (3 << 12)

#define UBX_CFGPRT_SPI_MODE(x)  ((x) << 1)
#define UBX_CFGPRT_SPI_FLOW     (1<<6)
#define UBX_CFGPRT_SPI_FFCNT(x) ((x) << 8)

#define UBX_CFGPRT_DDC_ADDR(x)  ((x) << 1)
    uint32_t    baud_rate;
    uint16_t    in_proto_mask;
    uint16_t    out_proto_mask;
#define UBX_CFGPRT_PROTO_UBX    0x01
#define UBX_CFGPRT_PROTO_NMEA   0x02
#define UBX_CFGPRT_PROTO_RTCM   0x04
    uint16_t    flags;
#define UBX_CFGPRT_FLAGS_XTXTMO (1<<1)
    uint16_t    reserved5;
} ubx_cfg_prt;

#define UBX_TYP_CFG_NAV5 0x2406
typedef struct {
    UBX_HEAD
    uint16_t    mask;
#define UBX_CFGNAV5_MASK_DYN        (1<<0)
#define UBX_CFGNAV5_MASK_MINEL      (1<<1)
#define UBX_CFGNAV5_MASK_POSFIX     (1<<2)
#define UBX_CFGNAV5_MASK_POSMASK    (1<<4)
#define UBX_CFGNAV5_MASK_TIME       (1<<5)
#define UBX_CFGNAV5_MASK_STATICHOLD (1<<6)
#define UBX_CFGNAV5_MASK_DGPS       (1<<7)
#define UBX_CFGNAV5_MASK_ALL        0xff
    uint8_t     dyn_model;
#define UBX_CFGNAV5_DYN_PORTABLE    0
#define UBX_CFGNAV5_DYN_STATIONARY  2
#define UBX_CFGNAV5_DYN_PEDESTRIAN  3
#define UBX_CFGNAV5_DYN_AUTOMOTIVE  4
#define UBX_CFGNAV5_DYN_SEA         5
#define UBX_CFGNAV5_DYN_AIRBORNE1G  6
#define UBX_CFGNAV5_DYN_AIRBORNE2G  7
#define UBX_CFGNAV5_DYN_AIRBORNE4G  8
    uint8_t     fix_mode;
#define UBX_CFGNAV5_FIX_2D          1
#define UBX_CFGNAV5_FIX_3D          2
#define UBX_CFGNAV5_FIX_AUTO        3
    int32_t     fixed_alt;
    uint32_t    fixed_alt_var;
    int8_t      min_elev;
    uint8_t     dr_limit;
    uint16_t    p_dop;
    uint16_t    t_dop;
    uint16_t    p_acc;
    uint16_t    t_acc;
    uint8_t     static_hold_thresh;
    uint8_t     dgps_timeout;
    uint8_t     cno_thresh_numSVs;
    uint8_t     cno_thresh;
    uint16_t    reserved2;
    uint32_t    reserved3;
    uint32_t    reserved4;
} ubx_cfg_nav5;

#define UBX_TYP_CFG_RST 0x0406
typedef struct {
    UBX_HEAD
    uint16_t    nav_bbr_mask;
#define UBX_CFGRST_BBR_HOT  0x0000
#define UBX_CFGRST_BBR_WARM 0x0001
#define UBX_CFGRST_BBR_COLD 0xffff
#define UBX_CFGRST_BBR_EPH      (1<<0)
#define UBX_CFGRST_BBR_ALM      (1<<1)
#define UBX_CFGRST_BBR_HEALTH   (1<<2)
#define UBX_CFGRST_BBR_KLOB     (1<<3)
#define UBX_CFGRST_BBR_POS      (1<<4)
#define UBX_CFGRST_BBR_CLKD     (1<<5)
#define UBX_CFGRST_BBR_OSC      (1<<6)
#define UBX_CFGRST_BBR_UTC      (1<<7)
#define UBX_CFGRST_BBR_RTC      (1<<8)
#define UBX_CFGRST_BBR_SFDR     (1<<11)
#define UBX_CFGRST_BBR_VMON     (1<<12)
#define UBX_CFGRST_BBR_TCT      (1<<13)
#define UBX_CFGRST_BBR_AOP      (1<<15)
    uint8_t     reset_mode;
#define UBX_CFGRST_RST_HARD     0x00
#define UBX_CFGRST_RST_SOFT     0x01
#define UBX_CFGRST_RST_GNSS     0x02
#define UBX_CFGRST_RST_AFTER    0x04
#define UBX_CFGRST_RST_STOP     0x08
#define UBX_CFGRST_RST_START    0x09
    uint8_t     reserved1;
} ubx_cfg_rst;

#define UBX_TYP_NAV_PVT 0x0701
typedef struct {
    UBX_HEAD
    uint32_t    iTOW;
    uint16_t    year;
    uint8_t     month;
    uint8_t     day;
    uint8_t     hour;
    uint8_t     min;
    uint8_t     sec;
    uint8_t     valid;
#define UBX_NAVPVT_VALID_DATE   (1<<0)
#define UBX_NAVPVT_VALID_TIME   (1<<1)
#define UBX_NAVPVT_VALID_FULL   (1<<2)
    uint32_t    t_acc;
    int32_t     nano;
    uint8_t     fix_type;
#define UBX_NAVPVT_FIX_NONE     0
#define UBX_NAVPVT_FIX_DRECK    1
#define UBX_NAVPVT_FIX_2D       2
#define UBX_NAVPVT_FIX_3D       3
#define UBX_NAVPVT_FIX_BOTH     4
#define UBX_NAVPVT_FIX_TIME     5
    int8_t      flags;
#define UBX_NAVPVT_FLAGS_GNSSOK (1<<0)
#define UBX_NAVPVT_FLAGS_DIFF   (1<<1)
#define UBX_NAVPVT_FLAGS_PSM(x) (((x)>>2) & 0x7)
#define UBX_NAVPVT_FLAGS_PSM_ENABLED    1
#define UBX_NAVPVT_FLAGS_PSM_AQUISION   2
#define UBX_NAVPVT_FLAGS_PSM_TRACKING   3
#define UBX_NAVPVT_FLAGS_PSM_POTRACK    4
#define UBX_NAVPVT_FLAGS_PSM_INACTIVE   5
    uint8_t     reserved1;
    uint8_t     num_sv;
    int32_t     lon;
    int32_t     lat;
    int32_t     height;
    int32_t     h_msl;
    uint32_t    h_acc;
    uint32_t    v_acc;
    int32_t     vel_N;
    int32_t     vel_E;
    int32_t     vel_D;
    int32_t     g_speed;
    int32_t     heading;
    uint32_t    s_acc;
    uint32_t    heading_acc;
    uint16_t    p_dop;
    uint16_t    reserved2;
    uint32_t    reserved3;
} ubx_nav_pvt;

#endif
