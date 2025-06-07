/* \file
	\brief 射频芯片驱动程序框架
	\details 定义射频芯片驱动程序需要实现的结构体功能函数及变量
	\author 田朝阳<zytain@gmail.com>
	\date 2016-12-21
 */
#ifndef __RF_DRV_H__
#define __RF_DRV_H__

#include <stdint.h>
//#ifndef SB3500
//#ifndef osFastPause
//#define osFastPause(x) (void(0))
//#endif
//#ifndef wait_ms
//#define wait_ms(x) (void(0))
//#endif
//#endif
/* \brief 射频驱动
	\details 射频芯片驱动结构体，带有操作函数，如初始化、初始配置、校准等。 */
typedef struct _ST_RF_DRV
{
	volatile int16_t spLock;								//!< 自旋锁用于有延时操作时闭锁
	int16_t  siIsInited; 	   								//!< 初始化标识
	int16_t  siMode;            							//!< 工作模式 0-TDD;1-FDD
	int16_t  siRx0GainMax,siRx0GainMin;						//!< 接收通道0最大、最小接收增益dB
	int16_t  siRx1GainMax,siRx1GainMin;						//!< 接收通道1最大、最小接收增益dB
    int16_t  siTx1AttenMin;    								//!< 发射衰减最小值dBm
    int16_t  siTx0AttenMax;    								//!< 发射衰减最大值dBm
	int16_t  siTx0AttenMin;    								//!< 发射衰减最小值dBm
	int16_t  siTx1AttenMax;    								//!< 发射衰减最大值dBm
#ifdef ACP_IRIS
	uint8_t  siTxDbm_local;									//!< 本板发射功率偏置，对应于衰减为0dB时的空口绝对功率
    uint8_t  siRxDb0_local, siRxDb1_local;					//!< 本板接收功率偏置，对应于增益索引值为0时的接收增益
    void    (*GetFreq)(double *tx, double *rx);			    //!< 读取收发频点
    uint32_t    (*InitChip)(void);     							//!< 芯片配置初始化
#else
	int16_t  siTxDbm_local;									//!< 本板发射功率偏置，对应于衰减为0dB时的空口绝对功率
	int16_t  siRxDb0_local, siRxDb1_local;					//!< 本板接收功率偏置，对应于增益索引值为0时的接收增益
	void    (*GetFreq)(double *rx, double *tx);			//!< 读取收发频点
	uint32_t    (*InitChip)(void);     							//!< 芯片配置初始化
#endif
	int16_t  siFreqOffset_local;							//!< 本板频率绝对偏置，对应于默认发射频率设置值与测量值的差值
	uint32_t uiCalFreq;
	int16_t  siCalDeltaF;

	void    (*Initialization)(void);						//!< 结构初始化包含SPI/NGPIO初始化
	void    (*ResetChip)(void);   							//!< 芯片软复位
	void    (*Calibration)(void);							//!< 芯片校准
	void    (*SetMode)(int16_t mode);						//!< 设置芯片工作模式
	void    (*SetFreq)(uint32_t rx, uint32_t tx);			//!< 设置收发频点
	void    (*GetRxGain)(uint8_t ch, int16_t *ind);			//!< 读取接收增益索引值
	void    (*SetRxGain)(uint8_t ch, int16_t ind);			//!< 设置接收增益索引
	void 	(*GetTxAtten)(uint8_t ch,int16_t *atten); 		//!< 读取发射衰减
	void 	(*SetTxAtten)(uint8_t ch,int16_t atten); 		//!< 设置发射衰减
	void    (*GetRxDb)(uint8_t ch, int16_t *db);			//!< 读取接收增益
	void    (*SetRxDb)(uint8_t ch, int16_t db);		 		//!< 设置接收增益
	void 	(*GetTxDbm)(uint8_t ch, int16_t *dbm);	 		//!< 读取发射绝对功率
	void 	(*SetTxDbm)(uint8_t ch,int16_t dbm);	 		//!< 设置发射绝对功率
	void	(*AdjustFreqOffset)(int16_t offset);			//!< 频偏补偿
	void    (*GetGpio)(uint16_t *dir,uint16_t *val);		//!< 读取GPIO方向及状态值
	void    (*SetGpio)(uint16_t mask,uint16_t val);			//!< 设置GPIO状态值
	void    (*TxCtrl)(int16_t onoff);						//!< 发射控制
	void    (*RxCtrl)(int16_t onoff);						//!< 接收控制
	void    (*TxPACtrl)(int16_t onoff);						//!< 前置功放控制
	void    (*ChipCtrl)(int16_t onoff);						//!< 开关芯片，关闭时置低功耗
	uint32_t    (*GetVolt)(void);
	void    (*SetVolt)(uint32_t voltage);
} ST_RF_DRV;

/*
	\code
	ST_RF_DRV	stRfDrv;					// 定义结构体变量，一般是全局的定义
	...
			max2580(&stRfDrv);				// 用显示地调用驱动模块提供的初始化
											// 之后操作用隐式调用驱动模块的函数
			stRfDrv.Initialization();	 	// 调用结构体初始化
			stRfDrv.InitChip();				// 调用芯片初始配置
	...
			stRfDrv.SetRxGain(40);			// 调用驱动功能函数；
	...
	\endcode

*/

#endif //RF_DRV_H__
/*
 * EOF
 */
/* Node SPI functions */
// wait for the SPI SM to be busy - started!
#define spi_wait_until_busy(_node)      \
   while ((XSPI_READ((_node), SBX_SPI_STATUS) & 0x01) == 0)

// flush the READ FIFO until empty and the SPI SM is idle
#define spi_flush_read_fifo(_node)      \
  while ((XSPI_READ((_node), SBX_SPI_STATUS) & 0x10) != 0x10) { XSPI_READ((_node), SBX_SPI_READ);  }

// wait for the CMD FIFO to be empty and an idle SPI SM
#define spi_wait_until_idle(_node)      \
  while ((XSPI_READ((_node), SBX_SPI_STATUS) & 0x41) != 0x40)

// wait for Data to be available in the READ FIFO
#define spi_wait_for_rdat(_node)        \
    while ((XSPI_READ((_node), SBX_SPI_STATUS) & 0x11) != 0)

/* \brief 锁定驱动 */
#define SPIN_LOCK()			{ while((pstChipDrv->spLock)!=0) osFastPause(10); (pstChipDrv->spLock) = 1;}
/* \brief 解锁驱动 */
#define SPIN_UNLOCK() 		(pstChipDrv->spLock) = 0
