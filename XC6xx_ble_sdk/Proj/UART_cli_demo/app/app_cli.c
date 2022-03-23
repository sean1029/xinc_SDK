/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#include "xinc_cli.h"

#if XINC_CLI_ENABLED

#include <stdlib.h>
#include "xinc_gpio.h"
#include "boards.h"
static void cmd_led(xinc_cli_t const * p_cli, size_t argc, char **argv)
{
    UNUSED_PARAMETER(argc);
    UNUSED_PARAMETER(argv);

    if (xinc_cli_help_requested(p_cli))
    {
        xinc_cli_help_print(p_cli, NULL, 0);
        return;
    }
  
}

#define XINC_CLI_HELP_XINC              "Useful, not Unix-like XINC commands."
#define XINC_CLI_HELP_LED_OFF         "Disable led off."
#define XINC_CLI_HELP_LED_ON          "Enable led on."
#define XINC_CLI_HELP_LED             "Trun on / off led."
#define XINC_CLI_MSG_SPECIFY_SUBCOMMAND1  "Please specify a subcommand."
#define XINC_CLI_MSG_UNKNOWN_PARAMETER1   " unknown parameter: "

static void xinc_cli_cmd_led_off(xinc_cli_t const * p_cli, size_t argc, char **argv)
{
    uint8_t idx;
    if (argc == 1 )
    {
        bsp_board_leds_off();
        idx = 0xff;
    }
    else if (argc <= 5)
    {
        for(int i = 1; i <= argc;i++)
        {
            idx = strtol(argv[i],NULL,10);
            bsp_board_led_off(idx);
        }
        
    }
    else
    {
        xinc_cli_help_print(p_cli, NULL, 0);
        return;   
    }
      
    xinc_cli_print(p_cli, "cmd led off");

}

static void xinc_cli_cmd_led_on(xinc_cli_t const * p_cli, size_t argc, char **argv)
{
    uint8_t idx;
    if (argc == 1 )
    {
        bsp_board_leds_on();
        idx = 0xff;
    }
     else if (argc <= 5)
    {
        for(int i = 1; i <= argc;i++)
        {
            idx = strtol(argv[i],NULL,10);
            bsp_board_led_on(idx);
        }   
    }
    else
    {
        xinc_cli_help_print(p_cli, NULL, 0);
        return;   
    }
      
    xinc_cli_print(p_cli, "cmd led on");


}

static void xinc_cli_cmd_xinc(xinc_cli_t const * p_cli, size_t argc, char **argv)
{
    ASSERT(p_cli);
    ASSERT(p_cli->p_ctx && p_cli->p_iface && p_cli->p_name);
    UNUSED_PARAMETER(argv);
    if ((argc == 1) || ((argc == 2) && xinc_cli_help_requested(p_cli)) )
    {
        xinc_cli_help_print(p_cli, NULL, 0);
        return;
    }
    xinc_cli_error(p_cli, XINC_CLI_MSG_SPECIFY_SUBCOMMAND1);
}

static void xinc_cli_cmd_led(xinc_cli_t const * p_cli, size_t argc, char **argv)
{
    ASSERT(p_cli);
    ASSERT(p_cli->p_ctx && p_cli->p_iface && p_cli->p_name);
    if (argc == 1)
    {
        xinc_cli_help_print(p_cli, NULL, 0);
        return;
    } 

    xinc_cli_error(p_cli, "%s:%s%s", argv[0], XINC_CLI_MSG_UNKNOWN_PARAMETER1, argv[1]);
}
/**
 * @brief Command set array
 * */

XINC_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_led)
{
    XINC_CLI_CMD(off,    NULL,   XINC_CLI_HELP_LED_OFF,        xinc_cli_cmd_led_off),
    XINC_CLI_CMD(on,     NULL,   XINC_CLI_HELP_LED_ON,         xinc_cli_cmd_led_on),
    XINC_CLI_SUBCMD_SET_END
};


XINC_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_bsp)
{
    XINC_CLI_CMD(led, &m_sub_led,    XINC_CLI_HELP_LED,        xinc_cli_cmd_led),


    XINC_CLI_SUBCMD_SET_END
};


XINC_CLI_CMD_REGISTER(bsp, &m_sub_bsp, XINC_CLI_HELP_XINC, xinc_cli_cmd_xinc);


#endif

