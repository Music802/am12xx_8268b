/*****************************************************************************
**
**             TEXAS INSTRUMENTS PROPRIETARY INFORMATION
**
**  (c) Copyright, Texas Instruments Incorporated, 2008
**      All Rights Reserved.
**
**  Property of Texas Instruments Incorporated. Restricted Rights -
**  Use, duplication, or disclosure is subject to restrictions set
**  forth in TI's program license agreement and associated documentation.
******************************************************************************/
#ifndef __dpp2600
#define __dpp2600

#define MAIN_STATUS               0x03
#define PBC_CONTROL               0x08
#define INPUT_SOURCE              0x0B
#define INPUT_RESOLUTION          0x0C
#define DATA_FORMAT               0x0D
#define IMG_ROTATION              0x0E
#define LONG_FLIP                 0x0F
#define SHORT_FLIP                0x10
#define TEST_PAT_SELECT           0x11
#define R_DRIVE_CURRENT           0x12
#define G_DRIVE_CURRENT           0x13
#define B_DRIVE_CURRENT           0x14
#define READ_REG_SELECT           0x15
#define RGB_DRIVER_ENABLE         0x16

#define CPU_IF_MODE               0x18
#define FRAME_RATE                0x19
#define CPU_IF_SYNC_METHOD        0x1A
#define CPU_IF_SOF                0x1B
#define CPU_IF_EOF                0x1C
#define CPU_IF_SLEEP              0x1D

#define SEQUENCE_MODE             0x1E
#define SOFT_RESET                0x1F
#define FRONT_END_RESET           0x21
#define AUTO_PWR_ENABLE           0x22

#define VSYNC_LINE_DELAY          0x23
#define CPU_PI_HORIZ_START        0x24
#define CPU_PI_VERT_START         0x25
#define CPU_PI_HORIZ_WIDTH        0x26
#define CPU_PI_VERT_HEIGHT        0x27

#define PIXEL_MASK_CROP           0x28
#define CROP_FIRST_LINE           0x29
#define CROP_LAST_LINE            0x2A
#define CROP_FIRST_PIXEL          0x2B
#define CROP_LAST_PIXEL           0x2C
#define DMD_PARK_TRIGGER		  0x2D

#define MISC_REG                  0x30

#define DMD_TYPE				  0x34

// AGC registers
#define AGC_CTRL                  0x50
#define AGC_CLIPPED_PIXS          0x55
#define AGC_BRIGHT_PIXS           0x56
#define AGC_BG_PIXS               0x57
#define AGC_SAFETY_MARGIN         0x17

// CCA registers
#define CCA_ENABLE                0x5E
#define CCA_C1A                   0x5F
#define CCA_C1B                   0x60
#define CCA_C1C                   0x61
#define CCA_C2A                   0x62
#define CCA_C2B                   0x63
#define CCA_C2C                   0x64
#define CCA_C3A                   0x65
#define CCA_C3B                   0x66
#define CCA_C3C                   0x67
#define CCA_C7A                   0x71
#define CCA_C7B                   0x72
#define CCA_C7C                   0x73

// registers for DMA operations from flash to DPP2600 LUTs
#define FLASH_ADDR_BYTES          0x74
#define FLASH_DUMMY_BYTES         0x75
#define FLASH_WRITE_BYTES         0x76
#define FLASH_READ_BYTES          0x77
#define FLASH_OPCODE              0x78
#define FLASH_START_ADDR          0x79
#define FLASH_DUMMY2              0x7A
#define FLASH_WRITE_DATA          0x7B

#define TEMPORAL_DITH_DISABLE     0x7E
#define SEQ_CONTROL               0x82
#define SEQ_VECTOR                0x83

#define SEQ_STATUS                0x8F
#define FLASH_CLOCK_CONTROL       0x98
#define DMD_PARK                  0x2D

#define SDRAM_BIST_ENABLE         0x46
#define DDR_DRIVER_STRENGTH       0x9A
#define SDC_ENABLE                0x9D
#define SDC_BUFF_SWAP_DISABLE     0xA3
#define CURTAIN_CONTROL           0xA6
#define DDR_BUS_SWAP_ENABLE       0xA7
#define DMD_TRC_ENABLE            0xA8
#define DMD_BUS_SWAP_ENABLE       0xA9

#define ACTGEN_ENABLE             0xAE
#define ACTGEN_CONTROL            0xAF
#define ACTGEN_HORIZ_BP           0xB0
#define ACTGEN_VERT_BP            0xB1

// LUT access
#define CMT_SPLASH_LUT_START_ADDR   0xFA
#define CMT_SPLASH_LUT_DEST_SELECT  0xFB
#define CMT_SPLASH_LUT_DATA         0xFC
#define SEQ_RESET_LUT_START_ADDR    0xFD
#define SEQ_RESET_LUT_DEST_SELECT   0xFE
#define SEQ_RESET_LUT_DATA          0xFF

// input source defines
#define PARALLEL_RGB              0
#define INT_TEST_PATTERN          1
#define SPLASH_SCREEN             2
#define CPU_INTF                  3
#define BT656                     4

// input resolution defines
#define QVGA_PORTRAIT             0       // (240h*320v)
#define QVGA_LANDSCAPE            1       // (320h*240v)
#define QWVGA_LANDSCAPE           3       // (427h*240v)
#define VGA_PORTRAIT_2_3          4       // (430h*640v)
#define VGA_LANDSCAPE_3_2         5       // (640h*430v)
#define VGA_PORTRAIT              6       // (480h*640v)
#define VGA_LANDSCAPE             7       // (640h*480v)//.24 VGA for 317
#define WVGA_720_PORTRAIT         8       // (480h*720v)
#define WVGA_720_LANDSCAPE        9       // (720h*480v)
#define WVGA_752_PORTRAIT         10      // (480h*752v)
#define WVGA_752_LANDSCAPE        11      // (752h*480v)
#define WVGA_800_PORTRAIT         12      // (480h*800v)
#define WVGA_800_LANDSCAPE        13      // (800h*480v)
#define WVGA_852_PORTRAIT         14      // (480h*852v)
#define WVGA_852_LANDSCAPE        15      // (852h*480v)
#define WVGA_853_PORTRAIT         16      // (480h*853v)
#define WVGA_853_LANDSCAPE        17      // (853h*480v)
#define WVGA_854_PORTRAIT         18      // (480h*854v)
#define WVGA_854_LANDSCAPE        19      // (854h*480v)//.3 WVGA for 318
#define WVGA_864_PORTRAIT         20      // (480h*864v)
#define WVGA_864_LANDSCAPE        21      // (864h*480v)
#define NTSC_LANDSCAPE            23      // (720h*240v)
#define PAL_LANDSCAPE             25      // (720h*288v)
#define nHD_Landscape			  27	  // (640h*360v)//.2 nHD
#define VGA_DMD_OPTICAL_TEST      33      // (456h*684v)
#define WVGA_DMD_OPTICAL_TEST     35      // (608h*684v)


// data format defines
#define RGB565                    0
#define RGB666                    1
#define RGB888                    2

// test pattern defines
#define TPG_CHECKERBOARD          0
#define TPG_BLACK                 1
#define TPG_WHITE                 2
#define TPG_RED                   3
#define TPG_BLUE                  4
#define TPG_GREEN                 5
#define TPG_VLINES_BLACK          6
#define TPG_HLINES_BLACK          7
#define TPG_VLINES_ALT            8
#define TPG_HLINES_ALT            9
#define TPG_DIAG_LINES            10
#define TPG_GREYRAMP_VERT         11
#define TPG_GREYRAMP_HORIZ        12
#define TPG_ANSI_CHECKERBOARD     13

// sequence mode defines
#define SEQ_FREE_RUN              0
#define SEQ_LOCK                  1

// curtain color defines
#define CURTAIN_BLACK             0
#define CURTAIN_RED               1
#define CURTAIN_GREEN             2
#define CURTAIN_YELLOW            3
#define CURTAIN_BLUE              4
#define CURTAIN_MAGENTA           5
#define CURTAIN_CYAN              6
#define CURTAIN_WHITE             7

// LUT defines
#define CMT_LUT_NONE              0
#define CMT_LUT_GREEN             1
#define CMT_LUT_RED               2
#define CMT_LUT_BLUE              3
#define CMT_LUT_ALL               4
#define SPLASH_LUT                5

#define SEQ_LUT_NONE              0
#define SEQ_DRC_LUT_0             1
#define SEQ_DRC_LUT_1             2
#define SEQ_DRC_LUT_2             3
#define SEQ_DRC_LUT_3             4
#define SEQ_SEQ_LUT               5
#define SEQ_DRC_LUT_ALL           6
#define WPC_PROGRAM_LUT           7

#define ICP_COMPOUND_I2C_STATUS                 0xc4
#define ICP_COMPOUND_I2C_READ_PAD1000_REG_VALUE 0xd1

#define DMA_STATUS                BIT8
#define TIMER_VALUE 			  65535   // 1 MHz system clk so timer value is 65 msec ( 65535/1000000)

#endif /* #ifndef __dpp2600 */

