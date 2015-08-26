/** \brief CoAP 6top application.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, February 2013.
\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, July 2014
*/

#include "opendefs.h"
#include "c6t.h"
#include "sixtop.h"
#include "idmanager.h"
#include "openqueue.h"
#include "neighbors.h"
#include "string.h"
#include "schedule.h"
//=========================== defines =========================================
#define space_conc(str1,str2) #str1 " " #str2

const uint8_t c6t_path0[] = "6t";

//=========================== variables =======================================

c6t_vars_t c6t_vars;
uint8_t slot;
extern schedule_vars_t schedule_vars;
extern neighbors_vars_t neighbors_vars;


//=========================== prototypes ======================================
void c6t_put8b(char * buf,
   int8_t number
);
void c6t_put16b(
   char * buf,
   uint16_t number
);
owerror_t c6t_receive(
   OpenQueueEntry_t* msg,
   coap_header_iht*  coap_header,
   coap_option_iht*  coap_options
);
void    c6t_sendDone(
   OpenQueueEntry_t* msg,
   owerror_t         error
);

//=========================== public ==========================================

void c6t_init() {
   if(idmanager_getIsDAGroot()==TRUE) return;

   // prepare the resource descriptor for the /6t path
   c6t_vars.desc.path0len            = sizeof(c6t_path0)-1;
   c6t_vars.desc.path0val            = (uint8_t*)(&c6t_path0);
   c6t_vars.desc.path1len            = 0;
   c6t_vars.desc.path1val            = NULL;
   c6t_vars.desc.componentID         = COMPONENT_C6T;
   c6t_vars.desc.discoverable        = TRUE;
   c6t_vars.desc.callbackRx          = &c6t_receive;
   c6t_vars.desc.callbackSendDone    = &c6t_sendDone;

   opencoap_register(&c6t_vars.desc);
}

//=========================== private =========================================

/**
\brief Receives a command and a list of items to be used by the command.

\param[in] msg          The received message. CoAP header and options already
   parsed.
\param[in] coap_header  The CoAP header contained in the message.
\param[in] coap_options The CoAP options contained in the message.

\return Whether the response is prepared successfully.
*/
owerror_t c6t_receive(
      OpenQueueEntry_t* msg,
      coap_header_iht*  coap_header,
      coap_option_iht*  coap_options
   ) {

   int i;
   owerror_t            outcome;
   open_addr_t          neighbor;
   bool                 foundNeighbor;
   neighborSignal_t     signalData;

   switch (coap_header->Code) {

      case COAP_CODE_REQ_PUT:
         // add a slot

         // reset packet payload
         msg->payload                  = &(msg->packet[127]);
         msg->length                   = 0;

         // get preferred parent
         foundNeighbor = neighbors_getPreferredParentEui64(&neighbor);
         if (foundNeighbor==FALSE) {
            outcome                    = E_FAIL;
            coap_header->Code          = COAP_CODE_RESP_PRECONDFAILED;
            break;
         }


         sixtop_setHandler(SIX_HANDLER_OTF);
         // call sixtop
         sixtop_addCells(
            &neighbor,
            1
         );

         // set the CoAP header
         coap_header->Code             = COAP_CODE_RESP_CHANGED;

         outcome                       = E_SUCCESS;
         break;

      case COAP_CODE_REQ_DELETE:
         // delete a slot

         // reset packet payload
         msg->payload                  = &(msg->packet[127]);
         msg->length                   = 0;

         // get preferred parent
         foundNeighbor = neighbors_getPreferredParentEui64(&neighbor);
         if (foundNeighbor==FALSE) {
            outcome                    = E_FAIL;
            coap_header->Code          = COAP_CODE_RESP_PRECONDFAILED;
            break;
         }

         sixtop_setHandler(SIX_HANDLER_OTF);
         // call sixtop
         sixtop_removeCell(
            &neighbor
         );

         // set the CoAP header
         coap_header->Code             = COAP_CODE_RESP_CHANGED;

         outcome                       = E_SUCCESS;
         break;

      case COAP_CODE_REQ_GET:
        //=== reset packet payload (we will reuse this packetBuffer)
        msg->payload                     = &(msg->packet[127]);
        msg->length                      = 0;

        //=== prepare  CoAP response

        neighbors_getPreferredParentEui64(&neighbor);
        neighbors_getSignalData(&neighbor,&signalData);

        packetfunctions_reserveHeaderSize(msg,1);
        msg->payload[0]='\n';
        packetfunctions_reserveHeaderSize(msg,sizeof(signalData.lqi)*2);
        c6t_put8b(&msg->payload[0], signalData.lqi);
        packetfunctions_reserveHeaderSize(msg,1);
        msg->payload[0]=',';
        packetfunctions_reserveHeaderSize(msg,sizeof(signalData.ackRssi)*2);
        c6t_put8b(&msg->payload[0], signalData.ackRssi);
        //get slotoffset to show
        for(i=0;i<NUMCELL;i++){
          //only get TX cells
          while(schedule_vars.scheduleBuf[slot].type!=CELLTYPE_TX){
            slot=(slot+1)%MAXACTIVESLOTS;
          }
          // cell
          packetfunctions_reserveHeaderSize(msg,1);
          msg->payload[0]='\n';
          packetfunctions_reserveHeaderSize(msg,sizeof(schedule_vars.scheduleBuf[slot].numTxACK)*2);
          c6t_put16b(&msg->payload[0],schedule_vars.scheduleBuf[slot].numTxACK);
          packetfunctions_reserveHeaderSize(msg,1);
          msg->payload[0]='/';
          packetfunctions_reserveHeaderSize(msg,sizeof(schedule_vars.scheduleBuf[slot].numTx)*2);
          c6t_put16b(&msg->payload[0],schedule_vars.scheduleBuf[slot].numTx);
          packetfunctions_reserveHeaderSize(msg,1);
          msg->payload[0]=':';
          packetfunctions_reserveHeaderSize(msg,sizeof(schedule_vars.scheduleBuf[slot].slotOffset)*2);
          c6t_put16b(&msg->payload[0],schedule_vars.scheduleBuf[slot].slotOffset);
          slot=(slot+1)%MAXACTIVESLOTS;

        }

        //payload marker
        packetfunctions_reserveHeaderSize(msg,1);
        msg->payload[0] = COAP_PAYLOAD_MARKER;

        // set the CoAP header
        coap_header->Code                = COAP_CODE_RESP_CONTENT;

        outcome                          = E_SUCCESS;
         break;
      default:
         outcome = E_FAIL;
         break;
   }

   return outcome;
}

void c6t_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}

void c6t_put16b(char * buf, uint16_t number){
  const char hex[] = "0123456789abcdef";
  buf[3] = hex[number & 0xF];
  buf[2] = hex[(number >> 4) & 0xF];
  buf[1] = hex[(number >> 8) & 0xF];
  buf[0] = hex[number >> 12];
}

void c6t_put8b(char * buf, int8_t number){
  const char hex[] = "0123456789abcdef";
  buf[1] = hex[number & 0xF];
  buf[0] = hex[(number >> 4) & 0xF];
}
