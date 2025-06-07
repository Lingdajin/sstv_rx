/*!
 * \file
 */
#ifndef __IRIS404_H__
#define __IRIS404_H__


#define IRIS404_REGISTER_NUMBER               (64)	//!< IRIS404芯片可用寄存器个数
typedef struct st_iris404mac_row {
    uint32_t                ram_id;                   //!< ram_id 编号
    uint32_t                ucValue;                    //!< default value
    uint8_t                 ucType;                  //0 ->FPGA, 1 ->RegWrite, 2 ->MacWrite, 3->MacMacro, 4 ->MacStart;
} ST_IRIS404MAC_ROW;

typedef struct iris404init
{
	uint16_t id;
	uint32_t init_data;

}IRIS404INIT;

typedef enum
{
  Busy      = 1 << 0,
  LtchRFnE  = 1 << 1,
  LtchRFF   = 1 << 2,
  LtchCFE   = 1 << 3,
  RFEEmpty  = 1 << 4,
  EFFull    = 1 << 5,
  CFEmpty   = 1 << 6,
  CFFull    = 1 << 7,
}SPIx_STATUS_BIT;

/* idle2fdidle config */
ST_IRIS404MAC_ROW seq_idle2fdidle[6] = {
	{0x000,0x36000002,0x10},
	{0x001,0x36c9c000,0x11},
	{0x002,0x36ddc000,0x11},
	{0x003,0x5d808008,0x01},
	{0x004,0x5b26f440,0x01},
	{0x005,0x3a094003,0x01},
};

#define   INIT_NUM    (2465)       //初始化数据处理行数
#define TXPAC_NUM    (685)
#define RXPAC_NUM    (96)
uint32_t    reg_read(uint32_t reg_add);
uint32_t    reg_write(uint32_t reg_add, uint32_t reg_data);

/*! \brief AD9361芯片的寄存器结构体 */
typedef struct st_iris404reg {
    uint8_t                 uiAddr;                         //!< 寄存器地址
    uint8_t                 uiDefault;                      //!< 默认初始化值
    uint8_t                 ucType;                         //!< 寄存器类型，1~3有效。1-R; 2-W; 3-R&W; other-undefined;
    uint32_t                ucValue;                        //!< 寄存器值

} ST_IRIS404REG;
#define FAFACTOR                    (33554432)                   //!< 芯片分频小数部分因子


#define STAT1_BASE 0x80000F00                               //!< 状态寄存器地址

#define XSPI_BASE  0x8FC04000                               //!< SPI接口寄存器基地址

#define NODE_1 1
#define NODE_2 2
#define NODE_3 3
/*
 * SPI_clk = (HSN_clk/((n+1)*2)), or n = (HSN_clk/(2*SPI_clk)) - 1
 * HSN_clk = 275MHz n=6，then 275MHz/((6+1)*2) = 19.64MHz
 */
// #define SPI_DIV 6
#define SPI_DIV     (24)

#define WRITE_STAT1(__off, __val) \
    (*((volatile unsigned*)(STAT1_BASE | __off)) = (__val))

#define XSPI_WRITE(__node, __off, __val)  \
             MMIO_WRITE32( (XSPI_BASE | (__node<<28)), (__off), (__val) )
#define XSPI_READ(__node, __off)   \
             MMIO_READ32( (XSPI_BASE | (__node<<28)), (__off))

#define wait_ms(__val)                       osFastPause((__val)*135000)  //!< 等待大约1ms

#define FREQ                        (30720000)                  //!< 板载主晶振频率

#define TX_ATTEN_MAX                (359)                       //!< 发射增益最大设置值

#define TDD_MODE					(0)
#define FDD_MODE					(1)

#define _ON_						(1)
#define _OFF_						(0)

uint32_t    spi_read(uint32_t reg_addr);
void        spi_write(uint32_t reg_addr, uint32_t reg_data);

#endif // __IRIS404_H__

// *
// EOF
//
