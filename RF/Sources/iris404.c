/*!
 * \file	ad9361.c
 * \detail 用于ADI公司ad9361的驱动程序
 * \author 田朝阳<zytian@gmail.com>
 * \date   2016-09-26
 */
#include <device/sbdspsoc_sb3500.h>
#include <device/sbdc_sb3500.h>
#include <device/sbarmsoc_sb3500.h>
#include <device/mmioutil.h>
#include "iris404.h"
#include <stdlib.h>
#include <stdint.h>

// should include when the RF interface ready
#include "iris404state.inc"
#include "iris404reg.inc"
//#define TDD_MODE					(0)
//#define FDD_MODE					(1)

#include "rf_drv.h"

uint32_t ret_data[40]={0};       //!<用来保存连续读返回的data值；


#define RX_GAIN_NUM      (96)
//extern uint32_t xspi_read_data_register(uint32_t sbx_node, uint32_t offset, uint32_t data, uint32_t tx_count, uint32_t rx_count);

ST_RF_DRV *pstChipDrv;   //!<

/*! \brief 锁定驱动 */
//#define SPIN_LOCK()			{ while((pstChipDrv->spLock)!=0) osFastPause(10); (pstChipDrv->spLock) = 1;}
/*! \brief 解锁驱动 */
//#define SPIN_UNLOCK() 		(pstChipDrv->spLock) = 0

/*! \brief 配置芯片接口 */
void iris404_config(void)
{
	SPIN_LOCK();

    /*
     * Set PSDA for PSD3 & PSD4
     *   PSD3 <= Node3 PSDA
     *   PSD4(15:8) <= Node3 GPIO; PSD4(7:0) <= Node2 GPIO
     */
    MMIO_WRITE32(SBARMSOC_MMIO_SBDC, SBDC_PSD_SHARE, 0x0F);
    //配置spi接口为3线模式
	//XSPI_WRITE(NODE_2, SBX_SPI_CONFIG, 0x800);
	wait_ms(20);
    // 初始化spi3接口
    /* set the SPI frequency Prescaler value = SPI_DIV 19.64MHz */
    XSPI_WRITE(NODE_2, SBX_SPI_PRESCALE, SPI_DIV);

    wait_ms(20);

	XSPI_WRITE(NODE_2, SBX_SPI_CONFIG, 0x800);
    // 初始化sbx_gpio3
    /*
     * Set all Node3 GPIO bits as output,
     * enable the power to the XCVR LNA & configure the switches
     *  Bit 7:PSD_4_15:O: RX2_SPDT_V2
     *  Bit 6:PSD_4_14:O: RX2_SPDT_V1
     *  Bit 5:PSD_4_13:O: RX1_SPDT_V2
     *  Bit 4:PSD_4_12:O: RX1_SPDT_V1
     *  Bit 3:PSD_4_11:O: RX_RF_PWR_EN
     *  Bit 2:PSD_4_10:O: TX_RF_PWR_EN
     *  Bit 1:PSD_4_9 :O: TX_SPDT_V2
     *  Bit 0:PSD_4_8 :O: TX_SPDT_V1
     */
    SBX_GPIO_DIR_WRITE(SBX_GPIO_BASE(NODE_3), 0xFF, 0xFF);


//    SBX_GPIO_DATA_WRITE(SBX_GPIO_BASE(NODE_3), 0xFC, 0x5A); // B01011010 - ifdef USE_HI_BAND == 1
//ifndef USE_HI_BAND == 1
	SBX_GPIO_DATA_WRITE(SBX_GPIO_BASE(NODE_3), 0xFD, 0x0D); // B00011100 - ifndef HALF_FDD
//    SBX_GPIO_DATA_WRITE(SBX_GPIO_BASE(NODE_3), 0x54, 0x54); // 开机Rx，打开 DCDC_APT - ifdef HALF_FDD

    // 初始化sbx_gpio1
    /*
     * Keep the XCVR in Reset
     *  Node1 GPIO
     *  Bit 7:NGPIO_1_7:I: GPO_3 (input: configurable at XCVR, unused for now)
     *  Bit 6:NGPIO_1_6:I: AGCSTRB (input: interrupt to BB IC, unused for now)
     *  Bit 5:NGPIO_1_5:O: TX_FRM_EN (output: strobed enable)
     *  Bit 4:NGPIO_1_4:O: XCVR_RESETN (output: reset when low)
     *  Bit 3:NGPIO_1_3:I: Node_Int (input: from DPMU
     *  Bit 2:NGPIO_1_2:O: TXNRX (output: TX when high; RX when Low)
     *  Bit 1:NGPIO_1_1:I: RSSI_DIG (input: RSSI after channel selection)
     *  Bit 0:NGPIO_1_0:I: RSSI_AN (input: RSSI after down conversion)
     */
//#ifndef HALF_FDD
	SBX_GPIO_DIR_WRITE(SBX_GPIO_BASE(NODE_1), 0xFF, 0x30);
	SBX_GPIO_DATA_WRITE(SBX_GPIO_BASE(NODE_1), 0x30, 0x00);
//#else
//    SBX_GPIO_DIR_WRITE(SBX_GPIO_BASE(NODE_1), 0xFF, 0xFF);
//    SBX_GPIO_DATA_WRITE(SBX_GPIO_BASE(NODE_1), 0x10, 0x00);
//#endif
    wait_ms(2);
    // set XCVR_RESETN=1 after reset.
    SBX_GPIO_DATA_WRITE(SBX_GPIO_BASE(NODE_1), 0x10, 0x10);
    wait_ms(10);
    wait_ms(500);
    wait_ms(500);

	SPIN_UNLOCK();

}

/* Node SPI functions */
// wait for the SPI SM to be busy - started!
//#define spi_wait_until_busy(_node)      \
//   while ((XSPI_READ((_node), SBX_SPI_STATUS) & 0x01) == 0)
//
//// flush the READ FIFO until empty and the SPI SM is idle
//#define spi_flush_read_fifo(_node)      \
//  while ((XSPI_READ((_node), SBX_SPI_STATUS) & 0x10) != 0x10) { XSPI_READ((_node), SBX_SPI_READ);  }
//
//// wait for the CMD FIFO to be empty and an idle SPI SM
//#define spi_wait_until_idle(_node)      \
//  while ((XSPI_READ((_node), SBX_SPI_STATUS) & 0x41) != 0x40)
//
//// wait for Data to be available in the READ FIFO
//#define spi_wait_for_rdat(_node)        \
//    while ((XSPI_READ((_node), SBX_SPI_STATUS) & 0x11) != 0)

uint32_t spi_read_fifo(uint32_t sbx_node)
{
    while ((XSPI_READ(sbx_node, SBX_SPI_STATUS) & 0x10) != 0); {}
    return XSPI_READ(sbx_node, SBX_SPI_READ);
}

void xspi_write_data(uint32_t sbx_node,
                     uint32_t addr,
                     uint32_t data,
                     uint32_t length)
{
    XSPI_WRITE(sbx_node, SBX_SPI_CTL, length);
    XSPI_WRITE(sbx_node, addr, (data << (32 - length)) );

    spi_wait_until_busy(sbx_node);
    spi_wait_until_idle(sbx_node);
}

void xspi_wrrd_data(uint32_t sbx_node,
                    uint32_t addr,
                    uint32_t data,
                    uint32_t rlength,
                    uint32_t wlength)
{
    XSPI_WRITE(sbx_node, SBX_SPI_CTL, (0x1000 | ((rlength & 0x3F) << 6) | (wlength & 0x3F)));
    XSPI_WRITE(sbx_node, addr, (data << (32 - wlength)) );

    spi_wait_until_busy(sbx_node);
    spi_wait_until_idle(sbx_node);
}
/* ***************************************** */



uint32_t xspi_config_mac(uint32_t sbx_node, uint32_t offset, uint32_t data, uint32_t rx_count, uint32_t tx_count)
{
  uint32_t spidata;
  if(sbx_node > 3)
     {
      printf("the sbx_node error\n");
	  return EXIT_FAILURE;
	 }

  if(rx_count == 0)
     spidata = tx_count;
  else
     spidata = 0x1000 | ((rx_count & 0x3F) << 6) | (tx_count & 0x3F);
  //配置控制寄存器；
  XSPI_WRITE(sbx_node, SBX_SPI_CTL, spidata);
  //配置片选寄存器
  XSPI_WRITE(sbx_node, offset, data);

  spi_wait_until_busy(sbx_node);
  spi_wait_until_idle(sbx_node);

  return EXIT_SUCCESS;
}

/* ***************************************** */



/*********************************************************************************************************************************
*函数原型：uint32_t xspi_read_data_register(uint32_t sbx_node, uint32_t offset, uint32_t data, uint32_t tx_count, uint32_t rx_count
*输入参数：sbx_node SPI对应DSP核端口号(1~3)  offset 寄存器偏移地址  data 写入寄存器值  tx_count写入值长度  rx_count要读取的值长度
*输出参数：EXIT_SUCCESS  程序返回成功          EXIT_FAILURE  程序返回错误
*函数功能：读操作配置控制寄存器并向只写寄存器写入值
*********************************************************************************************************************************/
uint32_t xspi_read_data_register(uint32_t sbx_node,
                                 uint32_t offset,
							     uint32_t data,
							     uint32_t tx_count,
							     uint32_t rx_count)
{
  if(sbx_node>3)
   {
     printf("the sbx_node error\n");
	 return EXIT_FAILURE;
   }
  //配置控制寄存器TX_COUNT,RX_COUNT;
  XSPI_WRITE(sbx_node, SBX_SPI_CTL, 0x1000 | ((rx_count & 0x3F) << 6) | (tx_count & 0x3F));

  //向指定片选寄存器写值
  XSPI_WRITE(sbx_node, offset, data << (32-tx_count));

  spi_wait_until_busy(sbx_node);
  spi_wait_until_idle(sbx_node);

  return EXIT_SUCCESS;
}

/***************************************************************************************************************
*函数原型：uint32_t xspi_write_data_register(uint32_t sbx_node, uint32_t offset, uint32_t data, uint32_t length)
*输入参数：sbx_node SPI对应DSP核端口号(1~3)   offset 寄存器偏移地址   data 写入寄存器值  length写入值长度
*输出参数：EXIT_SUCCESS  程序返回成功          EXIT_FAILURE  程序返回错误
*函数功能：写操作配置控制寄存器并向只写寄存器写入值
***************************************************************************************************************/
uint32_t xspi_write_data_register(uint32_t sbx_node,
                                  uint32_t offset,
								  uint32_t data,
								  uint32_t length)
{
  if(sbx_node>3)
    {
	 printf("the sbx_node error\n");
	 return EXIT_FAILURE;
	}
  //配置控制寄存器的TX_COUNT
  //printf("config ctl_register......\n");
  XSPI_WRITE(sbx_node, SBX_SPI_CTRL, length);
  //配置只写存储器
  XSPI_WRITE(sbx_node, offset,(data<<(32-length)));
  //MMIO_WRITE32(sbx_node, offset, (data<<(32-length)));

  spi_wait_until_busy(sbx_node);
  spi_wait_until_idle(sbx_node);
  return EXIT_SUCCESS;
}
/******************************************************************************
*函数原型： uint32_t reg_write(uint32_t reg_add, uint32_t reg_data)
*输入参数： reg_add 寄存器地址     reg_data 写入寄存器值
*输出参数：	EXIT_SUCCESS  程序返回成功          EXIT_FAILURE  程序返回错误
*函数功能： 对DSP核SPI端口进行写寄存器操作
******************************************************************************/
uint32_t reg_write(uint32_t reg_add, uint32_t reg_data)
{
  uint32_t spidata, ret;

  spidata = 0 | ((reg_add & 0x3F)<<25) | (reg_data & 0x1FFFFFF);
 // printf("come into reg_write......\n");
  ret = xspi_write_data_register(NODE_2, SBX_SPI_CSA, spidata, 32);
  if(ret != 0)
  {
	 printf("reg_write config failed\n");
	 return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}


/*****************************************************************************
*函数原型： uint32_t reg_read(uint32_t reg_add)
*输入参数： reg_add 寄存器地址
*输出参数：	spidata寄存器值 程序返回成功          EXIT_FAILURE  程序返回错误
*函数功能： 对DSP核SPI端口进行读寄存器操作
******************************************************************************/
uint32_t reg_read(uint32_t reg_add)
{

  uint32_t spidata, ret;

  spidata = 0x40 | (reg_add & 0x3F);
  //printf("come into reg_read......\n");
  //配置控制寄存器并向片选寄存器A写值
  ret = xspi_read_data_register(NODE_2, SBX_SPI_CSA, spidata, 7, 25);

  if(ret !=0)
  {
	  printf("reg_read config faild\n");
	  return EXIT_FAILURE;
  }
  spidata = spi_read_fifo(NODE_2);
  spidata &= 0x1FFFFFF;

  spi_flush_read_fifo(NODE_2);

  return spidata;
}

/********************************************************************************************************
*函数原型：uint32_t xspi_write_data_ram(uint32_t sbx_node, uint32_t offset, uint32_t data, uint32_t length)
*输入参数：sbx_node SPI对应DSP核端口号(1~3)   offset 寄存器偏移地址   data 写入寄存器值  length写入值长度
*输出参数：EXIT_SUCCESS  程序返回成功          EXIT_FAILURE  程序返回错误
*函数功能：写操作配置控制寄存器并向只写寄存器写入值
*********************************************************************************************************/
uint32_t xspi_write_data_ram(uint32_t sbx_node,
                             uint32_t offset,
						     uint32_t ram_num,
						     uint32_t ram_row)
{

  uint32_t spidata, spicomd;
  int i, ram_start;
  ram_start = ram_num;
  if(sbx_node>3)
    {
	 printf("the sbx_node error\n");
	 return EXIT_FAILURE;
	}
  //配置控制寄存器的TX_COUNT
  spicomd = 0 | 0x1B << 25 | (((ram_num & 0x7FF) << 12) | (ram_row & 0x7FF)) << 1;
  //以ram 的方式写一次控制质量
  xspi_config_mac(sbx_node, offset, spicomd, 0, 32);

  for(i=0; i<ram_row+1; i++)                   //此处如果用for循环发送数据的话每一条流之间可能有比较大的延迟;
     {
        spidata = dyconfig[ram_start].ucValue ;
   	    if(i==ram_row)
   	        spidata = 0x7FFFFFFF & spidata;    //如果是最后一行的spidata则MSB位应该位1；
   	    else
            spidata = 0x80000000 | spidata;
   	   xspi_config_mac(sbx_node, offset, spidata, 0, 32);
       ram_start++;
     }

  return EXIT_SUCCESS;
}

/******************************************************************************
*函数原型： uint32_t RAM_WRITE(uint32_t reg_add, uint32_t reg_data)
*输入参数： reg_add 寄存器地址     reg_data 写入寄存器值
*输出参数：	EXIT_SUCCESS  程序返回成功          EXIT_FAILURE  程序返回错误
*函数功能： 对DSP核SPI端口进行写寄存器操作
******************************************************************************/
uint32_t ram_write(uint32_t ram_num, uint32_t ram_row)
{

  uint32_t ret;

  ret = xspi_write_data_ram(NODE_2, SBX_SPI_CSA, ram_num, ram_row);
  if(ret != 0)
  {
	 printf("REG_WRITE config failed\n");
	 return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

/*********************************************************************************************************************************
*函数原型：uint32_t xspi_read_data_register(uint32_t sbx_node, uint32_t offset, uint32_t data, uint32_t tx_count, uint32_t rx_count
*输入参数：sbx_node SPI对应DSP核端口号(1~3)  offset 寄存器偏移地址  data 写入寄存器值  tx_count写入值长度  rx_count要读取的值长度
*输出参数：EXIT_SUCCESS  程序返回成功          EXIT_FAILURE  程序返回错误
*函数功能：读操作配置控制寄存器并向只写寄存器写入值
*********************************************************************************************************************************/
uint32_t xspi_read_data_ram(uint32_t sbx_node,
                            uint32_t offset,
					        uint32_t data,
						    uint32_t tx_count,
						    uint32_t rx_count)
{
  if(sbx_node>3)
   {
     printf("the sbx_node error");
	 return EXIT_FAILURE;
   }
  //配置控制寄存器TX_COUNT,RX_COUNT;
   XSPI_WRITE(sbx_node, SBX_SPI_CTL, 0x1000 | ((rx_count & 0x3F) << 6) | (tx_count & 0x3F));

  //向指定片选寄存器写值
   XSPI_WRITE(sbx_node, offset, data);

   spi_wait_until_busy(sbx_node);
   spi_wait_until_idle(sbx_node);

   ret_data[0] = spi_read_fifo(sbx_node);
   spi_flush_read_fifo(sbx_node);

   return EXIT_SUCCESS;
}

inline uint32_t ram_read(uint32_t ram_id, uint32_t ram_row)
{
  uint32_t spidata, ret, spicomd;
  int i;
  spicomd = 1<<31 | 0x1B << 25 | (((ram_id & 0x7FF) << 11) | (ram_row & 0x7FF)) << 1;
  printf("spicomd %x \n",spicomd);
  //spicomd = 0x00|(27 <<25)|((ramstart_id & 0x7FF)<<12)|((row & 0x7FF)<1);
  //配置控制寄存器并向片选寄存器A写值

   ret = xspi_read_data_ram(NODE_2, SBX_SPI_CRCSA, spicomd, 32, 32);

   if(ret !=0)
   {
 	  printf("reg_read config faild\n");
 	  return EXIT_FAILURE;
   }

   for(i=0; i<ram_row; i++)              //此处如果用for循环发送数据的话每一条流之间可能有比较大的延迟;
     {
        spidata = dyconfig[ram_id].ucValue ;
        //printf("the spidata defaule value is [%x] ...\n",spidata);

        xspi_config_mac(NODE_2, SBX_SPI_CSA, spidata, 32, 0);
   	    //MAC_MACRO(HLCMD,(0<<31)|(27<<25)|(1<<23)|(SETMODEMACRO<<12));
        ret_data[i+1] = spi_read_fifo(NODE_2);
        spi_flush_read_fifo(NODE_2);
     }
  return EXIT_SUCCESS;
}



uint32_t xspi_write_data_mac(uint32_t sbx_node,
                             uint32_t offset,
						     uint32_t ram_num,
						     uint32_t ram_vaule)
{
  uint32_t spicomd;
  spicomd = 0 | 0x1B << 25 | ((ram_num & 0x7FF) << 12) ;
  if(sbx_node>3)
   {
     printf("the sbx_node error");
	 return EXIT_FAILURE;
   }
   xspi_config_mac(sbx_node, offset, spicomd, 0, 32);
   wait_ms(1);
   xspi_config_mac(sbx_node, offset, ram_vaule, 0, 32);

   return EXIT_SUCCESS;
}


uint32_t mac_write(uint32_t ram_num, uint32_t ram_vaule)
{
  uint32_t ret;
  ret = xspi_write_data_mac(NODE_2, SBX_SPI_CSA, ram_num, ram_vaule);
  if(ret != 0)
  {
	 printf("REG_WRITE config failed\n");
	 return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}


uint32_t mac_start(uint16_t ram_id, uint16_t timer, uint8_t flag)
{
  uint32_t spidata, ret;

  spidata = 0 | (0x1B<<25) | (0x01<<23) | ((ram_id & 0x7FF)<<12) | ((timer & 0x7FF)<<1) | (flag & 1);
//  printf("come into mac_start spidata is [%x]......\n",spidata);
  ret = xspi_write_data_register(NODE_2, SBX_SPI_CSA, spidata, 32);
  if(ret != 0)
  {
	 printf("reg_write config failed\n");
	 return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

//初始化芯片寄存器，通过读取数组config_data[i].init_data中的值，并将其一条条发出去
uint32_t iris404_init(void)
{
	uint32_t i, ret, spidata;
//	printf("init_config start \n");

	for(i=0;i<INIT_NUM;i++)
	{
		spidata = config_data[i].init_data;
		ret = xspi_write_data_register(NODE_2, SBX_SPI_CSA, spidata, 32);
		wait_ms(1);
		if(ret !=0)
		{
		    printf("init_config failed [%d]\n",i);
	        return EXIT_FAILURE;
		}
	}
	//printf("init_config done \n");
	return EXIT_SUCCESS;
}

// ! \brief 控制发射开启关闭
void iris404_txctrl(int16_t onoff)
{
	if(onoff==_ON_)
	{
//#ifndef HALF_FDD
		SBX_GPIO_DATA_WRITE( SBX_GPIO_BASE(NODE_1), 0x20, 0x20);
//#else
//    	SBX_GPIO_DATA_WRITE( SBX_GPIO_BASE(NODE_3), 0x11, 0x01);
//#endif
		if (pstChipDrv->siMode==FDD_MODE)
		{
			/* do nothing */
		}
		else
		{

		}
	}
	else
	{
		SBX_GPIO_DATA_WRITE( SBX_GPIO_BASE(NODE_3), 0x11, 0x10);
		if (pstChipDrv->siMode==FDD_MODE)
		{
			/* do nothing */
			printf("pstchipdrv->simode == FDD_MODE\n");
		}
		else
		{
    		printf("pstchipdrv->simode == TDD_MODE\n");
		}

	}

}
inline void get_ngpio (uint16_t node, uint16_t *dir, uint16_t *val)
{
    uintptr_t ngpioX;
    uint32_t  spidata;
    if( (node!=1)&&(node!=3) ) return ;
    ngpioX = SBX_GPIO_BASE(node);
    spidata = SBX_GPIO_DIR_READ(ngpioX, 0xFF);
    *dir = (uint16_t)(spidata & 0xFFFF);
    spidata = SBX_GPIO_DATA_READ(ngpioX, 0xFF);
    *val = (uint16_t)(spidata & 0xFFFF);
}

inline void set_ngpio(uint16_t node, uint16_t mask, uint16_t  val)
{
    uint32_t  spidata;
    uint32_t  spimask;
    if( (node!=1)&&(node!=3) ) return ;
    spimask = mask;
    spidata = SBX_GPIO_DIR_READ( SBX_GPIO_BASE(node), 0xFF);
    spimask = (spidata & 0xFF) & spimask;
    SBX_GPIO_DIR_WRITE( SBX_GPIO_BASE(node), 0xFF, spidata);
    spidata = spidata & val;
    SBX_GPIO_DATA_WRITE( SBX_GPIO_BASE(node), spimask, spidata );
}

/*! \brief 读出控制脚状态 */
void iris404_getgpio(uint16_t *dir, uint16_t *val)
{
	uint16_t dir1,dir3,val1,val3;
	get_ngpio(1, &dir1, &val1);
	get_ngpio(3, &dir3, &val3);
	*dir = ((dir1 << 8) & 0xFF00) | (dir3 & 0xFF);
	*val = ((val1 << 8) & 0xFF00) | (val3 & 0xFF);
}

/*! \brief 设置控制脚状态值 */
void iris404_setgpio(uint16_t mask, uint16_t val)
{
	uint16_t mask1, mask3, val1, val3;
	mask1 = (mask >> 8) & 0xFF;
	val1  = (val >> 8) & 0xFF;
	mask3 = mask & 0xFF;
 	val3  = val & 0xFF;
	set_ngpio(1, mask1, val1);
	set_ngpio(3, mask3, val3);
}

void cal_freq(uint32_t inter_part, uint32_t fra_part, double *freq)
{
    uint32_t nr, nf, reg;
    double pllfreq;

    reg                     = reg_read(inter_part);
    nr                      = reg>>16;
    nf                      = reg_read(fra_part);
    //printf("the nf is [%d]\n",nf);
    pllfreq                 = (double)nf/FAFACTOR;
    //printf("the pllfreq1 is [%.10f]\n",pllfreq);
    pllfreq                 = (nr+pllfreq)*26;
    //printf("the pllfreq2 is [%.10f]\n",pllfreq);
//	#if USE_HI_BAND == 1
//     *freq                  = pllfreq/2;
//	#else
     *freq                  = pllfreq/4;
//	#endif
}

/*! \brief 读取接收发射的频率设置 */
void iris404_getfreq (double *txf, double *rxf)
{
    uint32_t nr, nf;
    nr=0x04; nf=0x05;
    cal_freq(nr, nf, txf);

    nr=0x0E; nf=0x0F;
    cal_freq(nr, nf, rxf);
}


void set_cal_freq(double freq, uint32_t inter_part, uint32_t fra_part)
{
    uint32_t inter_num;
    double pllfreq, fra_num;

	#if USE_HI_BAND == 1
     pllfreq                = freq*2/26;
	#else
     pllfreq                = freq*4/26;
	#endif

    inter_num               = pllfreq;
    fra_num                 = (pllfreq-inter_num)*FAFACTOR;

    //fra_num               =(uint32_t)(pllfreq*1000000)%1000000;
	//printf("fra_num is %.10f\n",fra_num);
	//printf("inter_num is %d\n",inter_num);
	//printf("pllfreq is %.10f\n",pllfreq);

	inter_num               = inter_num<<16;
	reg_write(fra_part,fra_num);
	reg_write(inter_part,inter_num);
}

/*! \brief 设置接收发射的频率设置 */
void iris404_setfreq(uint32_t rxf, uint32_t txf)
{
    uint32_t nr,nf;
    double tx_freq,rx_freq;
    tx_freq = (double)txf/1000000;
    rx_freq = (double)rxf/1000000;
    nr = 0x04; nf = 0x05;
    set_cal_freq(tx_freq,nr,nf);
    nr = 0x0E; nf = 0x0F;
    set_cal_freq(rx_freq,nr,nf);

}

/*读取tx pgc table 功率值*/
uint8_t read_txpgctable(uint32_t reg_data, int16_t *dBm)
{
	int16_t i=0;
	for(i=0;i<TXPAC_NUM;i++)
	{
		if(reg_data == pgcTable_lo[i]){
			i=i>>3;
			*dBm = 26-i;
			//printf("read_txpgctable123 is %d \n",i);
			break;
		}
	}
	if(i==TXPAC_NUM)
	{
		printf("not find txpgx data...\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


/*! \brief 读取发射衰减设置 */
void iris404_gettxatten(uint8_t ch, int16_t *atten)
{
    uint32_t reg;
    uint8_t ret;


    if( ch==0 )
    {
    	reg                           = reg_read(0x01);
        ret	                          = read_txpgctable(reg, atten);
    	if(ret != 0)
    		return;
    }
    else if(ch==1)
    {
    	reg                           = reg_read(0x01);
    	ret                           = read_txpgctable(reg, atten);
    	if(ret != 0)
    		return;
    }
    else
    {
        printf("chanel number error ,range is 0~~1.\n");
    }
}

uint8_t write_txpgctable(uint32_t *reg_data, int16_t dBm)
{
	int16_t index;

	index = (26-dBm)<<3;
	if(index>=TXPAC_NUM)
	{
		printf("read txpactable failed \n");
		return EXIT_FAILURE;
	}
	*reg_data = pgcTable_lo[index];

	return EXIT_SUCCESS;
}

/*! \brief 设置发射衰减值 */
void iris404_settxatten(uint8_t ch, int16_t atten)
{
    uint32_t reg;
    int16_t siInd;
    if( ch==0 )
    {
		siInd = (atten>=(pstChipDrv->siTx0AttenMax))?(pstChipDrv->siTx0AttenMax):		\
			((atten<=pstChipDrv->siTx0AttenMin)?(pstChipDrv->siTx0AttenMin):atten);
    	write_txpgctable(&reg, siInd);
    	reg_write(0x01, reg);

    }
    else if( ch==1 )
    {
		siInd = (atten>=(pstChipDrv->siTx1AttenMax))?(pstChipDrv->siTx1AttenMax):		\
			((atten<=pstChipDrv->siTx1AttenMin)?(pstChipDrv->siTx1AttenMin):atten);
    	write_txpgctable(&reg, siInd);
    	reg_write(0x01, reg);

    }else{
    	printf("chanel num is wrong, range is 0~~1.\n");
    }

}

/*! \brief 设置频偏纠正值 */
void iris404_adjustfreqoffset(int16_t offset)
{
	/* Sorry, I don't know how to do. */
	//int32_t dacword, caldeltaf;
	///double txf, rxf;
	uint32_t calfreq, reg_data_h, reg_data_l, freq;
	//int64_t  temp;
	//uint32_t dir ;
	//dir = (offset>0)?0:1;
	//reg = reg_read( 0x01A );
	//dacword = (reg & 0x3) + reg_read( 0x018 ) << 2;
	//printf("\nDAC=0x%04X", dacword);

	///iris404_getfreq(&txf, &rxf);
	reg_data_h  = reg_read(0x19)>>10;
	reg_data_l  = reg_read(0x19)&0x3FF;
	freq        = offset/1;
	reg_data_h += freq;
	calfreq     = reg_data_h<<10|reg_data_l;

	reg_write(0x19,calfreq);
	//calfreq   = (pstChipDrv->uiCalFreq) >> 20;
	//caldeltaf = pstChipDrv->siCalDeltaF;
	//temp = 1;
	//temp = temp * offset*calfreq;
	//dacword = (temp * 0x3FB)/(freq*caldeltaf) + dacword;
	// printf("\ncalfreq=%u caldeltaf=%d freq=%u; offset=%d", (calfreq<<20), caldeltaf, (freq<<20), offset);

	//dacword = (dacword<0x200)?0x200:(dacword>0x3E0)?0x3E0:dacword;
	//printf("set DAC reg 25 [%x]\n", calfreq);
	//printf(", after write DAC=0x%04X\n", dacword);
}
uint32_t iris404_getVolt(void)
{
	uint32_t reg_data_h;

	reg_data_h  = reg_read(0x19)>>10;

	return reg_data_h;
}

void iris404_setVolt(uint32_t voltage)
{
	uint32_t calfreq, reg_data_h, reg_data_l;

	reg_data_h  = voltage;
	reg_data_l  = reg_read(0x19)&0x3FF;
	calfreq     = reg_data_h<<10|reg_data_l;
	reg_write(0x19,calfreq);

}

/*! \brief 设置工作模式 */
void iris404_setmode(int16_t mode)
{
	pstChipDrv->siMode = (mode==FDD_MODE)?FDD_MODE:TDD_MODE;
}

/*! \brief 芯片软件复位 */
void iris404_reset(void)
{
	SPIN_LOCK();
    // set XCVR_RESETN=1 after reset.
    SBX_GPIO_DATA_WRITE(SBX_GPIO_BASE(NODE_1), 0x10, 0x10);
    wait_ms(10);
    wait_ms(500);
    wait_ms(500);
	SPIN_UNLOCK();
}

/*! \brief 读取接收增益索引值 */
void iris404_getrxgain(uint8_t ch, int16_t *ind)
{
	uint32_t siInd = 0;
	int16_t i = 0;
	if(ch==0)
	{
		siInd 								= reg_read( 0x09 );
		for(i=0;i<RX_GAIN_NUM;i++)
		{
			if(siInd == rxGainTable_4G_FDD[i])
			{
				*ind = i+2;
				break;
			}
		}
		if(i==RX_GAIN_NUM)
			printf("getrxgain not find data\n");
	}
	else if(ch==1)
	{
		siInd 								= reg_read( 0x09 );
		for(i=0;i<RX_GAIN_NUM;i++)
		{
			if(siInd == rxGainTable_4G_FDD[i])
			{
				*ind = i+2;
				break;
			}
		}
		if(i==RX_GAIN_NUM)
			printf("getrxgain not find data\n");
	}
	else
	{
		printf("iris404_getrxgain range is wrong, range is 0~~1.\n");
	}
}

/*! \brief 设置接收增益索引 */
void iris404_setrxgain(uint8_t ch, int16_t index)
{
	int16_t siInd;
	uint32_t rxg;
	siInd = index;
	if(ch==0)
	{
		siInd = (index>=(pstChipDrv->siRx0GainMax))?(pstChipDrv->siRx0GainMax):		\
			((index<=pstChipDrv->siRx0GainMin)?(pstChipDrv->siRx0GainMin):index);
		if(siInd<2||siInd>97)
			printf("rx gain rang is 2---97 dB\n");
		rxg = rxGainTable_4G_FDD[siInd-2];
		//printf("rx gain is [%x] \n",rxg);
		reg_write(0x09,	rxg);
	}
	else if(ch==1)
	{
		siInd = (index>=(pstChipDrv->siRx1GainMax))?(pstChipDrv->siRx1GainMax):	\
			((index<=pstChipDrv->siRx1GainMin)?(pstChipDrv->siRx1GainMin):index);

		if(siInd<2||siInd>97)
			printf("rx gain rang is 2---97 dB\n");
		rxg = rxGainTable_4G_FDD[siInd-2];
		//printf("rx gain is [%x] \n",rxg);
		reg_write(0x09,	rxg);
	}
	else
	{
		printf("iris404_setrxgain chanel is wrong, range is 0~~1. \n");
	}
}

/*! \brief 读取接收增益 */
void iris404_getrxdb(uint8_t ch, int16_t *db)
{
	int16_t ind;
	//int16_t delta;
	//delta								= 0;
	iris404_getrxgain(ch, &ind);
	if(ch==0)
	{
		//delta 							= pstChipDrv->siRxDb0_local;
	}
	else if(ch==1)
	{
		//delta 							= pstChipDrv->siRxDb1_local;
	}
	else
	{
		printf("iris404_getrxdb chanel is wrong, range is 0~~1. \n");
	}
	//*db 								= ind + delta;
	*db 								= ind;
}
/*! \brief 设置接收增益 */
void iris404_setrxdb(uint8_t ch, int16_t db)
{
	int16_t ind;
	//int16_t delta;
	//delta 								= 0;
	iris404_getrxgain(ch, &ind);

	if(ch==0)
	{
		//delta 							= pstChipDrv->siRxDb0_local;
	}
	else if(ch==1)
	{
		//delta 							= pstChipDrv->siRxDb1_local;
	}
	else
	{
	}
	//ind  								= db + delta;
	ind  								= db + ind;
	iris404_setrxgain(ch, ind);
}

/*! \brief 读取发射功率 */
void iris404_gettxdbm(uint8_t ch, int16_t *dbm)
{
	int16_t atten;
	iris404_gettxatten(ch, &atten);
	//*dbm 								= pstChipDrv->siTxDbm_local - atten;
	*dbm 								= atten;
}

/*! \brief 设置发射功率 */
void iris404_settxdbm(uint8_t ch, int16_t db)
{
	int16_t atten, data;
	//atten 								= pstChipDrv->siTxDbm_local - dbm;
	iris404_gettxatten(ch, &atten);
	data									=atten+db;
	iris404_settxatten(ch, data);
}

void iris404_rxctrl(int16_t onoff)
{
	if(onoff==_ON_)
	{
	    /*
    	 * Node1 GPIO bits
    	 *  	Bit 2: TXNRX (output: TX when high; RX when Low)
	     */
//#ifndef HALF_FDD
    	SBX_GPIO_DATA_WRITE( SBX_GPIO_BASE(NODE_1), 0x04, 0x04);
//#else
//		SBX_GPIO_DATA_WRITE( SBX_GPIO_BASE(NODE_1), 0x11, 0x10);
//#endif
		if (pstChipDrv->siMode==FDD_MODE)
		{
			/*
			 * Node3 GPIO bits
     		 *  	Bit 1: PSD_4_9 :O: TX_SPDT_V2 set 1
	     	 *  	Bit 0: PSD_4_9 :O: TX_SPDT_V1 set 0
		     */
//#ifndef HALF_FDD
    		SBX_GPIO_DATA_WRITE( SBX_GPIO_BASE(NODE_3), 0x03, 0x2);   //这里怎么配置现在不清楚，后面再改吧  20170904 by wuqiang
//#endif
		}
		else if(pstChipDrv->siMode==TDD_MODE)
		{
			/*
			 * Node3 GPIO bits
     		 *  	Bit 1: PSD_4_9 :O: TX_SPDT_V2 set 0
	     	 *  	Bit 0: PSD_4_9 :O: TX_SPDT_V1 set 1
		     */
    	//	SBX_GPIO_DATA_WRITE( SBX_GPIO_BASE(NODE_3), 0x03, 0x1);
		}
		else
		{
		}
	}
	else
	{
		/* Sorry, I don't know.*/
		//OFF
		SBX_GPIO_DATA_WRITE( SBX_GPIO_BASE(NODE_1), 0x11, 0x00);
	}
}

/*! \brief 控制功放开启关闭 */
void iris404_pactrl(int16_t onoff)
{
	if(onoff==_ON_)
	{
    	/*
	     * Node3 GPIO bits
	     *  	Bit 2: TX_RF_PWR_EN  set 1
	     */
//#ifndef HALF_FDD
		SBX_GPIO_DATA_WRITE( SBX_GPIO_BASE(NODE_3), 0x04, 0x04);
//#else
//    	SBX_GPIO_DATA_WRITE( SBX_GPIO_BASE(NODE_3), 0x10, 0x10);
//#endif
	}
	else
	{
    	/*
	     * Node3 GPIO bits
    	 *  	Bit 2: TX_RF_PWR_EN set 0
	     */
//#ifndef HALF_FDD
		SBX_GPIO_DATA_WRITE( SBX_GPIO_BASE(NODE_3), 0x04, 0x00);
//#else
//	    SBX_GPIO_DATA_WRITE( SBX_GPIO_BASE(NODE_3), 0x10, 0x00);
//#endif
	}
}

/*! \brief 控制芯片全速运行或低功耗待机 */
void iris404_chipctrl(int16_t onoff)
{
	if(onoff==_ON_)
	{
		/*
	     *  Node1 GPIO
	     *  Bit 5:NGPIO_1_5:O: TX_FRM_EN  set 1
		*/
	    SBX_GPIO_DATA_WRITE( SBX_GPIO_BASE(NODE_1), 0x20, 0x20);
	}
	else
	{
		/*
	     *  Node1 GPIO
	     *  Bit 5:NGPIO_1_5:O: TX_FRM_EN  set 0
		*/
	    SBX_GPIO_DATA_WRITE( SBX_GPIO_BASE(NODE_1), 0x20, 0x00);
	}
}

void cmd_standby_idle(char *par)
{
    reg_write(0x08, 0x1800000);
    reg_write(0x20, 0x1000040);
    reg_write(0x1C, 0x1BF7FB8);
    reg_write(0x03, 0x1E10000);
	///printf("state changed, standby-->idle \n");
}
void cmd_idle_fdidle(char *par)
{
	uint8_t i;
	uint32_t spidata;

	for(i=0;i<6;i++)
	{
		spidata = seq_idle2fdidle[i].ucValue;
		xspi_write_data(NODE_2,SBX_SPI_CSA,spidata,32);
		if(seq_idle2fdidle[i].ucValue == 0x04)
			wait_ms(1);
	}
	///printf("state changed, idle-->fdidle \n");
}

void cmd_idle_fdidle_1_4M()
{
	uint32_t spidata;
	spidata = 0x36ddc000;
	xspi_write_data(NODE_2,SBX_SPI_CSA,spidata,32);
}
void cmd_idle_fdidle_200k()
{
	uint32_t spidata;
	spidata = 0x36f94000;
	xspi_write_data(NODE_2,SBX_SPI_CSA,spidata,32);
}

void cmd_start_fdrx(char *par)
{
	//double txf, rxf;
	reg_write(0x09, 0x1F2C260);
	reg_write(0x0F, 0x113B13B);
	reg_write(0x0E, 0x870000);
	reg_write(0x17, 0x800000);
	mac_start(200,0,0);
	wait_ms(1);
	///printf("state changed, start fdrx \n");
	//iris404_getfreq(&txf,&rxf);
    //printf("the txf is [%.9f]MHz, rxf is [%.9f]MHz.\n",txf,rxf);
}

void cmd_start_fdtx(char *par)
{
    reg_write(0x01, 0x980A);
    reg_write(0x05, 0x13B13B1);
    reg_write(0x04, 0x800000);
    reg_write(0x16, 0x800000);
    mac_start(223,0,0);
    wait_ms(1);
    reg_write(0x2D, 0x1A6F440);
    reg_write(0x1D, 0xF404C);
    reg_write(0x1D, 0xF5C00);
	///printf("state changed, start fdtx \n");
}
/*! \brief 驱动程序初始化，将全局变量初始化并加入回调函数 */
void iris404(ST_RF_DRV *drv)
{
	pstChipDrv = drv;
	pstChipDrv->spLock						= 0;
	pstChipDrv->siIsInited					= 1;
	pstChipDrv->siMode						= FDD_MODE;
	pstChipDrv->siRx0GainMax				= 97;
	pstChipDrv->siRx0GainMin				= 2;//5;
	pstChipDrv->siRx1GainMax				= 97;
	pstChipDrv->siRx1GainMin				= 2;// 5;
	pstChipDrv->siTx0AttenMax				= 26;// 最小衰减值，内部是4格1dBm，故<<2
	pstChipDrv->siTx0AttenMin               = -59;
	pstChipDrv->siTx1AttenMax				= 26;// 最小衰减值，内部是4格1dBm，故<<2
	pstChipDrv->siTx1AttenMin               = -59;

	pstChipDrv->siTxDbm_local				= 26;//16
	pstChipDrv->siRxDb0_local				= 30;//20
	pstChipDrv->siRxDb1_local				= 30;//20
	pstChipDrv->siFreqOffset_local			= 0;
	pstChipDrv->uiCalFreq					= 12;
	//pstChipDrv->siCalDeltaF					= 26000000;

	pstChipDrv->Initialization 				= iris404_config;
	pstChipDrv->InitChip  					= iris404_init;
	pstChipDrv->ResetChip					= iris404_reset;
	pstChipDrv->SetMode						= iris404_setmode;
	pstChipDrv->GetFreq						= iris404_getfreq;
	pstChipDrv->SetFreq						= iris404_setfreq;
	pstChipDrv->GetRxGain					= iris404_getrxgain;
	pstChipDrv->SetRxGain					= iris404_setrxgain;
	pstChipDrv->GetTxAtten					= iris404_gettxatten;
	pstChipDrv->SetTxAtten					= iris404_settxatten;
	pstChipDrv->GetRxDb						= iris404_getrxdb;
	pstChipDrv->SetRxDb						= iris404_setrxdb;
	pstChipDrv->GetTxDbm					= iris404_gettxdbm;
	pstChipDrv->SetTxDbm					= iris404_settxdbm;
	pstChipDrv->GetGpio						= iris404_getgpio;
	pstChipDrv->SetGpio						= iris404_setgpio;
	pstChipDrv->TxCtrl 						= iris404_txctrl;
	pstChipDrv->RxCtrl						= iris404_rxctrl;
	pstChipDrv->TxPACtrl					= iris404_pactrl;
	pstChipDrv->ChipCtrl					= iris404_chipctrl;
	pstChipDrv->AdjustFreqOffset			= iris404_adjustfreqoffset;
	pstChipDrv->GetVolt			= iris404_getVolt;
	pstChipDrv->SetVolt			= iris404_setVolt;

}
/*
 * EOF
 */

