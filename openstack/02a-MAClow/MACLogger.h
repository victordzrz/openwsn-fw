#ifndef __MACLOGGER_H
#define __MACLOGGER_H

/**
\addtogroup MAChigh
\{
\addtogroup MACLogger
\{
*/
#include "opendefs.h"


//=========================== define ==========================================

#define CHANNEL_N       16

//=========================== typedef =========================================
BEGIN_PACK
typedef struct {
  uint16_t txNum;
  uint16_t ackNum;
  int8_t rssi;
  int32_t rssiSum;
  uint8_t lqi;
  uint32_t lqiSum;
  uint16_t avgCount;
} channelLog_t;
END_PACK


//=========================== module variables ================================

channelLog_t         MACLogger_channelData[CHANNEL_N];

//=========================== prototypes ======================================
void MACLogger_init(void);

void MACLogger_logTx(
  uint8_t channel
);

void MACLogger_logAck(
  uint8_t channel,
  int8_t rssi,
  uint8_t lqi
);

void MACLogger_notifyAck(
  open_addr_t*          neighbor,
  int8_t                rssi,
  uint8_t               lqi
);

void MACLogger_getChannelData(
  uint8_t         channel,
  channelLog_t*    data
);

#endif
