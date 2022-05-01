/**************************************************************************
 * MODULE NAME:    lmp_link_key_db.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    Link Key Database
 * AUTHOR:         Gary Fleming
 * DATE:           20-12-1999
 *
 * SOURCE CONTROL: $Id: lmp_link_key_db.c,v 1.28 2010/05/11 14:34:14 nicolal Exp $

 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *    
 * NOTES
 * This object is responsible for maintaining and controlling the link
 * key database in the host controller.
 *
 *******************************************************************/

#include "sys_types.h"
#include "sys_config.h"
#include "lmp_types.h"
#include "lmp_config.h"
#include "lmp_acl_container.h"
#include "lmp_link_key_db.h"
#include "lmp_const.h"
#include "lmp_utils.h"
#include "hc_event_gen.h"
#include "hc_const.h"

/*****************************************
 * Structure of the DB entry
 *****************************************/

typedef struct {
    u_int8 active;
    u_int8 link_key[16];
    t_bd_addr   bd_addr;
} t_link_key_entry;


/*****************************************
 * Local Funtions
 *****************************************/

#if (PRH_BS_CFG_SYS_LINK_KEY_DATABASE_SUPPORTED==1)
static void _Set_DB_Entry_Link_Key(t_link_key_entry* entry, t_link_key key);
static u_int8 _DB_Delete_All_Link_Keys(void);
static u_int8 _DB_Read_All_Keys(void);
static t_error _DB_Delete_Link_Key(t_bd_addr* p_bd_addr);

/*****************************************
 * Local Funtions to generate events to the
 * higher layers.
 *****************************************/

static void _Send_HC_Return_Link_Key_Event(t_bd_addr* p_bd_addr,
                                           u_int8* p_link_key,
                                           u_int8 num_keys);


/*****************************************
 * Local Static Data. 
 *****************************************/
static t_link_key_entry link_key_database[PRH_BS_CFG_SYS_MAX_STORED_LINK_KEYS];
static u_int8 num_link_keys;
#endif


/*********************************************************
 * Function : LMkeydb_Initialise
 *
 * Description :
 * Initialses the Link Key Database object. All entries are
 * set to inactive and the call back event is initialised.
 *
 ********************************************************/

void LMkeydb_Initialise()
{
#if (PRH_BS_CFG_SYS_LINK_KEY_DATABASE_SUPPORTED==1)
    u_int8 db_index = 0;

    num_link_keys = 0;

    /* Initialise the Link Key DB */
    while(db_index < PRH_BS_CFG_SYS_MAX_STORED_LINK_KEYS)
    {
        link_key_database[db_index].active= 0;
        db_index++;
    }
#endif
}

/*********************************************************
 *
 * Function : LMkeydb_LM_Read_Stored_Link_Key
 *
 * Description :
 * Actions dependent on the read_all_flag parameter. 
 * if the "read_all_flag" is not set then the link key
 * entry for a given BD_ADDR is read, and returned in the
 * Return Link Key event.
 *   
 * If the "read_all_flag" is set then each link key is the
 * DB is returned individually using the Return Link Key Event
 * 
 *
 * Only one key is returned in each Return Link Key Event.
 *
 ********************************************************/

void LMkeydb_LM_Read_Stored_Link_Key(t_bd_addr* p_bd_addr, u_int8 read_all_flag,
                                     t_cmd_complete_event* p_cmd_complete_info)
{
#if (PRH_BS_CFG_SYS_LINK_KEY_DATABASE_SUPPORTED==1)
    u_int8* p_link_key;  
    u_int16  num_keys_read=0;
	t_error status = NO_ERROR;

	if(read_all_flag > 1)
    {
		status = INVALID_HCI_PARAMETERS;
    }
    else if(read_all_flag)
    {
        num_keys_read = _DB_Read_All_Keys();
    }
    else
    {
        p_link_key = LMkeydb_Read_Link_Key(p_bd_addr);
        if(p_link_key)
        {
            /* Raise An Event to indicate Link Key Read */
            _Send_HC_Return_Link_Key_Event(p_bd_addr,p_link_key,0x01); 
            num_keys_read++;
        }
    }
    p_cmd_complete_info->returnParams.readStoredLinkKey.max_num_keys = PRH_BS_CFG_SYS_MAX_STORED_LINK_KEYS;
    p_cmd_complete_info->returnParams.readStoredLinkKey.num_keys_read = num_keys_read;
    p_cmd_complete_info->status = status;

#endif
}

/*********************************************************
 *
 * Function : LMkeydb_LM_Write_Stored_Link_Key
 *
 * Description :
 * Write a link key and the associated BD_ADDR in the the 
 * link key DB. 
 * 
 ********************************************************/
void LMkeydb_LM_Write_Stored_Link_Key(t_bd_addr* p_bd_addr,
                                      u_int8* link_key,
                                      t_cmd_complete_event* p_cmd_complete_info)
{
#if (PRH_BS_CFG_SYS_LINK_KEY_DATABASE_SUPPORTED==1)

    if(LMkeydb_Write_Link_Key(p_bd_addr,link_key) == NO_ERROR)
    {
        p_cmd_complete_info->number = 1; 
    }
    else  
    {
        p_cmd_complete_info->status = NO_ERROR;
        p_cmd_complete_info->number = 0;  
    }
#endif
}

/*********************************************************
 *
 * Function : LMkeydb_LM_Delete_Stored_Link_Key
 *
 * Description :
 * Actions dependent on the delete_all_flag parameter. 
 * if the "delete_all_flag" is not set then the link key
 * entry for a given BD_ADDR is deleted.
 *
 * if the "delete_all_flag" is set then all entries in the 
 * DB are cleared.
 ********************************************************/
void LMkeydb_LM_Delete_Stored_Link_Key(t_bd_addr* p_bd_addr,
                                       u_int8 delete_all_flag,
                                       t_cmd_complete_event* p_event)
{
#if (PRH_BS_CFG_SYS_LINK_KEY_DATABASE_SUPPORTED==1)

	t_lmp_link *p_link;
	u_int16 i;
	
    /* Disable the "link_key_exists" flag of the t_lmp_link structure */
	if(delete_all_flag)
	{
		for(i=0; i< PRH_MAX_ACL_LINKS; i++)
		{
			p_link = LMaclctr_Find_Handle((u_int16)(i+PRH_ACL_CONNECTION_HANDLE_OFFSET));
            if (p_link != 0)
			   p_link->link_key_exists = FALSE;
		}
	}
	else
	{
		p_link = LMaclctr_Find_Bd_Addr(p_bd_addr);
		if(p_link)
			p_link->link_key_exists = FALSE;
	}
	
    if (delete_all_flag) 
    {
        p_event->value16bit = _DB_Delete_All_Link_Keys();
    }
    else
    {
        p_event->value16bit = (NO_ERROR == _DB_Delete_Link_Key(p_bd_addr));
    } 
    p_event->status = NO_ERROR;        
#endif
}

/*********************************************************
 *
 * Function : LMkeydb_Read_Link_Key
 *
 * Description :
 * Determines if the link key database contains an entry
 * for a given BD_ADDR. If a link key exists the for the 
 * BD_ADDR, it is returned. If no link key exists for the 
 * BD_ADDR a 0 is returned
 ********************************************************/
u_int8* LMkeydb_Read_Link_Key(t_bd_addr* p_bd_addr)
{
#if (PRH_BS_CFG_SYS_LINK_KEY_DATABASE_SUPPORTED==1)

    u_int8  db_index = 0;
    t_link_key_entry* p_entry;
 
    while(db_index < PRH_BS_CFG_SYS_MAX_STORED_LINK_KEYS)
    {
        p_entry = &link_key_database[db_index];
        if((p_entry->active) && (LMutils_Bd_Addr_Match(&p_entry->bd_addr,p_bd_addr)))
        {
           return p_entry->link_key;
        }
        db_index++;
    }
#endif
    return 0;
}

/*********************************************************
 *
 * Function : LMkeydb_Write_Link_Key
 *
 * Description :
 * Writes a new link key (& Bd_ADDR) into the first available
 * space in the link key DB.
 * NOTE :- ( Chap H:1 Section 6.9 )
 * "Note that for the Write_Stored_Link_Key command, no error is returned when the Host Control-ler
  *  can not store any more link keys. The Host Controller stores as many link
  * keys as there is free memory to store in, and the Host is notified of how many
  * link keys were successfully stored."
 ********************************************************/
t_error LMkeydb_Write_Link_Key(t_bd_addr* p_bd_addr,t_link_key link_key)
{
#if (PRH_BS_CFG_SYS_LINK_KEY_DATABASE_SUPPORTED==1)
    u_int8  db_index=0;
    t_link_key_entry* p_entry;
 
    /* First Search the DB for an existing entry for this BD_ADDR */
    while(db_index < PRH_BS_CFG_SYS_MAX_STORED_LINK_KEYS)
    {
        p_entry = &link_key_database[db_index];
        if((p_entry->active) && (LMutils_Bd_Addr_Match(&p_entry->bd_addr,p_bd_addr)))
        {
            _Set_DB_Entry_Link_Key(p_entry,link_key);
            return NO_ERROR;
        }
        else
            db_index++;
    }

    /* If the DB did not contain an entry for this link key already */
    db_index=0;
    while(db_index < PRH_BS_CFG_SYS_MAX_STORED_LINK_KEYS)
    {
        p_entry = &link_key_database[db_index];
        if(!p_entry->active)
        {
            _Set_DB_Entry_Link_Key(p_entry,link_key);
             LMutils_Set_Bd_Addr(&p_entry->bd_addr,p_bd_addr);

            p_entry->active = 1;
            num_link_keys++;
            return NO_ERROR;
        }
        else
        {
            db_index++;
        }
    }
    return MEMORY_FULL;
#else
    return UNSUPPORTED_FEATURE;
#endif

}

/*********************************************************
 *
 * Function : _Send_HC_Return_Link_Key_Event
 *
 * Description :
 * Generates a Return Link Key Event containing one Link Key
 * to the higher layers.
 ********************************************************/
void _Send_HC_Return_Link_Key_Event(
    t_bd_addr* p_bd_addr,u_int8* p_link_key,u_int8 num_keys) 
{
#if (PRH_BS_CFG_SYS_LINK_KEY_DATABASE_SUPPORTED==1)

    t_lm_event_info lm_return_lk_event;

    lm_return_lk_event.number = num_keys;
    lm_return_lk_event.p_bd_addr = p_bd_addr;
    lm_return_lk_event.p_u_int8 = p_link_key;
    g_LM_config_info.lmp_event[_LM_RETURN_LINK_KEY_DB_EVENT]
        (HCI_RETURN_LINK_KEYS_EVENT, &lm_return_lk_event);
#endif
}

/*********************************************************
 *
 * Function : _DB_Delete_Link_Key
 *
 * Description :
 * Deletes the link key for a given BD_ADDR.
 ********************************************************/
t_error _DB_Delete_Link_Key(t_bd_addr* p_bd_addr)
{
#if (PRH_BS_CFG_SYS_LINK_KEY_DATABASE_SUPPORTED==1)
    u_int8  db_index=0;
    t_link_key_entry* p_entry;

    while(db_index < PRH_BS_CFG_SYS_MAX_STORED_LINK_KEYS)
    {
        p_entry = &link_key_database[db_index];
        if((p_entry->active) && LMutils_Bd_Addr_Match(&p_entry->bd_addr,p_bd_addr))
        {
            p_entry->active =0;
            num_link_keys--;
            return NO_ERROR;
        }
        else
        {
            db_index++;
        }
    }
#endif

    return UNSPECIFIED_ERROR;
}

/*********************************************************
 *
 * Function : _DB_Delete_Link_Key
 *
 * Description :
 * Deletes the link key for a given BD_ADDR.
 ********************************************************/
u_int8 _DB_Delete_All_Link_Keys(void)
{
#if (PRH_BS_CFG_SYS_LINK_KEY_DATABASE_SUPPORTED==1)
    u_int8  db_index=0;
    u_int8  num_keys_deleted=0;

    while(db_index < PRH_BS_CFG_SYS_MAX_STORED_LINK_KEYS)
    {
        if(link_key_database[db_index].active == 1)
        {
            link_key_database[db_index].active = 0;
            num_keys_deleted++;
        }
        db_index++;
    }
    num_link_keys = 0;
    return num_keys_deleted;
#else
    return 0;
#endif
}


/*********************************************************
 * Function : _DB_Read_All_Keys
 *
 * Description :
 * Traverses the link key DB and generates a 
 * Return_Link_Key_Event for each active entry in the DB.
 *
 ********************************************************/
u_int8 _DB_Read_All_Keys(void)
{
#if (PRH_BS_CFG_SYS_LINK_KEY_DATABASE_SUPPORTED==1)

    u_int8  db_index=0;
    u_int8  num_keys_read=0;
    t_link_key_entry* p_entry;

    while(db_index < PRH_BS_CFG_SYS_MAX_STORED_LINK_KEYS)
    {
        p_entry = &link_key_database[db_index];
        if(p_entry->active)
        {
            _Send_HC_Return_Link_Key_Event(
                &p_entry->bd_addr, p_entry->link_key,0x01);
            num_keys_read++;
        }
        db_index++;
    }
    return num_keys_read;
#else
    return 0;
#endif
}

/********************************************************
 *
 * Function : _Set_DB_Entry_Link_Key
 *
 * Description :
 * Sets link key for a given DB Entry.
 ********************************************************/
void _Set_DB_Entry_Link_Key(t_link_key_entry* p_entry, t_link_key key)
{
#if (PRH_BS_CFG_SYS_LINK_KEY_DATABASE_SUPPORTED==1)

    u_int8 i=0;
    do
    {
        p_entry->link_key[i] = key[i];
        i++;
    } 
    while (i < LINK_KEY_SIZE);
#endif
}
