#include "MACLogger.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================


void MACLogger_init(void){
  int i;

  for(i=0;i<CHANNEL_N;i++){
    MACLogger_channelData[i].txNum=0;
    MACLogger_channelData[i].ackNum=0;
    MACLogger_channelData[i].rssi=0;
    MACLogger_channelData[i].lqi=0;
    MACLogger_channelData[i].rssiSum=0;
    MACLogger_channelData[i].lqiSum=0;

  }

}

void MACLogger_logTx(uint8_t channel){
  MACLogger_channelData[channel].txNum++;
}

void MACLogger_logAck(uint8_t channel,int8_t rssi,uint8_t lqi){
  channelLog_t * entry;
  entry=&MACLogger_channelData[channel];
  entry->ackNum++;
  uint8_t adjustedLQI;
  // if(entry->rssi==0){
  //   entry->rssi=rssi;
  // }
  // else{
  //   entry->rssi=(rssi+entry->rssi+1)/2;
  // }
  //adjust lqi to 0-255
  // if(entry->lqi==0){
  //   //adjust to 0-255
  //   entry->lqiSum=adjustedLQI;
  // }
  // else{
  //   entry->lqi=(adjustedLQI+entry->lqi+1)/2;
  // }
  openserial_messageAppend('%');
  openserial_messageAppend(rssi);
  openserial_messageAppend('%');
  openserial_messageFlush();
  entry->rssiSum+=rssi;
  openserial_messageAppend('%');
  openserial_messageAppendBuffer(&(entry->rssiSum),4);
  openserial_messageAppend('%');
  openserial_messageFlush();
  adjustedLQI=((255*lqi+10)/20)-1135;
  entry->lqiSum+=adjustedLQI;
  entry->avgCount++;
}

void MACLogger_getChannelData(uint8_t channel,channelLog_t* data){
  //calculate averages
  MACLogger_channelData[channel].rssi=
    (MACLogger_channelData[channel].rssiSum -
    MACLogger_channelData[channel].avgCount/2) /
    MACLogger_channelData[channel].avgCount;
  MACLogger_channelData[channel].lqi=
    (MACLogger_channelData[channel].lqiSum +
    MACLogger_channelData[channel].avgCount/2) /
    MACLogger_channelData[channel].avgCount;
  *data=MACLogger_channelData[channel];
  MACLogger_channelData[channel].rssi=0;
  MACLogger_channelData[channel].rssiSum=0;
  MACLogger_channelData[channel].lqi=0;
  MACLogger_channelData[channel].lqiSum=0;
  MACLogger_channelData[channel].avgCount=0;
}

//=========================== private =========================================
