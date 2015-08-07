#ifndef __DUMMY_H
#define __DUMMY_H

#define MAXDUMMY 4

void dummy_init(void);
OpenQueueEntry_t * dummy_getPacket(open_addr_t * dest);
void dummy_createDummy(open_addr_t * neighbor);
void dummy_deleteDummy(open_addr_t * neighbor);

#endif
