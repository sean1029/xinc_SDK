/**************************************************************************
 * MODULE NAME:    lmp_con_filter.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    LMP Connection Filter
 *     
 * AUTHOR:         Gary Fleming
 * DATE:           22-12-1999
 *
 * SOURCE CONTROL: $Id: lmp_con_filter.c,v 1.36 2010/05/18 12:20:26 garyf Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *   All rights reserved.
 *
 * DESCRIPTION
 * This module is responsible for supporting and enforcing the event filters
 * in the link manager. It allows filters to be written and modified by the
 * the higher layers. This determines how the link manager will respond to 
 * incoming connections and inquiry results.
 *
 * CONDITIONAL
 * Module conditional upon PRH_BS_CFG_SYS_FILTERS_SUPPORTED
 *
 * RESTRICTIONS
 * Number of connection and inquiry filters must be same PRH_BS_CFG_SYS_MAX_FILTERS
 *************************************************************************/
#include "sys_config.h"

#if (PRH_BS_CFG_SYS_FILTERS_SUPPORTED == 1)

#include "lmp_types.h"
#include "lmp_utils.h"
#include "lmp_const.h"
#include "lmp_con_filter.h"
#include "lmp_config.h"

/*
 * Filter Condition Types
 */
#define ALL_DEVICES                     0x00
#define CLASS_DEVICE                    0x01
#define BD_ADDR                         0x02

/*
 * Filter Types
 */
#define CLEAR_ALL                       0x00
#define INQUIRY_RESULT                  0x01
#define CONNECTION_SETUP                0x02

typedef struct 
{
    t_filter   filter;
    u_int8     used;
} t_filter_entry;

static t_filter_entry inquiry_filters[PRH_BS_CFG_SYS_MAX_FILTERS];      /* Array of Inquiry Filters         */
static t_filter_entry connection_filters[PRH_BS_CFG_SYS_MAX_FILTERS];   /* Array of Connection Filters      */

static u_int8 num_inquiry_filters;                   /* Numer of active inquiry filters     */
static u_int8 num_connection_filters;                /* Numer of active connection filters  */

/*
 * Auxilliary functions
 */
static void LMfltr_Inquiry_Init(void);
static void LMfltr_Connection_Init(void);
static t_filter* LMfltr_Get_Next_Free_Filter(t_filter_entry* filters_array);

/************************************************************************
 *  Function :- LMfltr_Initialise()
 *
 *  Description
 *  Initialises the Link Manager Filters.
 *************************************************************************/
void LMfltr_Initialise(void)
{
    LMfltr_Connection_Init();
    LMfltr_Inquiry_Init();
}

/************************************************************************
 *  Function :- LMfltr_Connection_Init()
 *
 *  Description
 *  Initialises the Connection Filters
 *************************************************************************/
void LMfltr_Connection_Init(void)
{
    int i;

    for(i = 0; i < PRH_BS_CFG_SYS_MAX_FILTERS; i++)
    {
        connection_filters[i].used = 0;
    }
    num_connection_filters = 0;
}

/************************************************************************
 *  Function :- LMfltr_Inquiry_Init()
 *
 *  Description
 *  Initialise the Inquiry Filter.
 *************************************************************************/
void LMfltr_Inquiry_Init(void)
{
    int i;

    for(i = 0; i < PRH_BS_CFG_SYS_MAX_FILTERS; i++)
    {
        inquiry_filters[i].used = 0;
    }
    num_inquiry_filters = 0;
}

/************************************************************************
 *  Function :- LMfltr_LM_Set_Filter
 *
 *  Description
 *  Sets a Inquiry Result or Connection Setup filter. The "filter_type" identifies
 *  on of the following actions 
 *        
 *              0/  Clear All Filters
 *              1/  Create Inquiry Result Filter
 *              2/  Create Connection Setup Filter
 *
 *************************************************************************/
t_error LMfltr_LM_Set_Filter(u_int8 filter_type, t_filter* p_filter)
{
   t_filter* p_new_filter;
   t_error status = NO_ERROR;

   if(filter_type == CLEAR_ALL)
   {
       LMfltr_Initialise();
   }
   else if ((filter_type == INQUIRY_RESULT) || (filter_type == CONNECTION_SETUP))
   {
       /* Allocate a new filter */
       if (filter_type == INQUIRY_RESULT)
       {
           p_new_filter = LMfltr_Get_Next_Free_Filter(inquiry_filters);
       }
       else
       {
           p_new_filter = LMfltr_Get_Next_Free_Filter(connection_filters);
       }
       if(p_new_filter) /* A filter is available */
       {
           switch(p_filter->filter_condition_type)
           {
           case ALL_DEVICES :
               if (filter_type == INQUIRY_RESULT)
               {
                   LMfltr_Inquiry_Init();
                   return NO_ERROR; /* Immediate return to preclude the incrementing of the num filters */
               }
               break;

           case CLASS_DEVICE :
               p_new_filter->class_of_device = p_filter->class_of_device;
               p_new_filter->class_of_device_mask = p_filter->class_of_device_mask;
               break;

           case BD_ADDR :
               LMutils_Set_Bd_Addr(&p_new_filter->bd_addr,&p_filter->bd_addr);
               break;

           default :
               status = INVALID_HCI_PARAMETERS;
               break;
           }
           if (status == NO_ERROR)
           {
               if (filter_type == INQUIRY_RESULT)
               {
                   num_inquiry_filters++;
               }
               else
               {
                   p_new_filter->auto_accept = p_filter->auto_accept;
                   num_connection_filters++;
               }
               p_new_filter->filter_condition_type = p_filter->filter_condition_type;
           }
       }
       else
       {
           status = MEMORY_FULL;
       }
   }
   else
   {
       status = INVALID_HCI_PARAMETERS;
   }
   return status;
}

/************************************************************************
 *  Function :- LM_Connection_Filter_Check
 *
 *  Description
 *  Check the LMP Connection Filters for a match
 *
 *  Returns either
 *     REJECT
 *     DONT_AUTO_ACCEPT
 *     AUTO_ACCEPT
 *     AUTO_ACCEPT_WITH_MSS
 *     
 *************************************************************************/
u_int8 LMfltr_Connection_Filter_Check(t_bd_addr* p_bd_addr,
                                             t_classDevice device_class, 
											 t_linkType link_type)
{
   t_filter* p_current_filter;
   int i;

   if (num_connection_filters == 0)
   {
       return DONT_AUTO_ACCEPT;
   }
   
   /* Start from the most recent filter */
   i = PRH_BS_CFG_SYS_MAX_FILTERS;

   do
   {
       i--;

       if(connection_filters[i].used)
       {
           p_current_filter = &connection_filters[i].filter;
           switch(p_current_filter->filter_condition_type)
           {
           case ALL_DEVICES :
               return p_current_filter->auto_accept;
               break;
           case CLASS_DEVICE :
#if 1 // GF 12 May
			   if ((device_class == 0x000000) && (link_type != ACL_LINK))
			   {
					return AUTO_ACCEPT;
			   }
               else if ((device_class & p_current_filter->class_of_device_mask) ==
                   (p_current_filter->class_of_device & p_current_filter->class_of_device_mask)) 
#else
               if ((device_class & p_current_filter->class_of_device_mask) ==
                   (p_current_filter->class_of_device & p_current_filter->class_of_device_mask)) 
#endif
               {
                   return p_current_filter->auto_accept;
               }
               break;
           case BD_ADDR : /* If the Bd_Addr matches the Addr in the filter */
               if (LMutils_Bd_Addr_Match(p_bd_addr,&p_current_filter->bd_addr))
                   return p_current_filter->auto_accept;
               break;

           }
       }
    }
    while (i>0);
   return REJECT; 
}

/************************************************************************
 *
 *  Function :- LMfltr_Inquiry_Filter_Check
 *
 *  Description
 *  Check the LMP Inquiry Filters for a match
 *
 *  Returns either
 *     TRUE  
 *     FALSE
 *************************************************************************/
u_int8 LMfltr_Inquiry_Filter_Check(t_bd_addr* p_bd_addr,
                                             t_classDevice device_class)
{
   t_filter* p_current_filter;
   int i;

   /* 
    * No inquiry filters, then return match
    */
   if (num_inquiry_filters == 0)
   {
       return TRUE;
   }

   /* Start from the most recent filter */

   i = PRH_BS_CFG_SYS_MAX_FILTERS;
   do
   {  
       i--;

       if(inquiry_filters[i].used)
       {
           p_current_filter = &inquiry_filters[i].filter;
           switch(p_current_filter->filter_condition_type)
           {
           case ALL_DEVICES :
               return TRUE;
               break;
           case CLASS_DEVICE :
               if ((device_class & p_current_filter->class_of_device_mask) ==
                   (p_current_filter->class_of_device & p_current_filter->class_of_device_mask)) 
               {
                   return TRUE;
               }
               break;
           case BD_ADDR : /* If the Bd_Addr matches the Addr in the filter */
               if (LMutils_Bd_Addr_Match(p_bd_addr,&p_current_filter->bd_addr))
                   return TRUE;
               break;
           }
       }
    }
    while (i>0);

    return FALSE; 
}


/************************************************************************
 *
 *  Function :- LMfltr_Get_Next_Free_Filter
 *  Description
 *  Gets the next available filter.
 *************************************************************************/
static t_filter* LMfltr_Get_Next_Free_Filter(t_filter_entry* filters_array)
{
    int i = 0;
 
    for (i=0; i < PRH_BS_CFG_SYS_MAX_FILTERS; i++)
    {
       if(filters_array[i].used == 0)
       {
           filters_array[i].used = 1;
           return &filters_array[i].filter;
       }
    }
    return 0;
}
#else

/*
 * Completely empty source files are illegal in ANSI C
 */
void __lmp_con_filter_dummy(void) { }

#endif /*(PRH_BS_CFG_SYS_FILTERS_SUPPORTED == 1)*/

