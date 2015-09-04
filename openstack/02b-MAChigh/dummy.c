#include "opendefs.h"
#include "IEEE802154.h"
#include "openserial.h"
#include "openqueue.h"
#include "dummy.h"
#include "packetfunctions.h"
#include "sixtop.h"
#include "ieee802154_security_driver.h"
#include "IEEE802154E.h"
#include "openrandom.h"


//=========================== variables =======================================

OpenQueueEntry_t dummyList[MAXDUMMY];
extern sixtop_vars_t sixtop_vars;


//=========================== prototypes ======================================

//=========================== public ==========================================

void dummy_init(void){
  int j;

  for(j=0;j<MAXDUMMY;j++){
    dummy_reset(&dummyList[j]);
  }

}

OpenQueueEntry_t* dummy_getPacket(open_addr_t* dest){
  //find dummy with address
  int dummyIndex=0;
  uint8_t random8;
  for(dummyIndex=0;dummyIndex<MAXDUMMY;dummyIndex++){
    if(packetfunctions_sameAddress(&(dummyList[dummyIndex].l2_nextORpreviousHop), dest) ){
      break;
    }
  }

  if(dummyIndex>=MAXDUMMY){
    openserial_printMessage("ERROR GETTING DUMMY",20);
    return NULL;
  }
  random8=openrandom_get16b() & 0x0F;
  dummyList[dummyIndex].l2_payload[0]=random8;
  //print random byte in the begining
  dummyList[dummyIndex].l2_dsn=sixtop_vars.dsn++;
  dummyList[dummyIndex].l2_numTxAttempts=0;
  return &dummyList[dummyIndex];
}


void dummy_createDummy(open_addr_t* neighbor){
  //find dummy without address
  int dummyIndex=0;
  for(dummyIndex=0;dummyIndex<MAXDUMMY;dummyIndex++){
    if(dummyList[dummyIndex].l2_nextORpreviousHop.type== ADDR_NONE ){
      break;
    }
  }
  if(dummyIndex>=MAXDUMMY){
    openserial_printMessage("ERROR CREATING DUMMY",20);
    return;
  }




  //set the address
  memcpy(&dummyList[dummyIndex].l2_nextORpreviousHop,neighbor,sizeof(open_addr_t));
  //add the header
  ieee802154_prependHeader(&dummyList[dummyIndex], dummyList[dummyIndex].l2_frameType,
                              FALSE,
                              dummyList[dummyIndex].l2_dsn,
                              &dummyList[dummyIndex].l2_nextORpreviousHop);

  openserial_messageAppendBuffer("create %",8);
  openserial_messageAppendBuffer(&dummyList[dummyIndex].l2_nextORpreviousHop.addr_64b[0],8);
  openserial_messageAppend('%');
  openserial_messageFlush();
  return;

}
void dummy_deleteDummy(open_addr_t* neighbor){
  //find dummy with address
  int dummyIndex=0;
  for(dummyIndex=0;dummyIndex<MAXDUMMY;dummyIndex++){
    if(packetfunctions_sameAddress(&dummyList[dummyIndex].l2_nextORpreviousHop, neighbor) ){
      break;
    }
  }
  if(dummyIndex>=MAXDUMMY){
    openserial_printMessage("ERROR DELETING DUMMY",20);
    return;
  }

  dummy_reset(&dummyList[dummyIndex]);

  return;


}



void dummy_reset(OpenQueueEntry_t* dummy) {
  int i;
  int length=100;
  //admin
  dummy->owner                        = COMPONENT_SIXTOP_TO_IEEE802154E;
  dummy->payload                      = &(dummy->packet[127 - IEEE802154_SECURITY_TAG_LEN]); // Footer is longer if security is used
  dummy->length                       = 0;
  //l4
  dummy->l4_protocol                  = IANA_UNDEFINED;
  //l3
  dummy->l3_destinationAdd.type       = ADDR_NONE;
  dummy->l3_sourceAdd.type            = ADDR_NONE;
  //l2
  dummy->l2_nextORpreviousHop.type    = ADDR_NONE;
  dummy->l2_frameType                 = IEEE154_TYPE_UNDEFINED;
  dummy->l2_retriesLeft               = 0;
  dummy->l2_IEListPresent             = 0;
  dummy->l2_payloadIEpresent          = 0;
  //l2-security
  dummy->l2_securityLevel             = 0;
  //memcpy(sixtop_message,"MS%0%",5);
  //openserial_messagePutHex(sixtop_message,2,msg->length);
  //openserial_printMessage(sixtop_message,5);
  packetfunctions_reserveHeaderSize(dummy,length);

  //openserial_messagePutHex(sixtop_message,2,msg->length);
  //openserial_printMessage(sixtop_message,5);

  ((uint8_t*)dummy->payload)[0]    = 0;
  for (i=1;i<length;i++){
    ((uint8_t*)dummy->payload)[i]  = i;
  }

  dummy->creator                   = COMPONENT_DUMMY;

  dummy->l2_frameType=IEEE154_TYPE_DUMMY;
  dummy->l2_securityLevel=IEEE802154_SECURITY_LEVEL;
  dummy->l2_keyIdMode=IEEE802154_SECURITY_KEYIDMODE;
  dummy->l2_keyIndex=IEEE802154_SECURITY_K2_KEY_INDEX;
  dummy->l1_txPower=TX_POWER;
}

//=========================== private =========================================
