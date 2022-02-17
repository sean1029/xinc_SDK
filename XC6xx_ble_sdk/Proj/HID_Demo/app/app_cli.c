/**
 * Copyright (c) 2018 - 2021, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include "nrf_cli.h"
//#include "nrf_log.h"
#include "nrf_gpio.h"
static void cmd_led(nrf_cli_t const * p_cli, size_t argc, char **argv)
{
    UNUSED_PARAMETER(argc);
    UNUSED_PARAMETER(argv);

    if (nrf_cli_help_requested(p_cli))
    {
        nrf_cli_help_print(p_cli, NULL, 0);
        return;
    }

//    NVIC_SystemReset();
		printf("cmd_led\r\n");
}

#define NRF_CLI_HELP_XINC              "Useful, not Unix-like XINC commands."
#define NRF_CLI_HELP_LED_OFF         "Disable led off."
#define NRF_CLI_HELP_LED_ON          "Enable led on."
#define NRF_CLI_HELP_LED             "Trun on / off led."
#define NRF_CLI_MSG_SPECIFY_SUBCOMMAND1  "Please specify a subcommand."
#define NRF_CLI_MSG_UNKNOWN_PARAMETER1   " unknown parameter: "

static void nrf_cli_cmd_led_off(nrf_cli_t const * p_cli, size_t argc, char **argv)
{
   
		for(int i = 0; i < argc;i++)
		{
			printf("argv[%d]:%s\r\n",i,argv[i]);
		}
    printf("cmd_led_off\r\n");
		nrf_gpio_pin_set(5);
}

static void nrf_cli_cmd_led_on(nrf_cli_t const * p_cli, size_t argc, char **argv)
{
		for(int i = 0; i < argc;i++)
		{
			printf("argv[%d]:%s\r\n",i,argv[i]);
		}
    printf("cmd_led_on\r\n");
		
		nrf_gpio_pin_clear(5);
}

static void nrf_cli_cmd_xinc(nrf_cli_t const * p_cli, size_t argc, char **argv)
{
    ASSERT(p_cli);
    ASSERT(p_cli->p_ctx && p_cli->p_iface && p_cli->p_name);
    UNUSED_PARAMETER(argv);
		printf("cmd_xinc\r\n");
    if ((argc == 1) || ((argc == 2) && nrf_cli_help_requested(p_cli)) )
    {
        nrf_cli_help_print(p_cli, NULL, 0);
        return;
    }
    nrf_cli_error(p_cli, NRF_CLI_MSG_SPECIFY_SUBCOMMAND1);
}

static void nrf_cli_cmd_led(nrf_cli_t const * p_cli, size_t argc, char **argv)
{
    ASSERT(p_cli);
    ASSERT(p_cli->p_ctx && p_cli->p_iface && p_cli->p_name);
		printf("cmd_led\r\n");
    if (argc == 1)
    {
        nrf_cli_help_print(p_cli, NULL, 0);
        return;
    } 

    nrf_cli_error(p_cli, "%s:%s%s", argv[0], NRF_CLI_MSG_UNKNOWN_PARAMETER1, argv[1]);
}
/**
 * @brief Command set array
 * */

NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_led)
{
    NRF_CLI_CMD(off,    NULL,   NRF_CLI_HELP_LED_OFF,        nrf_cli_cmd_led_off),
    NRF_CLI_CMD(on,     NULL,   NRF_CLI_HELP_LED_ON,         nrf_cli_cmd_led_on),
    NRF_CLI_SUBCMD_SET_END
};


NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_xinc)
{
    NRF_CLI_CMD(led, &m_sub_led,    NRF_CLI_HELP_LED,        nrf_cli_cmd_led),


    NRF_CLI_SUBCMD_SET_END
};


NRF_CLI_CMD_REGISTER(xinc, &m_sub_xinc, NRF_CLI_HELP_XINC, nrf_cli_cmd_xinc);


//#if NRF_MODULE_ENABLED(NRF_CLI_VT100_COLORS)
//NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_colors)
//{
//    NRF_CLI_CMD(off,    NULL,   NRF_CLI_HELP_COLORS_OFF,        nrf_cli_cmd_colors_off),
//    NRF_CLI_CMD(on,     NULL,   NRF_CLI_HELP_COLORS_ON,         nrf_cli_cmd_colors_on),
//    NRF_CLI_SUBCMD_SET_END
//};
//#endif

//NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_cli)
//{
//#if NRF_MODULE_ENABLED(NRF_CLI_VT100_COLORS)
//    NRF_CLI_CMD(colors, &m_sub_colors,      NRF_CLI_HELP_COLORS,        nrf_cli_cmd_colors),
//#endif
//    NRF_CLI_CMD(echo,   &m_sub_echo,        NRF_CLI_HELP_ECHO,          nrf_cli_cmd_echo),
//#if NRF_MODULE_ENABLED(NRF_CLI_STATISTICS)
//    NRF_CLI_CMD(stats, &m_sub_cli_stats,    NRF_CLI_HELP_STATISTICS,    nrf_cli_cmd_cli_stats),
//#endif
//    NRF_CLI_SUBCMD_SET_END
//};

//NRF_CLI_CMD_REGISTER(cli,       &m_sub_cli,     NRF_CLI_HELP_CLI,       nrf_cli_cmd_cli);
