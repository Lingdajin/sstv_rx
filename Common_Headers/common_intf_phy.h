/*******************************************************************************
    OpenAirInterface
    Copyright(c) 1999 - 2014 Eurecom

    OpenAirInterface is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.


    OpenAirInterface is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenAirInterface.The full GNU General Public License is
   included in this distribution in the file called "COPYING". If not,
   see <http://www.gnu.org/licenses/>.

  Contact Information
  OpenAirInterface Admin: openair_admin@eurecom.fr
  OpenAirInterface Tech : openair_tech@eurecom.fr
  OpenAirInterface Dev  : openair4g-devel@lists.eurecom.fr

  Address      : Eurecom, Campus SophiaTech, 450 Route des Chappes, CS 50193 - 06904 Biot Sophia Antipolis cedex, FRANCE

 *******************************************************************************/

/* \file PHY/common_intf_phy.h
* \brief LTE Physical channel configuration and variable structure definitions
* \author R. Knopp, F. Kaltenberger
* \date 2011
* \version 0.1
* \company Eurecom
* \email: knopp@eurecom.fr,florian.kaltenberger@eurecom.fr
* \note
* \warning
*/

#ifndef __COMMON_IMPLEMENTATION_DEFS_LTE_H__
#define __COMMON_IMPLEMENTATION_DEFS_LTE_H__

#define FREERTOS_FLAG 1

#include <stdint.h>
#if (0 == FREERTOS_FLAG)
#include "openair1/PHY/impl_defs_lte.h"
#endif
//#include "defs.h"

#if (1 == FREERTOS_FLAG)
#define LTE_NUMBER_OF_SUBFRAMES_PER_FRAME 10
#define LTE_SLOTS_PER_FRAME  20
#define LTE_CE_FILTER_LENGTH 5
#define LTE_CE_OFFSET LTE_CE_FILTER_LENGTH
#define TX_RX_SWITCH_SYMBOL (NUMBER_OF_SYMBOLS_PER_FRAME>>1)
#define PBCH_PDU_SIZE 3 //bytes

#define PRACH_SYMBOL 3 //position of the UL PSS wrt 2nd slot of special subframe

///#define NUMBER_OF_FREQUENCY_GROUPS (lte_frame_parms->N_RB_DL)

#define SSS_AMP 1148

#define MAX_NUM_PHICH_GROUPS 56  //110 RBs Ng=2, p.60 36-212, Sec. 6.9

#define MAX_MBSFN_AREA 8

#if defined(CBMIMO1) || defined(EXMIMO) || defined(OAI_USRP)
#define NUMBER_OF_eNB_MAX 1
#define NUMBER_OF_UE_MAX 4
#define NUMBER_OF_CONNECTED_eNB_MAX 3
#else
#ifdef LARGE_SCALE
#define NUMBER_OF_eNB_MAX 2
#define NUMBER_OF_UE_MAX 120
#define NUMBER_OF_CONNECTED_eNB_MAX 1 // to save some memory
#else
////#define NUMBER_OF_eNB_MAX 7
#define NUMBER_OF_UE_MAX 16
#define NUMBER_OF_CONNECTED_eNB_MAX 3
#endif
#endif

typedef enum {TDD=1,FDD=0} lte_frame_type_t;

typedef enum {EXTENDED=1,NORMAL=0} lte_prefix_type_t;

typedef enum {LOCALIZED=0,DISTRIBUTED=1} vrb_t;

#define NBIOT_RSRP_THRESHOLDS_PRACH_INFO_LIST_NUMBER 2
#define MAC_NPRACH_RESOURCES_NB_R13 3
/// PRACH-ConfigSIB or PRACH-Config from 36.331 RRC spec
typedef struct {
uint32_t     nprach_Periodicity_r13;
uint32_t     nprach_StartTime_r13;
uint32_t     nprach_SubcarrierOffset_r13;
uint32_t     nprach_NumSubcarriers_r13;
uint32_t     nprach_SubcarrierMSG3_RangeStart_r13_mult3;
uint32_t     maxNumPreambleAttemptCE_r13;
uint32_t     numRepetitionsPerPreambleAttempt_r13;
uint32_t     npdcch_NumRepetitions_RA_r13;
float        npdcch_StartSF_CSS_RA_r13;
float        npdcch_Offset_RA_r13;
} NPRACH_CONFIG_COMMON_PARA_t;
typedef struct {
    //  NPRACH_ConfigSIB_NB_r13__nprach_CP_Length_r13_us66dot7  = 0,
    //  NPRACH_ConfigSIB_NB_r13__nprach_CP_Length_r13_us266dot7 = 1
    uint32_t    nprach_CP_Length_r13;
    uint32_t    rsrp_ThresholdsPrachInfo[NBIOT_RSRP_THRESHOLDS_PRACH_INFO_LIST_NUMBER];
    NPRACH_CONFIG_COMMON_PARA_t para[MAC_NPRACH_RESOURCES_NB_R13];
    uint32_t    rsrp_ThresholdsNum;
    uint32_t    valid;    /// 0: invalid, 1: valid

} NPRACH_CONFIG_COMMON_NB_t;
typedef struct {
	uint32_t ra_ResponseWindowSize_r13;
	uint32_t mac_ContentionResolutionTimer_r13;
}RACH_INFO_t;
typedef struct {
    uint32_t    preambleTransMax;
    uint32_t    powerRampingStep;
    int32_t     initialTargetPower;
    RACH_INFO_t rachInfo[MAC_NPRACH_RESOURCES_NB_R13];
    uint32_t    valid;    /// 0: invalid, 1: valid

} RACH_CONFIG_COMMON_NB_t;

typedef struct {
    uint32_t nrs_Power_r13;
    uint32_t    valid;    /// 0: invalid, 1: valid
} NPDSCH_CONFIG_COMMON_NB_t;

typedef struct {
    uint32_t    default_paging_cycle_r13;
    uint32_t    nb_r13;
    uint32_t     npdcch_numrepetition_paging_r13;
    uint32_t    valid;    /// 0: invalid, 1: valid

} PCCH_CONFIG_COMMON_NB_t;

typedef struct {
	uint32_t	threeTone_BaseSequence_r13	/* OPTIONAL */;
	uint32_t	threeTone_CyclicShift_r13;
	uint32_t	sixTone_BaseSequence_r13	/* OPTIONAL */;
	uint32_t	sixTone_CyclicShift_r13;
	uint32_t	twelveTone_BaseSequence_r13	/* OPTIONAL */;
}NPUSCH_CONFIGCOMMON_NB_R13__DMRS_CONFIG_R13_T;

typedef struct {
    uint32_t ack_NACK_NumRepetitions_Msg4[MAC_NPRACH_RESOURCES_NB_R13];
    uint32_t groupHoppingEnabled_r13;
    uint32_t groupAssignmentNPUSCH_r13;
    uint32_t valid;    /// 0: invalid, 1: valid
	uint32_t srs_SubframeConfig_r13;
	NPUSCH_CONFIGCOMMON_NB_R13__DMRS_CONFIG_R13_T dmrs_Config_r13;
} NPUSCH_CONFIG_COMMON_NB_t;

typedef struct {
    uint32_t     dl_GapThreshold_r13;
    uint32_t     dl_GapPeriodicity_r13;
    float        dl_GapDurationCoeff_r13;
} DL_GAPCONFIG_NB_t;

typedef struct {
    uint32_t ack_NACK_NumRepetitions_r13;
    uint32_t npusch_AllSymbols_r13;
    uint32_t groupHoppingDisabled_r13;
    uint32_t    valid;    /// 0: invalid, 1: valid
} NPUSCH_CONFIG_DEDICATED_NB_t;

typedef struct {
    uint32_t npdcch_NumRepetitions_r13;
    float npdcch_StartSF_USS_r13;
    float npdcch_Offset_USS_r13;
    uint32_t    valid;    /// 0: invalid, 1: valid
} NPDCCH_CONFIG_DEDICATED_NB_t;


/// PDSCH-ConfigCommon from 36.331 RRC spec
typedef struct {
  // Parameter: Reference-signal power, see TS 36.213 (5.2). \vr{[-60..50]}\n Provides the downlink reference-signal EPRE. The actual value in dBm.
  int8_t referenceSignalPower;
  // Parameter: \f$P_B\f$, see TS 36.213 (Table 5.2-1). \vr{[0..3]}
  uint8_t p_b;
} PDSCH_CONFIG_COMMON;

// Enumeration for Parameter \f$P_A\f$ \ref PDSCH_CONFIG_DEDICATED::p_a.
typedef enum {
  dBm6=0, ///< (dB-6) corresponds to -6 dB
  dBm477, ///< (dB-4dot77) corresponds to -4.77 dB
  dBm3,   ///< (dB-3) corresponds to -3 dB
  dBm177, ///< (dB-1dot77) corresponds to -1.77 dB
  dB0,    ///< corresponds to 0 dB
  dB1,    ///< corresponds to 1 dB
  dB2,    ///< corresponds to 2 dB
  dB3     ///< corresponds to 3 dB
} PA_t;

// PDSCH-ConfigDedicated from 36.331 RRC spec
typedef struct {
  // Parameter: \f$P_A\f$, see TS 36.213 (5.2).
  PA_t p_a;
} PDSCH_CONFIG_DEDICATED;


// \note UNUSED
typedef enum {
  ulpc_al0=0,
  ulpc_al04=1,
  ulpc_al05=2,
  ulpc_al06=3,
  ulpc_al07=4,
  ulpc_al08=5,
  ulpc_al09=6,
  ulpc_al11=7
} UL_POWER_CONTROL_COMMON_alpha_t;

// Enumeration for parameter \f$\alpha\f$ \ref UL_POWER_CONTROL_CONFIG_COMMON::alpha.
typedef enum {
  al0=0,
  al04=1,
  al05=2,
  al06=3,
  al07=4,
  al08=5,
  al09=6,
  al1=7
} PUSCH_alpha_t;


/// UplinkPowerControlCommon Information Element from 36.331 RRC spec \note this structure does not currently make use of \ref deltaFList_PUCCH_t.
typedef struct {
  // Parameter: \f$\alpha\f$, see TS 36.213 (5.1.1.1) \warning the enumeration values do not correspond to the given values in the specification (al04 should be 0.4, ...)!
  PUSCH_alpha_t alpha;
  // Parameter: \f$P_\text{0\_NOMINAL\_PUSCH}(1)\f$, see TS 36.213 (5.1.1.1), unit dBm. \vr{[-126..24]}\n This field is applicable for non-persistent scheduling, only.
  int32_t p0_NominalPUSCH;
  // Parameter: \f$\Delta_\text{PREAMBLE\_Msg3}\f$ see TS 36.213 (5.1.1.1). \vr{[-1..6]}\n Actual value = IE value * 2 [dB].
  int32_t deltaPreambleMsg3;
//  int8_t spare[2];
} UL_POWER_CONTROL_CONFIG_COMMON_NB_t;



#define PBCH_A 24


//typedef struct {
//  uint8_t ra_PreambleIndex;
//  /// Corresponding RA-RNTI for UL-grant
//  uint16_t ra_RNTI;
//
//#ifdef NBIOT
//  uint8_t RA_PREAMBLE_resouce_id;   //// prach resouce
//  uint32_t RA_ContentionResolutionTimer;
//#endif
//
//  /// Pointer to Msg3 payload for UL-grant
//#ifdef NBIOT
//  uint8_t Msg3[88];
//#else
//  uint8_t *Msg3;
//#endif
//} PRACH_RESOURCES_t;



typedef enum {
  NOT_SYNCHED=0,
  RESYNCH=1,
  SYNCING_STEP1 = 2,
  SYNCING_STEP2 = 3,
  SSS    =5,
  PBCH_WAITING   =6,
  SIB1_WAITING = 7,
  SI_WAITING = 8,
  PUSCH_WAITING = 9,
  IDLE_WAITING = 10,
  PBCH =  11,
  SIB1   =12,
  SI     =13,
  PRACH  =14,
  RA_RESPONSE = 15,
  PUSCH=16,
  PAGING =17,
  UE_IDLE   = 18,
} UE_MODE_t;

#endif
typedef struct {
	/// PRACH_CONFIG
	RACH_CONFIG_COMMON_NB_t   rach_config_common_nb;
	NPRACH_CONFIG_COMMON_NB_t nprach_config_common_nb;
	/// PDSCH Config Common (from 36-331 RRC spec)
	NPDSCH_CONFIG_COMMON_NB_t npdsch_config_common_nb;
	/// PUSCH Config Common (from 36-331 RRC spec)
	NPUSCH_CONFIG_COMMON_NB_t npusch_config_common_nb;
	PCCH_CONFIG_COMMON_NB_t pcch_config_common_nb;
	DL_GAPCONFIG_NB_t dl_gap_config_nb;
	UL_POWER_CONTROL_CONFIG_COMMON_NB_t ul_power_control_common_nb;

	NPDCCH_CONFIG_DEDICATED_NB_t npdcch_ConfigDedicated_r13[NUMBER_OF_CONNECTED_eNB_MAX];
	NPUSCH_CONFIG_DEDICATED_NB_t npusch_ConfigDedicated_r13[NUMBER_OF_CONNECTED_eNB_MAX];
    int32_t  ul_carrier_freq_r13;//from sib2
}LTE_PHY_CONFIG;

#define MAX_NBIOT_SI 8
#if (1 == FREERTOS_FLAG)
typedef struct {
	uint32_t     systemInfoValueTagSIB1_r13;
    uint32_t     ab_Enabled_r13;
	uint32_t     schedulingInfoSIB1_r13;
    uint32_t     schedulingInfoSIB1_repeat_num;
    uint32_t     schedulingInfoSIB1_TBS;
    uint32_t    si_RadioFrameOffset_r13;  //from protocol
    uint32_t    si_WindowLength_r13;
    uint32_t    si_number;
    uint32_t    si_WindowLength_rf;
    uint32_t    si_Periodicity_r13[MAX_NBIOT_SI];
    uint32_t    si_Periodicity_rf[MAX_NBIOT_SI];
    uint32_t    si_RepetitionPattern_r13[MAX_NBIOT_SI];
    uint32_t    si_TB_r13[MAX_NBIOT_SI];
    uint32_t    si_StartOffset[MAX_NBIOT_SI];//offset for every SI

    uint32_t    si_repeat_interval[MAX_NBIOT_SI];
    uint32_t    si_repeat_number[MAX_NBIOT_SI];
    uint32_t    si_number_of_sf[MAX_NBIOT_SI];
    uint32_t    si_TB_size[MAX_NBIOT_SI];

    uint32_t    sib1_received;
	uint32_t afc_on;   //// 0: no AFC,  1: AFC
	int32_t  afc_step_hz;  //// <=0: use measured frequency;   >0: adjust frequency every time by afc_step_hz
	uint32_t afc_thresh_hz; //// the thresh-hold for frequency adjustment
	uint32_t afc_period;    ///// unit: 10 Frames, average the measured frequency offset during afc_period, should be >0
	uint32_t dl_crc_simulation;

	uint32_t agc_on;
	int32_t agc_target_scale;
} NBIOT_PARA_TYPE;
#else
typedef struct {
	uint32_t     systemInfoValueTagSIB1_r13;
    uint32_t     ab_Enabled_r13;
	uint32_t     schedulingInfoSIB1_r13;
    uint32_t     schedulingInfoSIB1_repeat_num;
    uint32_t     schedulingInfoSIB1_TBS;
    uint32_t    si_RadioFrameOffset_r13;  //from protocol
    uint32_t    si_WindowLength_r13;
    uint32_t    si_number;
    uint32_t    si_WindowLength_rf;
    uint32_t    si_Periodicity_r13[MAX_NBIOT_SI];
    uint32_t    si_Periodicity_rf[MAX_NBIOT_SI];
    uint32_t    si_RepetitionPattern_r13[MAX_NBIOT_SI];
    uint32_t    si_TB_r13[MAX_NBIOT_SI];
    uint32_t    si_StartOffset[MAX_NBIOT_SI];//offset for every SI

    uint32_t    si_repeat_interval[MAX_NBIOT_SI];
    uint32_t    si_repeat_number[MAX_NBIOT_SI];
    uint32_t    si_number_of_sf[MAX_NBIOT_SI];
    uint32_t    si_TB_size[MAX_NBIOT_SI];

    uint32_t    sib1_received;
} NBIOT_SI_PARA_TYPE;
#endif
#define MAX_NUM_LCID 4
#define MAX_NUM_LCGID 4

#define TIMERS_STOP 0

#define TIMERS_RUN 1

#define TIMERS_EXPIRE 2


typedef struct
{
	uint32_t rlcPduNum;
	uint32_t Status;
}LCID_status_t;
/*!\brief UE scheduling info */
typedef struct {
	 /// keep the number of bytes in rlc buffer for each lcid

	  uint32_t  BSR_bytes;
	  uint32_t  BSR; // should be more for mesh topology
	// buffer status for each lcid
	  LCID_status_t  Lcid_status[MAX_NUM_LCID];

	  uint32_t regularBSR_trigger;
	  uint32_t periodicBSR_trigger;
	  uint32_t retxBSR_active;

	  int32_t  retxBSR_SF;
	  uint32_t retxBSR_counter;

	  uint32_t periodicBSR_active;
	  uint32_t periodicBSR_counter;
	  int32_t  periodicBSR_SF;

	  uint32_t  enable_logchlSR_ProhibitTimer[MAX_NUM_LCID];// 0: not enable 1: enable
	  uint32_t logicalChannelSR_ProhibitTimer_status; // 0: stop 1:running 3: not value
	  uint32_t  logicalChannelSR_ProhibitTimer_counter;
	  uint32_t logicalChannelSR_ProhibitTimer;

	  // add TA tiemrs define
	  uint32_t timeAlignmentTime_status;// 0: stop  1: running 2: expire
	  int32_t timeAlignmentTimeCommon;
	  int32_t timeAlignmentTimerDedicated;
	  uint32_t timeAlignmentTime_counter;

} UE_SCHEDULING_INFO;

#endif


