/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#include "bsp_cli.h"

static bsp_event_callback_t   m_bsp_cli_callback         = NULL;

static void cmd_btn(nrf_cli_t const * p_cli, size_t argc, char **argv)
{
    uint32_t id;
    ASSERT(m_bsp_cli_callback != NULL);

    sscanf(argv[1], "%"SCNu32, &id);
    bsp_event_t ev = (bsp_event_t)(BSP_EVENT_KEY_0 + id);
    m_bsp_cli_callback(ev);
}

static void cmd_evt(nrf_cli_t const * p_cli, size_t argc, char **argv)
{
    uint32_t id;
    ASSERT(m_bsp_cli_callback != NULL);

    sscanf(argv[1], "%"SCNu32, &id);
    bsp_event_t ev = (bsp_event_t)id;
    m_bsp_cli_callback(ev);
}

ret_code_t bsp_cli_init(bsp_event_callback_t callback)
{
    m_bsp_cli_callback = callback;

    return XINC_SUCCESS;
}

XINC_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_bsp)
{
    XINC_CLI_CMD(btn, NULL, "BSP button event key", cmd_btn),
    XINC_CLI_CMD(evt, NULL, "BSP event id", cmd_evt),
    XINC_CLI_SUBCMD_SET_END
};
XINC_CLI_CMD_REGISTER(bsp, &m_sub_bsp, "bsp", NULL);
