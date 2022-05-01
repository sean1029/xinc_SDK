#ifndef __BTSTACK_MEMORY_VAL_H
#define __BTSTACK_MEMORY_VAL_H

#include "btstack_memory.h"
#include "btstack_memory_pool.h"

#include <stdlib.h>

//hci_connection_t hci_connection_storage[MAX_NR_HCI_CONNECTIONS];
extern btstack_memory_pool_t hci_connection_pool;

//gatt_client_t gatt_client_storage[MAX_NR_GATT_CLIENTS];
extern btstack_memory_pool_t gatt_client_pool;

//whitelist_entry_t whitelist_entry_storage[MAX_NR_WHITELIST_ENTRIES];
extern btstack_memory_pool_t whitelist_entry_pool;



//sm_lookup_entry_t sm_lookup_entry_storage[MAX_NR_SM_LOOKUP_ENTRIES];
extern btstack_memory_pool_t sm_lookup_entry_pool;


#endif //__BTSTACK_MEMORY_VAL_H
