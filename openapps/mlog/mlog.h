/**
\brief CoAP MAC Log Application

\author Victor Diez Rodriguez <vctdez14@student.hh.se>, August 2015
*/

#ifndef __MLOG_H
#define __MLOG_H

/**
\addtogroup AppCoAP
\{
\addtogroup mlog
\{
*/

#include "opendefs.h"
#include "opencoap.h"

//=========================== define ==========================================
#define MLOG_PERIOD_MS 2000

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void mlog_init(void);

void    mlog_sendDone(
   OpenQueueEntry_t* msg,
   owerror_t         error
);

void mlog_receive(OpenQueueEntry_t* pkt);

void mlog_start(void);

void mlog_stop(void);
/**
\}
\}
*/

#endif
