/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#include "nrf_cli.h"
#if XINC_MODULE_ENABLED(XINC_CLI)
static void cmd_reset(nrf_cli_t const * p_cli, size_t argc, char **argv)
{
    UNUSED_PARAMETER(argc);
    UNUSED_PARAMETER(argv);

    if (nrf_cli_help_requested(p_cli))
    {
        nrf_cli_help_print(p_cli, NULL, 0);
        return;
    }

//    NVIC_SystemReset();
		printf("cmd_reset\r\n");
}


static void cmd_error(nrf_cli_t const * p_cli, size_t argc, char **argv)
{
    UNUSED_PARAMETER(argc);
    UNUSED_PARAMETER(argv);

    if (nrf_cli_help_requested(p_cli))
    {
        nrf_cli_help_print(p_cli, NULL, 0);
        return;
    }
		printf("cmd_error\r\n");
    APP_ERROR_CHECK(XINC_ERROR_INTERNAL);
}


static void cmd_app_size(nrf_cli_t const * p_cli, size_t argc, char **argv)
{
    UNUSED_PARAMETER(argc);
    UNUSED_PARAMETER(argv);

    if (nrf_cli_help_requested(p_cli))
    {
        nrf_cli_help_print(p_cli, NULL, 0);
        return;
    }

      nrf_cli_fprintf(p_cli,
                    XINC_CLI_NORMAL,
                    "Application address:%d (0x%08X), size: %d (0x%08X)\r\n",
                    CODE_START,
                    CODE_START,
                    CODE_SIZE,
                    CODE_SIZE);
	//	nrf_cli_print_stream(0,0,1);
}


static void cmd_log_msg_error(nrf_cli_t const * p_cli, size_t argc, char **argv)
{
    if (nrf_cli_help_requested(p_cli))
    {
        nrf_cli_help_print(p_cli, NULL, 0);
        return;
    }

    switch (argc-1)
    {
        case 0:
          //  XINC_LOG_ERROR("test error message");
            break;
        case 1:
         //   XINC_LOG_ERROR("test error message: %d", strtol(argv[1], NULL, 10));
            break;
        case 2:
         //   XINC_LOG_ERROR("test error message: %d %d", strtol(argv[1], NULL, 10),
           //                                            strtol(argv[2], NULL, 10));
            break;
        default:
         //   XINC_LOG_ERROR("test error message with more than 3 arguments"); 
            break;
    }
}


static void cmd_log_msg_warning(nrf_cli_t const * p_cli, size_t argc, char **argv)
{
    if (nrf_cli_help_requested(p_cli))
    {
        nrf_cli_help_print(p_cli, NULL, 0);
        return;
    }

    switch (argc-1)
    {
        case 0:
       //     XINC_LOG_WARNING("test warning message");
            break;
        case 1:
         //   XINC_LOG_WARNING("test warning message: %d", strtol(argv[1], NULL, 10));
            break;
        case 2:
        //    XINC_LOG_WARNING("test warning message: %d %d", strtol(argv[1], NULL, 10),
         //                                              strtol(argv[2], NULL, 10));
            break;
        default:
         //   XINC_LOG_WARNING("test warning message with more than 3 arguments"); 
            break;
    }
}

/**
 * @brief Command set array
 * */

XINC_CLI_CMD_REGISTER(reset, NULL, "System reset.", cmd_reset);

XINC_CLI_CMD_REGISTER(error, NULL, "Trigger error.", cmd_error);

XINC_CLI_CMD_REGISTER(app_size, NULL, "Print application size.", cmd_app_size);

XINC_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_log_msg)
{
    XINC_CLI_CMD(error,   NULL, "Error log message with parameters", cmd_log_msg_error),
    XINC_CLI_CMD(warning, NULL, "Warning log message with parameters", cmd_log_msg_warning),
    XINC_CLI_SUBCMD_SET_END
};

XINC_CLI_CMD_REGISTER(log_msg, &m_sub_log_msg, "Trigger log message with decimal arguments", NULL);
#endif //

