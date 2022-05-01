 /*************************************************************************
 * MODULE NAME:    uslc_park_slave.c
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    Upper Software Link Controller Parl Slave statemachine.    
 * MAINTAINER:     Conor Morris
 *
 * SOURCE CONTROL: $Id: uslc_park_slave.c,v 1.35 2010/06/17 15:14:46 garyf Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * NOTES TO USERS:
 *    
 *************************************************************************/

#include "sys_config.h"
#include "lc_types.h"

#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
#include "lc_interface.h"
#include "lc_log.h"

#include "bt_timer.h"

#include "hw_lc.h"
#include "sys_mmi.h"
#include "hw_memcpy.h"

#include "dl_dev.h"

#include "lslc_pkt.h"
#include "lslc_hop.h"
#include "lslc_access.h"

#include "uslc_park_slave.h"
#include "uslc_chan_ctrl.h"

typedef struct t_parkSlave
{
    t_timer unpark_timer,
            t_access_timer,
            n_acc_slot_timer,
            access_window_end_timer,
            access_request_timer;

    u_int m_access;

    t_slots t_access,
            n_poll;
    
    t_ar_addr ar_addr;

    t_pm_addr pm_addr;

    t_am_addr am_addr;
    
    t_devicelink *p_active_device_link;

    t_deviceIndex device_index; /* Device to be parked */

    t_parkDeviceIndex park_device_index; /*Device to be un-parked */

    t_USLCparkSlave_ctrl_state ctrl_state;

    u_int n_acc_slot;

} t_parkSlave;

#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
/* deliberately not making volatile, since LMP cannot cancel
 * Park_Request */
t_parkSlave _parkSlave_info;
#endif


/*
 * Support function.
 */
void _Check_If_End_Of_Access_Window(void);


/******************************************************************
 *
 * USLCparkSlave_Park_Request
 *
 * Request from a higher layer to park a device.
 *
 ******************************************************************/
t_error USLCparkSlave_Park_Request(t_deviceIndex device_index, t_pm_addr pm_addr)
{
    t_error    status;
    t_parkSlave *parkSlave_info = &_parkSlave_info;

    LC_Log_Procedure_Request(PARKSLAVE_PARK_PROCEDURE);
 
    /*
     * Only activate procedure if currently not active and the procedure
     * parameterss are within the indicated bounds.
     */    
    if ((parkSlave_info->ctrl_state==OS_NOT_ACTIVE) && (device_index!=0) )    
    {

        LC_Log_Procedure_Accept(PARKSLAVE_PARK_PROCEDURE);

        parkSlave_info->p_active_device_link=DL_Get_Device_Ref(device_index);
        parkSlave_info->device_index = device_index;
        parkSlave_info->pm_addr = pm_addr;

        /*
         * Parking the Slave is handled like any other Chac procedure.
         * At the next Tim0 'assigned' to the Park_Slave object, the 
         * Slave device will be parked.  This has the advantage that the
         * device is parked in a determined manner.
         */

         parkSlave_info->ctrl_state = OS_PARK_START;
         USLCchac_Procedure_Request(PARKSLAVE_PARK_PROCEDURE, TRUE);        
         
         status = NO_ERROR;
     }
     else
     {
        status = UNSPECIFIED_ERROR;
     }

     return status;
}

/******************************************************************
 *
 * USLCparkSlave_Unpark_Request
 *
 * Request from a higher layer to unpark a device.
 *
 * Note:
 *  - In the case of Master-initiated unpark:
 *      - start_timer_ref: represents the bt_clk at the end of the 
 *                         beacon repetition period.
 *      - n_poll:          Not Applicable.
 *
 *  - In the case of Slave-initiated unpark:
 *      - start_timer_ref: Not Applicable.
 *
 * Assumption:
 *    - When a Slave-initiated unpark procedure is invoked it is 
 *      assumed that the USLCparkSlave object is currently monitoring
 *      the Access Window.  Thus, the 'start_timer_ref' is not required
 *      as the USLCparkSlave object can determine how long it 
 *      must listen for the the 1st POLL packet from the master:
 *         listen for the POLL pkt for: access_window_end_timer + n_poll
 *
 ******************************************************************/
t_error USLCparkSlave_Unpark_Request(t_deviceIndex* device_index,t_parkDeviceIndex park_device_index, 
                    t_unpark_type unpark_type, t_am_addr am_addr, t_clock start_timer_ref, t_slots n_poll)
{
    t_parkSlave *parkSlave_info = &_parkSlave_info;
    t_error    status = NO_ERROR;

    LC_Log_Procedure_Request(PARKSLAVE_PARK_PROCEDURE);
 
    /*
     * Only activate procedure if the parkSlave object is currently 
     * in the OS_NOT_ACTIVE state.
     */    
    if (parkSlave_info->ctrl_state == OS_NOT_ACTIVE) 
    {
        LC_Log_Procedure_Accept(PARKSLAVE_PARK_PROCEDURE);

        parkSlave_info->p_active_device_link = 
                            DL_Get_Device_Ref( (t_deviceIndex) park_device_index);
        parkSlave_info->park_device_index = park_device_index;
        parkSlave_info->am_addr = am_addr;
        parkSlave_info->n_poll = n_poll;  

       /*
        * For timeout value for receiving the POLL packet from the Master is: 
        *   - MASTER_INIT: End_of_Beacon_Repetition_Period + New_Connnetion_Timeout
        *                  n_poll is Not Applicable.
        * 
        *   - SLAVE_INIT: End_of_Last_Acess_Window + New_Connection_Timeout + n_poll
        *
        * In the Slave the Unpark_Request(..) is invoked at the following points:
        *   - MASTER_INIT: At the End_of_Beacon_Repetition_Period.  Thus, the device must
        *                  listen for the POLL for New_Connnetion_Timeout slots.
        *                  Note: 'start_timer_ref' indicates the number of slots until the 
        *                  end of the current Beacon_Repetition_Period.
        * 
        *   - SLAVE_INIT: At the End_of_Last_Acess_Window + n_poll.
        *                 Thus, the device must listen for the POLL for 
        *                 New_Connnetion_Timeout-start_timer_ref slots. 
        * 
        */
		

		if (unpark_type==UNPARK_SLAVE_INIT)
	        parkSlave_info->unpark_timer = BTtimer_Piconet_Set_Slots(
                        NEW_CONNECTION_TIMEOUT_DEFAULT-start_timer_ref );
		else
	        parkSlave_info->unpark_timer = BTtimer_Piconet_Set_Slots(
                        start_timer_ref + NEW_CONNECTION_TIMEOUT_DEFAULT);

        /*
         * Initialise the ctrl_state of the DeviceLink to osStart
         */
        parkSlave_info->ctrl_state = OS_UNPARK_START;
    
        USLCchac_Procedure_Request(PARKSLAVE_PARK_PROCEDURE, TRUE);
    }
    else
    {
       status = UNSPECIFIED_ERROR;
    }

    return status;
}



/******************************************************************
 *
 * USLCparkMaster_Access_Window_Request
 *
 * Request from a higher layer to create an Access Window.
 *
 * Assumption:
 *   The 'parked' Slave is synchronised to the channel
 *   (i.e. has already listened on the Beacon Train).
 *
 *****************************************************************/
t_error USLCparkSlave_Access_Window_Request(t_slots t_access, u_int m_access, 
                 u_int n_acc_slot, u_int8 n_poll, t_deviceIndex device_index, t_ar_addr ar_addr)
{
    t_parkSlave *parkSlave_info = &_parkSlave_info;
    t_error    status = NO_ERROR;


    LC_Log_Procedure_Request(PARKSLAVE_PARK_PROCEDURE);

    /*
     * Only activate procedure if the parkSlave object is currently 
     * in the OS_NOT_ACTIVE state.
     */    
    if (parkSlave_info->ctrl_state == OS_NOT_ACTIVE) 
    {
        LC_Log_Procedure_Accept(PARKSLAVE_PARK_PROCEDURE);

        parkSlave_info->t_access = t_access;

        parkSlave_info->m_access = m_access;

        parkSlave_info->n_acc_slot = n_acc_slot;

        parkSlave_info->n_poll = n_poll;

        parkSlave_info->p_active_device_link=DL_Get_Device_Ref(device_index);

        parkSlave_info->ar_addr = ar_addr;

        parkSlave_info->ctrl_state = OS_PARK_SLAVE_ACCESS_WINDOW_START;
        USLCchac_Procedure_Request(PARKSLAVE_PARK_PROCEDURE, TRUE);
    }
    else
    {
       status = UNSPECIFIED_ERROR;
    }

    return status;
}


/******************************************************************
 *
 * USLCparkSlave_Unpark_Commit
 *
 * Indication from the higher layer that the Unpark should be committed.
 *
 ******************************************************************/
t_error USLCparkSlave_Unpark_Commit(void)
{
    t_parkSlave *parkSlave_info = &_parkSlave_info;
    
    if (OS_W4_UNPARK_INSTRUCTION == parkSlave_info->ctrl_state)
    {
        parkSlave_info->ctrl_state = OS_END_UNPARK_SUCCESSFUL;
        return NO_ERROR;
    }
    else
        return UNSPECIFIED_ERROR;
}

/******************************************************************
 *
 * USLCparkSlave_Unpark_Rollback
 *
 * Indication from the higher layer that the Unpark should be committed.
 *
 ******************************************************************/
t_error USLCparkSlave_Unpark_Rollback(void)
{
    t_parkSlave *parkSlave_info = &_parkSlave_info;
    
    if (OS_W4_UNPARK_INSTRUCTION == parkSlave_info->ctrl_state)
    {
        parkSlave_info->ctrl_state = OS_END_UNPARK_UNSUCCESSFUL;
        return NO_ERROR;
    }
    else
        return UNSPECIFIED_ERROR;
}

/******************************************************************
 *
 * USLCparkSlave_Park
 *
 * 'One-shot' statemachine for the Park Slave object.
 *
 *****************************************************************/
void USLCparkSlave_Park(boolean dummy)
{

    t_packet event = INVALIDpkt;
    static t_LC_Event_Info event_info;
    t_parkSlave *parkSlave_info = &_parkSlave_info;
    t_TXRXstatus status = (t_TXRXstatus)NO_ERROR; //-  add (t_TXRXstatus) in order to avoid compiler warning.
    t_parkDeviceIndex park_device_index = 0;

    
    /*
     * If the parkSlave object is in a 'Start' state, then
     * notify the Chac object that the procedure has started.
     */
    if ( (parkSlave_info->ctrl_state >= OS_PARK_START) &&
         (parkSlave_info->ctrl_state <= OS_PARK_SLAVE_ACCESS_WINDOW_START) )
    {
        LC_Log_Procedure_Start(PARKSLAVE_PARK_PROCEDURE);
        USLCchac_Procedure_Start(PARKSLAVE_PARK_PROCEDURE, 
                          parkSlave_info->p_active_device_link);
    }

    /*
     * 'One-off' tasks associated with the parkSlave 'Start'
     * states. 
     */
    switch(parkSlave_info->ctrl_state)
    {
        case OS_UNPARK_START:
            /*
             * In order to unpark the Slave, the device super_state 
             * must be set to 'Connection'.
             */ 
            USLCchac_Set_Device_State(Connection);

            /*
             * For timeout value for receiving the POLL packet from the Master is: 
             *   - MASTER_INIT: End_of_Beacon_Repetition_Period + New_Connnetion_Timeout
             *   - SLAVE_INIT: End_of_Last_Acess_Window + New_Connection_Timeout + n_poll
             *
             * Thus, for Master initiated unpark, the n_poll value is not used.
             */
            parkSlave_info->device_index = DL_Unpark(parkSlave_info->park_device_index, 
                    MASTER /*role of peer*/, parkSlave_info->am_addr);
        
            parkSlave_info->ctrl_state = OS_UNPARK_RX;
            break;

        case OS_PARK_START: 
            park_device_index = 
                 DL_Park(parkSlave_info->device_index,parkSlave_info->pm_addr);
            parkSlave_info->ctrl_state = OS_PARK_END;
            break;

        case OS_PARK_SLAVE_ACCESS_WINDOW_START:
            /*
             * The Slave will only participate in the Access Window, iff, the
             * Slave wants to inform the Master that the Slave wishes to be
             * un-parked.
             */
            {
            parkSlave_info->access_window_end_timer = 
                 BTtimer_Piconet_Set_Slots( parkSlave_info->m_access*parkSlave_info->t_access);

            parkSlave_info->t_access_timer = 
                 BTtimer_Piconet_Set_Slots(parkSlave_info->t_access);

            /*
             *  AR_ADDR         acc_slot        timer_expire_in_slot
             *  -------         --------        --------------------
             *     1                1                   1
             *     2                1                   1
             *     3                2                   3
             *     4                2                   3
             *     5                3                   5
             *     6                3                   5
             *     ......
             *
             * Therefore :
             *  if (AR_ADDR==ODD)
             *      set timer for 'AR_ADDR' slots
             *  else
             *      set timer for 'AR_ADDR-1' slots
             */
            parkSlave_info->access_request_timer = 
                 BTtimer_Piconet_Set_Slots( (parkSlave_info->ar_addr & 0x01) ? 
                    parkSlave_info->ar_addr : parkSlave_info->ar_addr-1  );

            parkSlave_info->n_acc_slot_timer = 
                  BTtimer_Piconet_Set_Slots(parkSlave_info->n_acc_slot*2);

            }
            /*
             * In order to listen within the Access Window, the device super_state 
             * must be set to 'Connection'.
             */ 
            USLCchac_Set_Device_State(Connection);

            parkSlave_info->ctrl_state = OS_PARK_SLAVE_ACCESS_WINDOW_RX;         
            break;

        default:
            status = DL_Get_Rx_Status(parkSlave_info->p_active_device_link);
            if (RX_NO_PACKET != status)
            {
                event = DL_Get_Rx_Packet_Type(parkSlave_info->p_active_device_link);
            }
    }

    /* 
     * Park Slave Statemachine
     */
    switch(parkSlave_info->ctrl_state)
    {
        case OS_UNPARK_RX:
            /*
             * Ensure broadcast polls are not responded to (ie access window
             * may follow beacon train even after master-initiated unpark)
             * status is valid as is captured with event
             */
            if ( ((POLLpkt == event) || is_ACL_CRC_Packet(event)) && (RX_OK == status) /*&&
            DL_Get_Rx_Previous_Arqn(parkSlave_info->p_active_device_link)==NAK*/)
            {
                parkSlave_info->ctrl_state=OS_W4_UNPARK_INSTRUCTION;        
                LSLCpkt_Generate_NULL(parkSlave_info->p_active_device_link);
                event_info.status = NO_ERROR;
                event_info.deviceIndex = parkSlave_info->device_index;
                event_info.parkDeviceIndex = parkSlave_info->park_device_index;
                LC_Callback_Event_Handler(LC_EVENT_UNPARK_COMPLETE, &event_info);
            } 
            else if ( BTtimer_Piconet_if_Expired(parkSlave_info->unpark_timer))
            {
                parkSlave_info->ctrl_state = OS_END_UNPARK_UNSUCCESSFUL;
            }
            else
            {
                /* Stay in the same state */
            }
            break;

        case OS_W4_UNPARK_INSTRUCTION:
            /*
             * Wait for instructions from the higher layer, ie
             * Unpark_Commit or Unpark_Rollback.
             * Currently, in the LC there is no timer 'guarding' this
             * state.  It is assumed that the higher layer will have
             * a timer set.
             * If a 'guard' timer is ussed here, then when the timer
             * expires the following should occur:
             *   parkSlave_info->ctrl_state = OS_END_UNPARK_UNSUCCESSFUL;
             */
            break;


        case OS_PARK_SLAVE_ACCESS_WINDOW_RX:
            /*
             * Can only send an Access Request in the next Tx slot,
             * iff a valid boradcast packet is received.
             */
            if ( BTtimer_Piconet_if_Expired(parkSlave_info->access_request_timer))
            {
                if ((RX_NO_PACKET != status) && (INVALIDpkt != event))
                {                  
                    /*
                     * Must determine if the Access Request (IDpkt) 
                     * should be sent in the 1st or 2nd half of the
                     * Tx slot.
                     */
                    LSLCpkt_Generate_ID(parkSlave_info->p_active_device_link, 
                       (t_state)((parkSlave_info->ar_addr & 0x01) ? 
                       Connection : 
                       Connection_2ndHalf_Slot_Tx));
                }
                _Check_If_End_Of_Access_Window();
            }
            else
            {
                parkSlave_info->ctrl_state = OS_PARK_SLAVE_ACCESS_WINDOW_RX;               
            }
            break;

        case OS_PARK_SLAVE_W4_ACCESS_WINDOW_END:
            /*
             * Transition to next state is performed within the function.
             */
            _Check_If_End_Of_Access_Window();
            break;
        
        case OS_PARK_SLAVE_ACCESS_WINDOW_W4_UNPARK_MSG:
            if ( BTtimer_Piconet_if_Expired(parkSlave_info->access_window_end_timer))
                parkSlave_info->ctrl_state=OS_PARK_SLAVE_ACCESS_WINDOW_END;
            break;

        default:
            break;
    }


    switch (parkSlave_info->ctrl_state)
    {
        case OS_PARK_END:
            /*
             * Callback to the higher layer to indicate that the 
             * device has been parked.
             * Specify the park device_link assigned to the parked device.
             */
            event_info.status = NO_ERROR;
            event_info.parkDeviceIndex = park_device_index;
            event_info.deviceIndex = parkSlave_info->device_index;
            LC_Callback_Event_Handler(LC_EVENT_PARK_COMPLETE,&event_info);

            /*
             * Indicate to the USLCchac that the Park Slave procedure 
             * is finished, and indicate that the super_state is not
             * to be modified. 
             */
            USLCchac_Procedure_Finish(FALSE/*Do not return to prev super_state*/);

            parkSlave_info->ctrl_state=OS_NOT_ACTIVE;
            break;

        case OS_END_UNPARK_SUCCESSFUL:
            /* 
             * Unpark has been successful.
             * Commit the 'un_parked' device_link.
             */
            DL_Unpark_Commit(parkSlave_info->park_device_index, MASTER /*role of peer*/);        
          
            /*
             * Indicate to the USLCchac that the Park Slave procedure 
             * is finished, and indicate that the super_state is not
             * to be modified. 
             */
            USLCchac_Procedure_Finish(FALSE /* Do not return to 
                                                prev super_state*/);

            parkSlave_info->ctrl_state=OS_NOT_ACTIVE;
            break;

        case OS_END_UNPARK_UNSUCCESSFUL:
              /*
               * DL already unparked just before entering the OS_UNPARK_START
               * state. Unpark must be 'rolled-back'.
               */
               DL_Unpark_Rollback(parkSlave_info->device_index);

               event_info.status = UNSPECIFIED_ERROR;
               event_info.parkDeviceIndex = parkSlave_info->park_device_index;
               LC_Callback_Event_Handler(LC_EVENT_UNPARK_COMPLETE,&event_info);

               /*
                * Indicate to the USLCchac that the Park Slave procedure 
                * is finished, and indicate to set 
                * super_state=previous_super_state
                */

               USLCchac_Procedure_Finish(TRUE/*Return to prev super_state*/);
               parkSlave_info->ctrl_state=OS_NOT_ACTIVE;
            break;

        case OS_PARK_SLAVE_ACCESS_WINDOW_END:
            event_info.status = NO_ERROR;
            event_info.parkDeviceIndex = parkSlave_info->park_device_index;
            LC_Callback_Event_Handler(LC_EVENT_ACCESS_WINDOW_COMPLETE,&event_info);

            USLCchac_Procedure_Finish(TRUE/*Return to prev super_state*/);
            parkSlave_info->ctrl_state=OS_NOT_ACTIVE;
            break;

        default:
            break;
    }
}

/******************************************************************
 *
 * void USLCparkSlave_Initialise(void)
 *
 ******************************************************************/

void USLCparkSlave_Initialise(void)
{
    t_parkSlave *parkSlave_info = &_parkSlave_info;

    /*
     * Initialise defaults for Park Slave
     */

    parkSlave_info->ctrl_state = OS_NOT_ACTIVE;

}



/******************************************************************
 *
 * void _Check_If_End_Of_Access_Window(void)
 *
 ******************************************************************/
void _Check_If_End_Of_Access_Window(void)
{
    t_parkSlave *parkSlave_info = &_parkSlave_info;

    /* Check if End of current Access Window */
    if ( BTtimer_Piconet_if_Expired(parkSlave_info->t_access_timer))
    {
        /* Check if last Access Window */
        if ( BTtimer_Piconet_if_Expired(parkSlave_info->access_window_end_timer))
        {
            /*
             * Reached the end of the last Access Window.
             * Must listen for another 'n_poll' slots for an Unpark
             * message from the Master.
             * 'access_window_end_timer' will be extended by 'n_poll' slots.
             */
            parkSlave_info->access_window_end_timer = BTtimer_Piconet_Extend(
            parkSlave_info->access_window_end_timer,  parkSlave_info->n_poll);
            parkSlave_info->ctrl_state = OS_PARK_SLAVE_ACCESS_WINDOW_W4_UNPARK_MSG;
        }
        else /* access_window_end_timer NOT expired */
        {
            parkSlave_info->t_access_timer =
				BTtimer_Piconet_Reset_Slots(parkSlave_info->t_access_timer,
				parkSlave_info->t_access);

            parkSlave_info->access_request_timer = 
				BTtimer_Piconet_Reset_Slots(parkSlave_info->access_request_timer,
				parkSlave_info->t_access);

            parkSlave_info->n_acc_slot_timer = 
                BTtimer_Piconet_Set_Slots(parkSlave_info->n_acc_slot*2);
                    
            parkSlave_info->ctrl_state = OS_PARK_SLAVE_ACCESS_WINDOW_RX;                           
        }
    }
    else/* t_access_timer */
    {
        parkSlave_info->ctrl_state = OS_PARK_SLAVE_W4_ACCESS_WINDOW_END;               
    }
}

/*
 * Test support functions
 */ 
t_USLCparkSlave_ctrl_state USLCparkSlave_Get_Ctrl_State(void)
{
    return _parkSlave_info.ctrl_state;
}

#else /* (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==0)*/
/* completely empty C files are illegal in ANSI */
void USLCparkSlave_Dummy__(void) { }
#endif /* PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED */
