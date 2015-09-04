/**
\brief CoAP MAC Log Application

\author Victor Diez Rodriguez <vctdez14@student.hh.se>, August 2015
*/

#include "opendefs.h"
#include "mlog.h"
#include "openqueue.h"
#include "string.h"
#include "MACLogger.h"
#include "scheduler.h"
#include "IEEE802154E.h"
#include "openserial.h"
#include "openudp.h"
#include "idmanager.h"
#include "packetfunctions.h"
//=========================== defines =========================================

const uint8_t mlog_path0[] = "mlog";

static const uint8_t mlog_dst_addr[]   = {
   0xbb, 0xbb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
};
//=========================== variables =======================================

opentimer_id_t timerIdMlog;
//=========================== prototypes ======================================

void mlog_timer_cb(opentimer_id_t id);
void mlog_task_cb(void);
void mlog_putPayload(OpenQueueEntry_t* msg);
//=========================== public ==========================================

void mlog_init() {
  openserial_messageAppendBuffer("inim",4);
  openserial_messageFlush();

}

void mlog_start(){
  timerIdMlog=opentimers_start(
     MLOG_PERIOD_MS,
     TIMER_PERIODIC,TIME_MS,
     mlog_timer_cb
  );
}

void mlog_stop(){
  opentimers_stop(timerIdMlog);
}

void mlog_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}

void mlog_receive(OpenQueueEntry_t* pkt) {

   openqueue_freePacketBuffer(pkt);

   openserial_printError(
      COMPONENT_MLOG,
      ERR_RCVD_ECHO_REPLY,
      (errorparameter_t)0,
      (errorparameter_t)0
   );
}

//=========================== private =========================================
/**
\note timer fired, but we don't want to execute task in ISR mode instead, push
   task to scheduler with CoAP priority, and let scheduler take care of it.
*/
void mlog_timer_cb(opentimer_id_t id){

   scheduler_push_task(mlog_task_cb,TASKPRIO_COAP);

}

void mlog_task_cb() {
   OpenQueueEntry_t*    pkt;

   // don't run if not synch
   if (ieee154e_isSynch() == FALSE) return;

   // don't run on dagroot
   if (idmanager_getIsDAGroot()) {
      opentimers_stop(timerIdMlog);
      return;
   }

   // if you get here, send a packet

   // get a free packet buffer
   pkt = openqueue_getFreePacketBuffer(COMPONENT_MLOG);
   if (pkt==NULL) {
      openserial_printError(
         COMPONENT_MLOG,
         ERR_NO_FREE_PACKET_BUFFER,
         (errorparameter_t)0,
         (errorparameter_t)0
      );
      return;
   }

   pkt->owner                         = COMPONENT_MLOG;
   pkt->creator                       = COMPONENT_MLOG;
   pkt->l4_protocol                   = IANA_UDP;
   pkt->l4_destination_port           = WKP_UDP_MLOG;
   pkt->l4_sourcePortORicmpv6Type     = WKP_UDP_MLOG;
   pkt->l3_destinationAdd.type        = ADDR_128B;
   memcpy(&pkt->l3_destinationAdd.addr_128b[0],mlog_dst_addr,16);

   mlog_putPayload(pkt);

   if ((openudp_send(pkt))==E_FAIL) {
      openqueue_freePacketBuffer(pkt);
   }
}

void  mlog_putPayload(OpenQueueEntry_t* msg){
  static uint8_t currentChannel;
  channelLog_t channelData;
  uint8_t channelCount;
  uint8_t payloadPointer=0;

  msg->payload                  = &(msg->packet[127]);
  msg->length                   = 0;

  packetfunctions_reserveHeaderSize(msg,56);

  //=== prepare  CoAP response
  for(channelCount=0;channelCount<8;channelCount++){
    MACLogger_getChannelData(currentChannel,&channelData);
    memcpy(&msg->payload[payloadPointer],&currentChannel,sizeof(currentChannel));
    payloadPointer++;
    memcpy(&msg->payload[payloadPointer],&channelData.txNum,sizeof(channelData.txNum));
    payloadPointer+=2;
    memcpy(&msg->payload[payloadPointer],&channelData.ackNum,sizeof(channelData.ackNum));
    payloadPointer+=2;
    memcpy(&msg->payload[payloadPointer],&channelData.rssi,sizeof(channelData.rssi));
    payloadPointer++;
    memcpy(&msg->payload[payloadPointer],&channelData.lqi,sizeof(channelData.lqi));
    payloadPointer++;




    //packetfunctions_reserveHeaderSize(msg,sizeof(channelData.rssi));
    //memcpy(&msg->payload[0],&channelData.rssi,sizeof(channelData.rssi));
    //msg->payload[channel]=channel;
    //packetfunctions_reserveHeaderSize(msg,sizeof(channelData.ackNum));
    //memcpy(&msg->payload[0],&channelData.ackNum,sizeof(channelData.ackNum));
    //packetfunctions_reserveHeaderSize(msg,sizeof(channelData.txNum));
    //memcpy(&msg->payload[0],&channelData.txNum,sizeof(channelData.txNum));

    //packetfunctions_reserveHeaderSize(msg,1);
    //msg->payload[0] = 51;
    //packetfunctions_reserveHeaderSize(msg,sizeof(channelData.rssi));
    //memcpy(&msg->payload[0],&channelData.rssi,sizeof(channelData.rssi));
    //packetfunctions_reserveHeaderSize(msg,sizeof(channelData.ackNum));
    //memcpy(&msg->payload[0],&channelData.ackNum,sizeof(channelData.ackNum));
    //packetfunctions_reserveHeaderSize(msg,sizeof(channelData.txNum));
    //memcpy(&msg->payload[0],&channelData.txNum,sizeof(channelData.txNum));
    currentChannel=(currentChannel+1)%16;
  }

}
