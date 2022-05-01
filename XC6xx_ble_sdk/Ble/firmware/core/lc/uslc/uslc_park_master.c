 /*************************************************************************
 * MODULE NAME:    uslc_park_master.c
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    Upper Software Link Controller Park Master statemachine.    
 * MAINTAINER:     Conor Morris
 *
 * SOURCE CONTROL: $Id: uslc_park_master.c,v 1.43 2012/09/28 11:49:21 tomk Exp $
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
#include "lc_log.h"
#include "lc_interface.h"

#include "bt_timer.h"

#include "hw_lc.h"
#include "sys_mmi.h"
#include "hw_memcpy.h"

#include "dl_dev.h"

#include "lslc_pkt.h"
#include "lslc_access.h"

#include "uslc_park_master.h"
#include "uslc_chan_ctrl.h"

#if ( PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED == 1)

typedef struct t_parkMaster
{
    t_timer unpark_timer,
            n_acc_slot_timer;

    t_pm_addr pm_addr;

    t_am_addr am_addr;

    t_devicelink *p_active_device_link;

    t_deviceIndex device_index;

    t_parkDeviceIndex park_device_index;

    u_int8 n_acc_slot;

    t_clock access_window_start;

    t_USLCparkMaster_ctrl_state ctrl_state;

    t_callback access_window_callback;
    
#if (PRH_BS_DEV_TIM3_IDENTIFIES_AR_ADDR_HALF_SLOT==1)
    u_int8 half_slot;
#endif

} t_parkMaster;

#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
/* deliberately not making volatile, since LMP cannot cancel
 * Park_Request */
t_parkMaster _parkMaster_info;
#endif

/******************************************************************
 *
 * USLCparkSlave_Park_Request
 *
 * Request from a higher layer to park a device.
 *
 ******************************************************************/
t_error USLCparkMaster_Park_Request(t_deviceIndex device_index, t_pm_addr pm_addr)
{
    static t_LC_Event_Info event_info;

    /*
     * Callback to the higher layer to indicate that the 
     * device has been parked.
     */
     event_info.parkDeviceIndex = DL_Park(device_index, pm_addr);
     event_info.deviceIndex = device_index;
     event_info.status = NO_ERROR;
     LC_Callback_Event_Handler(LC_EVENT_PARK_COMPLETE,&event_info);
     return NO_ERROR;
}


/******************************************************************
 *
 * USLCparkMaster_Unpark_Request
 *
 * Request from a higher layer to unpark a device.
 *
 ******************************************************************/
t_error USLCparkMaster_Unpark_Request(t_deviceIndex* device_index, t_parkDeviceIndex park_device_index, 
                    t_unpark_type unpark_type, t_am_addr am_addr,
                    t_slots end_of_beacon_train, t_slots n_poll)
{
    t_parkMaster *parkMaster_info = &_parkMaster_info;
    t_error    status = NO_ERROR;

    LC_Log_Procedure_Request(PARKMASTER_PARK_PROCEDURE);
 
    /*
     * Only activate procedure if currently not active and the procedure
     * parameterss are within the indicated bounds.
     */    
    if (parkMaster_info->ctrl_state==OS_NOT_ACTIVE_)    
    {
        LC_Log_Procedure_Accept(PARKMASTER_PARK_PROCEDURE);

        parkMaster_info->park_device_index = park_device_index;
        parkMaster_info->am_addr = am_addr;

        parkMaster_info->device_index = DL_Unpark(parkMaster_info->park_device_index, 
                                SLAVE /*role of peer*/, parkMaster_info->am_addr);
        parkMaster_info->p_active_device_link = 
                        DL_Get_Device_Ref( parkMaster_info->device_index );

        *device_index = parkMaster_info->device_index;

        /*
         * For timeout value for receiving the POLL packet from the Master is: 
         *   - MASTER_INIT: End_of_Beacon_Repetition_Period + New_Connnetion_Timeout
         *   - SLAVE_INIT: End_of_Last_Acess_Window + New_Connection_Timeout + n_poll
         *
         * Thus, for Master initiated unpark, the n_poll value is not used.
         */
        if (UNPARK_MASTER_INIT==unpark_type)
        {
            n_poll=0;
        }
        parkMaster_info->unpark_timer = BTtimer_Set_Slots(
            end_of_beacon_train + n_poll + NEW_CONNECTION_TIMEOUT_DEFAULT);

        /*
         * Initialise the ctrl_state of the DeviceLink to osStart
         */
         parkMaster_info->ctrl_state = OS_UNPARK_START_;
         USLCchac_Procedure_Request(PARKMASTER_PARK_PROCEDURE, TRUE);
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
 *****************************************************************/
t_error USLCparkMaster_Access_Window_Request(u_int n_acc_slot)
{
    t_parkMaster	*parkMaster_info = &_parkMaster_info;
    t_error			status = NO_ERROR;


    LC_Log_Procedure_Request(PARKMASTER_PARK_PROCEDURE);

    /*
     * Only activate procedure if the parkMaster object is currently 
     * in the OS_NOT_ACTIVE state.
     */    
    if (parkMaster_info->ctrl_state == OS_NOT_ACTIVE_) 
    {
        LC_Log_Procedure_Accept(PARKMASTER_PARK_PROCEDURE);

        parkMaster_info->p_active_device_link=DL_Get_Local_Device_Ref();

#if (PRH_BS_DEV_FORCE_MINIMUM_N_ACCESS == 1)
        parkMaster_info->n_acc_slot = 2;
#else
        parkMaster_info->n_acc_slot = n_acc_slot;
#endif

        parkMaster_info->ctrl_state = OS_PARK_MASTER_ACCESS_WINDOW_START;
        USLCchac_Procedure_Request(PARKMASTER_PARK_PROCEDURE, TRUE);
    }
    else
    {
       status = UNSPECIFIED_ERROR;
    }

    return status;
}


/******************************************************************
 *
 * USLCparkMaster_Unpark_Commit
 *
 * Indication from the higher layer that the Unpark should be committed.
 *
 ******************************************************************/
t_error USLCparkMaster_Unpark_Commit(void)
{
    t_parkMaster *parkMaster_info = &_parkMaster_info;
    
    if ( (OS_UNPARK_RX_ == parkMaster_info->ctrl_state) ||
         (OS_UNPARK_TX == parkMaster_info->ctrl_state) )
    {
        parkMaster_info->ctrl_state = OS_END_UNPARK_SUCCESSFUL_;
        return NO_ERROR;
    }
    else
        return UNSPECIFIED_ERROR;
}

/******************************************************************
 *
 * USLCparkMaster_Unpark_Rollback
 *
 * Indication from the higher layer that the Unpark should be committed.
 *
 ******************************************************************/
t_error USLCparkMaster_Unpark_Rollback(void)
{
    t_parkMaster *parkMaster_info = &_parkMaster_info;
    
    if ( (OS_UNPARK_RX_ == parkMaster_info->ctrl_state) ||
         (OS_UNPARK_TX == parkMaster_info->ctrl_state) )
    {
        parkMaster_info->ctrl_state = OS_END_UNPARK_UNSUCCESSFUL_;
        return NO_ERROR;
    }
    else
        return UNSPECIFIED_ERROR;
}

/******************************************************************
 *
 * USLCparkMaster_Park
 *
 * 'One-shot' statemachine for the Park Slave object.
 *
 *****************************************************************/
void USLCparkMaster_Park(boolean look_ahead_to_next_frame)
{

    t_packet event = INVALIDpkt;
    static t_LC_Event_Info event_info;
    t_parkMaster *parkMaster_info = &_parkMaster_info;
    t_TXRXstatus status;
    t_parkDeviceIndex park_device_index = 0;
    t_clock clock_when_id_pkt_recvd;
    t_devicelink *pDL=0;


    /*
     * Look_ahead only relevant for the following state (all other
     * states immediately return):
     *        OS_PARK_MASTER_ACCESS_WINDOW_RX
     */
    if (   ( (look_ahead_to_next_frame==LOOK_AHEAD) && (parkMaster_info->ctrl_state!=OS_PARK_MASTER_ACCESS_WINDOW_RX) ) ||
           ( (look_ahead_to_next_frame==NFA_CHAN_MASTER) && (parkMaster_info->ctrl_state==OS_PARK_MASTER_ACCESS_WINDOW_RX)) )
        return;

    /*
     * If the parkMaster object is in a 'Start' state, then
     * notify the Chac object that the procedure has started.
     */
    if ( (parkMaster_info->ctrl_state >= OS_PARK_START_) &&
         (parkMaster_info->ctrl_state <= OS_PARK_MASTER_ACCESS_WINDOW_START) )
    {
        LC_Log_Procedure_Start(PARKMASTER_PARK_PROCEDURE);
        USLCchac_Procedure_Start(PARKMASTER_PARK_PROCEDURE, 
                          parkMaster_info->p_active_device_link);
    }

    /*
     * 'One-off' tasks associated with the parkMaster 'Start'
     * states. 
     */
    switch(parkMaster_info->ctrl_state)
    {
        case OS_PARK_START_:          
            park_device_index = 
                DL_Park(parkMaster_info->device_index, 
                                            parkMaster_info->pm_addr);
            parkMaster_info->ctrl_state = OS_PARK_END_;
            break;

        case OS_UNPARK_START_: 
            /*
             * For timeout value for receiving the POLL packet from the Master is: 
             *   - MASTER_INIT: End_of_Beacon_Repetition_Period + New_Connnetion_Timeout
             *   - SLAVE_INIT: End_of_Last_Acess_Window + New_Connection_Timeout + n_poll
             *
             * Thus, for Master initiated unpark, the n_poll value is not used.
             */
            
            parkMaster_info->ctrl_state = OS_UNPARK_TX;
            break;

        case OS_PARK_MASTER_ACCESS_WINDOW_START:
            /*
             *  acc_slot        timer_expire_in_slot
             *  --------        --------------------
             *     1                   1
             *     2                   1
             *     3                   3
             *     4                   3
             *     5                   5
             *     6                   5
             *     ......
             *
             * Therefore :
             *  if (AR_ADDR==ODD)
             *      set timer for 'AR_ADDR' slots
             *  else
             *      set timer for 'AR_ADDR-1' slots
             */
            parkMaster_info->n_acc_slot_timer = 
                 BTtimer_Set_Slots( (parkMaster_info->n_acc_slot & 0x01) ? 
                    parkMaster_info->n_acc_slot : parkMaster_info->n_acc_slot-1  );
            
            /* Access Window will start in the next Tx slot */
            parkMaster_info->access_window_start = (HW_get_bt_clk() + 4) & 0x0FFFFFFC;
            parkMaster_info->ctrl_state = OS_PARK_MASTER_ACCESS_WINDOW_TX;
            break;

        default:
            status = DL_Get_Rx_Status(parkMaster_info->p_active_device_link);
            if (RX_NO_PACKET != status)
            {
                event = DL_Get_Rx_Packet_Type(parkMaster_info->p_active_device_link);
            }
            break;
    } /* switch(parkMaster_info->ctrl_state) */




    /* 
     * Park Master Statemachine
     */
    switch(parkMaster_info->ctrl_state)
    {
        case OS_UNPARK_TX:
            /*
             * Master must POLL the Slave, i.e. grant the Slave access to the 
             * channel. The unparked slave must send a response with LMP_accepted.
             */
            LSLCpkt_Generate_POLL(parkMaster_info->p_active_device_link);
            parkMaster_info->ctrl_state = OS_UNPARK_RX_;
            break;

            case OS_UNPARK_RX_:
                /*
                 * In this state a DM1 packet may have been received from Slave.
                 * Contents of the DM1 will be decoded by the higher layer.
                 * The contents of the DM1 packet will determine outcome of the 
                 * Unpark procedure :
                 *    LMP_accepted: Unpark procedure completed successfully.
                 *    LMP_not_accepted: Unpark procedure completed unsuccessfully.
                 *
                 * Even if a DM1 packet has been received from the Slave, continue
                 * polling the device until the unpark_timer expires.
                 */
                 if ( BTtimer_Is_Expired(parkMaster_info->unpark_timer))
                 {
                    parkMaster_info->ctrl_state = OS_END_UNPARK_UNSUCCESSFUL_;
                 }
                 else
                 {
                    if ((event == NULLpkt) || (event == DM1))
                        parkMaster_info->ctrl_state = OS_END_UNPARK_SUCCESSFUL_;
                    else
                        parkMaster_info->ctrl_state = OS_UNPARK_TX;
             }
             break;

        case OS_PARK_MASTER_ACCESS_WINDOW_TX:
            {
                /*
                 * At this point must determine the packet to be
                 * sent on the channel.
                 * According to the Spec (v1.1, pg115):
                 * '.. the slots of the access window can also be used for 
                 * traffic on the piconet if required. For example, if an 
                 * SCO connection has to be supported, the slots reserved 
                 * for the SCO link may carry SCO information instead of being
                 * used for access requests, i.e. if the master-to-slave slot 
                 * in the access window contains a packet different from a 
                 * broadcast packet, the following slave-to-master
                 * slot cannot be used for slave access requests. Slots in 
                 * the access window not affected by traffic can still 
                 * be used according to the defined access structure; 
                 * an example is shown in Figure 10.11 on page 115: 
                 * the access procedure is continued as if no interruption 
                 * had taken place.'
                 *
                 * If a SCO packet is due in the next Tx/Rx frame, then 
                 * the Scheduler object will ensure that the USLCparkMaster
                 * object is not given access to the channel in the next
                 * Tx/Rx frame.  Thus, at this point it can be assumed that
                 * a SCO packet is not due in the next frame.
                 * 
                 * Assumption: SCO data is the only data that will interrupt 
                 * the broadcast packets sent in the Access Window. I.e. 
                 * Non-broadcast ACL data will not be sent in the Access Window
                 * (at least for the 1st 'n_acc_slot' slots).
                 *
                 * With this assumption, it can be assumed that the LM will 
                 * transmit a broadcast packet in the next Tx slot and thus
                 * the LC can accept Slave-init Unpark requests in the next frame.
                 */               
                   
                /*
                 * Determine the next packet to be broadcast in the 
                 * Access Window. 
                 * 
                 * Note: At this point 'LC_Get_Next_Master_Tx_Activity()' 
                 *       should always return a 'non-zero' result.
                 *       Only broadcast packets on the local_device_link
                 *       LMP_OUT_Q or L2CAP_OUT_Q are permitted here.
                 */
                /*
                 * If a callback registration has been made then call the 
                 * function. Otherwise default to sending NULL packet.
                 */
                if (parkMaster_info->access_window_callback)
                {
                    pDL = parkMaster_info->access_window_callback(); /* LC_Get_Next_Master_Tx_Activity(); */
                }
                if (  pDL !=0 )
                {
                    LSLCpkt_Generate_ACL(pDL);
                }
                else /* Generate NULLs in the access window */
                {
                    LSLCpkt_Generate_NULL(parkMaster_info->p_active_device_link);
                }

                 /*
                  * Setup Receiver to be capable of receiving an
                  * ID packet in either half of the next Rx slot.
                  */
                 USLCchac_Set_Device_State(Connection_Park_Access_Window_Rx);
#if (PRH_BS_DEV_TIM3_IDENTIFIES_AR_ADDR_HALF_SLOT==1)
                 parkMaster_info->half_slot = 0;
#endif

                 parkMaster_info->ctrl_state = OS_PARK_MASTER_ACCESS_WINDOW_RX;
            }
            break;

        case OS_PARK_MASTER_ACCESS_WINDOW_RX:
            if (look_ahead_to_next_frame)
            {
                LSLCpkt_Generate_NULL(parkMaster_info->p_active_device_link);
#if (PRH_BS_DEV_TIM3_IDENTIFIES_AR_ADDR_HALF_SLOT==1)
                parkMaster_info->half_slot = 0;
#endif
                parkMaster_info->ctrl_state = OS_PARK_LOOK_AHEAD_MASTER_ACCESS_WINDOW_RX;
            }
            else
            {
                if (IDpkt == event)
                {
                /*
                 * The current bt_clk is used to determine which 'half'
                 * the ID packet is received.
                 * Thus, an assumption is made that when the current
                 * bt_clk is read here, that we are still in the same
                 * 'half-slot' that the ID packet was received.
                 */
                clock_when_id_pkt_recvd = HW_get_native_clk();

                /*
                 * Equation for determining AR_ADDR:
                 * AR_ADDR = ((bt_clk(ID pkt) - bt_clk(Acc Win start) - 1)/2)+1
                 *
                 * Verify using example:
                 * Assume: access_window_start=4
                 *
                 * bt_clk(ID pkt)   Equation    =  AR_ADDR
                 *     6         ((6-4-1)/2)+1  =     1
                 *     7         ((7-4-1)/2)+1  =     2
                 *     10        ((10-4-1)/2)+1 =     3
                 *     11        ((11-4-1)/2)+1 =     4
                 *     ..........
                 */

                /*
                 * Note:
                 * The LC knows nothing about the AR_ADDR assigned to the 
                 * Parked devices.  This information is stored in the LM.
                 * Thus, the 'deviceIndex' element if the 'event_info' 
                 * structure indicates the AR_ADDR of the Parked Slave
                 * requesting the Slave-init unpark.
                 */
  				 event_info.deviceIndex =              
#if (PRH_BS_DEV_TIM3_IDENTIFIES_AR_ADDR_HALF_SLOT==1)
                   (BTtimer_Clock_Difference(parkMaster_info->access_window_start,
                    clock_when_id_pkt_recvd)/2)+parkMaster_info->half_slot;
#else
                   (BTtimer_Clock_Difference(parkMaster_info->access_window_start-1,
				    clock_when_id_pkt_recvd)/2)+1;
#endif

                 event_info.status = NO_ERROR;
                 LC_Callback_Event_Handler(LC_EVENT_PARK_SLAVE_INIT_UNPARK_REQUEST,
                                                              &event_info);
                }

                /* 
                 * Check if n_acc_slots has been reached 
                 */
                if ( BTtimer_Is_Expired(parkMaster_info->n_acc_slot_timer))
                {
                    parkMaster_info->ctrl_state = OS_PARK_MASTER_END_ACCESS_WINDOW;
                }
                else  /*n_acc_slot_timer NOT expired */
                {
                    parkMaster_info->ctrl_state = OS_PARK_MASTER_ACCESS_WINDOW_TX;
                }
            }
            break;

        case OS_PARK_LOOK_AHEAD_MASTER_ACCESS_WINDOW_RX:
                if (IDpkt == event)
                {
                /*
                 * The current bt_clk is used to determine which 'half'
                 * the ID packet is received.
                 * Thus, an assumption is made that when the current
                 * bt_clk is read here, that we are still in the same
                 * 'half-slot' that the ID packet was received.
                 */
                clock_when_id_pkt_recvd = HW_get_native_clk();

                /*
                 * Equation for determining AR_ADDR:
                 * AR_ADDR = ((bt_clk(ID pkt) - bt_clk(Acc Win start) - 1)/2)+1
                 *
                 * Verify using example:
                 * Assume: access_window_start=4
                 *
                 * bt_clk(ID pkt)   Equation    =  AR_ADDR
                 *     6         ((6-4-1)/2)+1  =     1
                 *     7         ((7-4-1)/2)+1  =     2
                 *     10        ((10-4-1)/2)+1 =     3
                 *     11        ((11-4-1)/2)+1 =     4
                 *     ..........
                 */

                /*
                 * Note:
                 * The LC knows nothing about the AR_ADDR assigned to the 
                 * Parked devices.  This information is stored in the LM.
                 * Thus, the 'deviceIndex' element if the 'event_info' 
                 * structure indicates the AR_ADDR of the Parked Slave
                 * requesting the Slave-init unpark.
                 */
				
 				 event_info.deviceIndex = 
#if (PRH_BS_DEV_TIM3_IDENTIFIES_AR_ADDR_HALF_SLOT==1)
                   (BTtimer_Clock_Difference(parkMaster_info->access_window_start,
                    clock_when_id_pkt_recvd)/2)+parkMaster_info->half_slot;
#else
                   (BTtimer_Clock_Difference(parkMaster_info->access_window_start-1,
				    clock_when_id_pkt_recvd)/2)+1;
#endif

                 event_info.status = NO_ERROR;
                 LC_Callback_Event_Handler(LC_EVENT_PARK_SLAVE_INIT_UNPARK_REQUEST,
                                                              &event_info);
                }

                 /* 
                  * Check if n_acc_slots has been reached 
                  */
                if ( BTtimer_Is_Expired(parkMaster_info->n_acc_slot_timer))
                {
                    /*
                     * CANCEL CURRENT TRANSMISSION OF ID PKTS THAT WAS PREPARED DURING 
                     * THE LOOK-AHEAD PROCEDURE.
                    */
                    LSLCacc_Disable_Transmitter();
                    parkMaster_info->ctrl_state = OS_PARK_MASTER_END_ACCESS_WINDOW;
                }
                else  /*n_acc_slot_timer NOT expired */
                    parkMaster_info->ctrl_state = OS_PARK_MASTER_ACCESS_WINDOW_RX;

                break;


        default:
            break;
    }

    /*
     * 'One-off' tasks associated with the parkMaster 'End'
     * states. 
     */
    switch ((u_int8)parkMaster_info->ctrl_state)
    {
        case OS_PARK_END_:
            /*
             * Callback to the higher layer to indicate that the 
             * device has been parked.
             * Specify the park device_link assigned to the parked device.
             */
            event_info.parkDeviceIndex = park_device_index;
            event_info.deviceIndex = parkMaster_info->device_index;
            event_info.status = NO_ERROR;
            LC_Callback_Event_Handler(LC_EVENT_PARK_COMPLETE,&event_info);

            /*
             * Indicate to the USLCchac that the Park Slave procedure 
             * is finished. 
             */
            USLCchac_Procedure_Finish(TRUE/*return to prev super_state*/);

            parkMaster_info->ctrl_state=OS_NOT_ACTIVE_;
            break;

        case OS_END_UNPARK_SUCCESSFUL_:
            /* 
             * Unpark has been successful.
             * Commit the 'un_parked' device_link.
             */
            DL_Unpark_Commit(parkMaster_info->park_device_index, SLAVE /*role of peer*/);        

            /*
             * Specify the device_link assigned to the unparked device.
             */
            event_info.status = NO_ERROR;
            event_info.deviceIndex = parkMaster_info->device_index;
            event_info.parkDeviceIndex = parkMaster_info->park_device_index;
            LC_Callback_Event_Handler(LC_EVENT_UNPARK_COMPLETE, &event_info);
            
            /*
             * Indicate to the USLCchac that the Park Slave procedure 
             * is finished, and indicate that the super_state is not
             * to be modified. 
             */
            USLCchac_Procedure_Finish(FALSE /* Do not return to 
                                                prev super_state*/);

            parkMaster_info->ctrl_state=OS_NOT_ACTIVE_;
            break;

        case OS_END_UNPARK_UNSUCCESSFUL_:
            /*
             * DL already unparked just before entering the OS_UNPARK_START
             * state. Unpark must be 'rolled-back'.
             */
            DL_Unpark_Rollback(parkMaster_info->device_index);

            event_info.status = UNSPECIFIED_ERROR;
            event_info.parkDeviceIndex = parkMaster_info->park_device_index;

            LC_Callback_Event_Handler(LC_EVENT_UNPARK_COMPLETE,&event_info);

            /*
             * Indicate to the USLCchac that the Park Slave procedure 
             * is finished, and indicate to set 
             * super_state=previous_super_state
             */

            USLCchac_Procedure_Finish(TRUE/*Return to prev super_state*/);
            parkMaster_info->ctrl_state=OS_NOT_ACTIVE_;
            break;

        case OS_PARK_MASTER_END_ACCESS_WINDOW:
            /*
             * Implicitly sets the device state back to Connection
             * (i.e. previous state).
             */
            USLCchac_Procedure_Finish(TRUE/*Return to prev super_state*/);
            parkMaster_info->ctrl_state=OS_NOT_ACTIVE_;
#if 1 // GF 24 March - Trial Putting RF in Standby after ACCESS-WINDOW
	  // This Seems to solve the problem -- MUST INVESTIGATE WHY !!!!!!
			HWradio_SetRadioMode(RADIO_MODE_STANDBY);
#endif

            break;
    }    /* switch (parkMaster_info->ctrl_state)*/

}

/******************************************************************
 *
 * void USLCparkMaster_Initialise(void)
 *
 ******************************************************************/

void USLCparkMaster_Initialise(void)
{
    t_parkMaster *parkMaster_info = &_parkMaster_info;

    /*
     * Initialise defaults for Park Slave
     */

    parkMaster_info->ctrl_state = OS_NOT_ACTIVE_;
    parkMaster_info->access_window_callback = 0;

}

/******************************************************************
 *
 * void USLCparkMaster_Register_Access_Window_Callback_Handler
 *
 ******************************************************************/
void USLCparkMaster_Register_Access_Window_Callback_Handler(t_callback func_ptr)
{
    t_parkMaster *parkMaster_info = &_parkMaster_info;

    parkMaster_info->access_window_callback = func_ptr;
}

/*
 * Test support functions
 */ 
t_USLCparkMaster_ctrl_state USLCparkMaster_Get_Ctrl_State(void)
{
    return _parkMaster_info.ctrl_state;
}

#if (PRH_BS_DEV_TIM3_IDENTIFIES_AR_ADDR_HALF_SLOT==1)
void USLCparkMaster_Second_Half_Slot()
{
    _parkMaster_info.half_slot = 1;
}
#endif

#else /* (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==0)*/
/* completely empty C files are illegal in ANSI */
void USLCparkMaster_Dummy__(void) { }
#endif /* PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED */

