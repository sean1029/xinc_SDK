/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#ifndef XINC_CLI_TYPES_H__
#define XINC_CLI_TYPES_H__

#include <inttypes.h>
#include "sdk_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (XINC_CLI_CMD_BUFF_SIZE > 65535)
    typedef uint32_t xinc_cli_cmd_len_t;
#elif (XINC_CLI_CMD_BUFF_SIZE > 255)
    typedef uint16_t xinc_cli_cmd_len_t;
#else
    typedef uint8_t xinc_cli_cmd_len_t;
#endif

typedef enum
{
    XINC_CLI_VT100_COLOR_DEFAULT,
    XINC_CLI_VT100_COLOR_BLACK,
    XINC_CLI_VT100_COLOR_RED,
    XINC_CLI_VT100_COLOR_GREEN,
    XINC_CLI_VT100_COLOR_YELLOW,
    XINC_CLI_VT100_COLOR_BLUE,
    XINC_CLI_VT100_COLOR_MAGENTA,
    XINC_CLI_VT100_COLOR_CYAN,
    XINC_CLI_VT100_COLOR_WHITE,

    VT100_COLOR_END
} xinc_cli_vt100_color_t;

typedef struct
{
    xinc_cli_vt100_color_t col;      // text color
    xinc_cli_vt100_color_t bgcol;    // background color
} xinc_cli_vt100_colors_t;

typedef struct
{
    xinc_cli_cmd_len_t cur_x;        // horizontal cursor position in edited command line
    xinc_cli_cmd_len_t cur_x_end;    // horizontal cursor position at the end of command
    xinc_cli_cmd_len_t cur_y;        // vertical cursor position in edited command
    xinc_cli_cmd_len_t cur_y_end;    // vertical cursor position at the end of command
    xinc_cli_cmd_len_t terminal_hei; // terminal screen height
    xinc_cli_cmd_len_t terminal_wid; // terminal screen width
    uint8_t name_len;               // console name length
} xinc_cli_multiline_cons_t;

typedef struct
{
    xinc_cli_multiline_cons_t cons;
    xinc_cli_vt100_colors_t col;
    xinc_cli_cmd_len_t printed_cmd;  // printed commands counter
} xinc_cli_vt100_ctx_t;

#ifdef __cplusplus
}
#endif

#endif /* XINC_CLI_TYPES_H__ */

