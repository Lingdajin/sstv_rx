/* \file
	\brief ��ƵоƬ����������
	\details ������ƵоƬ����������Ҫʵ�ֵĽṹ�幦�ܺ���������
	\author �ﳯ��<zytain@gmail.com>
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
/* \brief ��Ƶ����
	\details ��ƵоƬ�����ṹ�壬���в������������ʼ������ʼ���á�У׼�ȡ� */
typedef struct _ST_RF_DRV
{
	volatile int16_t spLock;								//!< ��������������ʱ����ʱ����
	int16_t  siIsInited; 	   								//!< ��ʼ����ʶ
	int16_t  siMode;            							//!< ����ģʽ 0-TDD;1-FDD
	int16_t  siRx0GainMax,siRx0GainMin;						//!< ����ͨ��0�����С��������dB
	int16_t  siRx1GainMax,siRx1GainMin;						//!< ����ͨ��1�����С��������dB
    int16_t  siTx1AttenMin;    								//!< ����˥����СֵdBm
    int16_t  siTx0AttenMax;    								//!< ����˥�����ֵdBm
	int16_t  siTx0AttenMin;    								//!< ����˥����СֵdBm
	int16_t  siTx1AttenMax;    								//!< ����˥�����ֵdBm
#ifdef ACP_IRIS
	uint8_t  siTxDbm_local;									//!< ���巢�书��ƫ�ã���Ӧ��˥��Ϊ0dBʱ�Ŀտھ��Թ���
    uint8_t  siRxDb0_local, siRxDb1_local;					//!< ������չ���ƫ�ã���Ӧ����������ֵΪ0ʱ�Ľ�������
    void    (*GetFreq)(double *tx, double *rx);			    //!< ��ȡ�շ�Ƶ��
    uint32_t    (*InitChip)(void);     							//!< оƬ���ó�ʼ��
#else
	int16_t  siTxDbm_local;									//!< ���巢�书��ƫ�ã���Ӧ��˥��Ϊ0dBʱ�Ŀտھ��Թ���
	int16_t  siRxDb0_local, siRxDb1_local;					//!< ������չ���ƫ�ã���Ӧ����������ֵΪ0ʱ�Ľ�������
	void    (*GetFreq)(double *rx, double *tx);			//!< ��ȡ�շ�Ƶ��
	uint32_t    (*InitChip)(void);     							//!< оƬ���ó�ʼ��
#endif
	int16_t  siFreqOffset_local;							//!< ����Ƶ�ʾ���ƫ�ã���Ӧ��Ĭ�Ϸ���Ƶ������ֵ�����ֵ�Ĳ�ֵ
	uint32_t uiCalFreq;
	int16_t  siCalDeltaF;

	void    (*Initialization)(void);						//!< �ṹ��ʼ������SPI/NGPIO��ʼ��
	void    (*ResetChip)(void);   							//!< оƬ��λ
	void    (*Calibration)(void);							//!< оƬУ׼
	void    (*SetMode)(int16_t mode);						//!< ����оƬ����ģʽ
	void    (*SetFreq)(uint32_t rx, uint32_t tx);			//!< �����շ�Ƶ��
	void    (*GetRxGain)(uint8_t ch, int16_t *ind);			//!< ��ȡ������������ֵ
	void    (*SetRxGain)(uint8_t ch, int16_t ind);			//!< ���ý�����������
	void 	(*GetTxAtten)(uint8_t ch,int16_t *atten); 		//!< ��ȡ����˥��
	void 	(*SetTxAtten)(uint8_t ch,int16_t atten); 		//!< ���÷���˥��
	void    (*GetRxDb)(uint8_t ch, int16_t *db);			//!< ��ȡ��������
	void    (*SetRxDb)(uint8_t ch, int16_t db);		 		//!< ���ý�������
	void 	(*GetTxDbm)(uint8_t ch, int16_t *dbm);	 		//!< ��ȡ������Թ���
	void 	(*SetTxDbm)(uint8_t ch,int16_t dbm);	 		//!< ���÷�����Թ���
	void	(*AdjustFreqOffset)(int16_t offset);			//!< Ƶƫ����
	void    (*GetGpio)(uint16_t *dir,uint16_t *val);		//!< ��ȡGPIO����״ֵ̬
	void    (*SetGpio)(uint16_t mask,uint16_t val);			//!< ����GPIO״ֵ̬
	void    (*TxCtrl)(int16_t onoff);						//!< �������
	void    (*RxCtrl)(int16_t onoff);						//!< ���տ���
	void    (*TxPACtrl)(int16_t onoff);						//!< ǰ�ù��ſ���
	void    (*ChipCtrl)(int16_t onoff);						//!< ����оƬ���ر�ʱ�õ͹���
	uint32_t    (*GetVolt)(void);
	void    (*SetVolt)(uint32_t voltage);
} ST_RF_DRV;

/*
	\code
	ST_RF_DRV	stRfDrv;					// ����ṹ�������һ����ȫ�ֵĶ���
	...
			max2580(&stRfDrv);				// ����ʾ�ص�������ģ���ṩ�ĳ�ʼ��
											// ֮���������ʽ��������ģ��ĺ���
			stRfDrv.Initialization();	 	// ���ýṹ���ʼ��
			stRfDrv.InitChip();				// ����оƬ��ʼ����
	...
			stRfDrv.SetRxGain(40);			// �����������ܺ�����
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

/* \brief �������� */
#define SPIN_LOCK()			{ while((pstChipDrv->spLock)!=0) osFastPause(10); (pstChipDrv->spLock) = 1;}
/* \brief �������� */
#define SPIN_UNLOCK() 		(pstChipDrv->spLock) = 0
