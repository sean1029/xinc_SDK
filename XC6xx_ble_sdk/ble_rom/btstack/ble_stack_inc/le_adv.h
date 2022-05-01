#ifndef     __LE_ADV_H
#define     __LE_ADV_H

typedef struct
{
	u_int16 advertising_interval;
	u_int8  advertising_type;
	u_int8  advertising_own_address_type;
	u_int8  direct_address_type;
	u_int8  direct_address[6];
	u_int8  advertising_channel_map;
	u_int8  advertising_channel_index;
	u_int8  advertising_channels_used;
	u_int8  advertising_filter_policy;
	u_int8  advertising_tx_power_level;
	u_int8  advertising_data_len;
	u_int8  advertising_data[31];
	u_int8  advertise_enable;
	u_int8  scan_response_data_len;
	u_int8  scan_response_data[31];
	t_timer direct_advertising_timer;
	t_timer next_adv_timer;
    t_timer advDelay;                   //add for ble lowpower.@20161212 
#if (PRH_BS_CFG_SYS_EXTENDED_HCI_COMMANDS_SUPPORTED==1)
	u_int8  user_direct_adv_timeout;
	u_int16 direct_adv_timeout;
	u_int8 advertising_delta;
	u_int8 delta_count;
#endif

} t_LE_Advertising;

#endif
