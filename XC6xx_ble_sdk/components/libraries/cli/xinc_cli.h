/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#ifndef XINC_CLI_H__
#define XINC_CLI_H__

#include "sdk_common.h"
#include "xinc_cli_types.h"
#include "xinc_section.h"
#include "xinc_log_backend_interface.h"
#include "xinc_queue.h"
//#include "xinc_log_ctrl.h"
#include "app_util_platform.h"
#include "xinc_memobj.h"

#if XINC_MODULE_ENABLED(XINC_CLI_USES_TASK_MANAGER)
#include "task_manager.h"
#endif

#include "xinc_fprintf.h"
#include "xinc_fprintf_format.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XINC_CLI_RX_BUFF_SIZE 16

/* CLI reserves top task manager flags, bits 0...18 are available for application. */
#define XINC_CLI_TRANSPORT_TX_RDY_TASK_EVT      (1UL << 19)
#define XINC_CLI_TRANSPORT_RX_RDY_TASK_EVT      (1UL << 20)
#define XINC_CLI_LOG_PENDING_TASK_EVT           (1UL << 21)
#define XINC_CLI_CMD_EXECUTE_EVT                (1UL << 22)
#define XINC_CLI_KILL_TASK_EVT                  (1UL << 23)

#define XINC_CLI_TASK_EVTS (XINC_CLI_TRANSPORT_TX_RDY_TASK_EVT | \
                           XINC_CLI_TRANSPORT_RX_RDY_TASK_EVT | \
                           XINC_CLI_LOG_PENDING_TASK_EVT      | \
                           XINC_CLI_CMD_EXECUTE_EVT           | \
                           XINC_CLI_KILL_TASK_EVT)
/**
 * @defgroup xinc_cli Command Line Interface
 * @ingroup app_common
 *
 * @brief Module for unified command line handling.
 *
 * @{
 */

/**
 * @brief   Aliases to: @ref xinc_cli, @ref xinc_cli_cmd_entry, and @ref xinc_cli_static_entry.
 *          Must be created here to satisfy module declaration order dependencies.
 */
typedef struct xinc_cli xinc_cli_t;
typedef struct xinc_cli_cmd_entry xinc_cli_cmd_entry_t;
typedef struct xinc_cli_static_entry xinc_cli_static_entry_t;

/**
 * @brief CLI dynamic command descriptor.
 *
 * @details Function shall fill the received @ref xinc_cli_static_entry structure with requested (idx)
 *          dynamic subcommand data. If there is more than one dynamic subcommand available,
 *          the function shall ensure that the returned commands: p_static->p_syntax are sorted in
 *          alphabetical order. If idx exceeds the available dynamic subcommands, the function must write
 *          to p_static->p_syntax NULL value. This will indicate to the CLI module that
 *          there are no more dynamic commands to read.
 */
typedef void (*xinc_cli_dynamic_get)(size_t idx, xinc_cli_static_entry_t * p_static);

/**
 * @brief CLI command descriptor.
 */
struct xinc_cli_cmd_entry
{
    bool is_dynamic;
    union
    {
        xinc_cli_dynamic_get p_dynamic_get;  //!< Pointer to function returning dynamic commands.
        xinc_cli_static_entry_t const *  p_static;   //!< Pointer to array of static commands.
    } u;
};

/**
 * @brief CLI command handler prototype.
 */
typedef void (*xinc_cli_cmd_handler)(xinc_cli_t const * p_cli, size_t argc, char **argv);

/**
 * @brief CLI static command descriptor.
 */
struct xinc_cli_static_entry
{
    char const * p_syntax;  //!< Command syntax strings.
    char const * p_help;    //!< Command help string.

    xinc_cli_cmd_entry_t const * p_subcmd;   //!< Pointer to subcommand.

    xinc_cli_cmd_handler handler; //!< Command handler.
};

/**
 * @brief Macro for defining and adding a root command (level 0).
 *
 * @note Each root command shall have unique syntax.
 *
 * @param[in] p_syntax  Command syntax (for example: history).
 * @param[in] p_subcmd  Pointer to a subcommands array.
 * @param[in] p_help    Pointer to a command help string.
 * @param[in] p_handler Pointer to a function handler.
 */
#define XINC_CLI_CMD_REGISTER(p_syntax, p_subcmd, p_help, p_handler)     \
    xinc_cli_static_entry_t const CONCAT_3(xinc_cli_, p_syntax, _raw) =   \
        XINC_CLI_CMD(p_syntax, p_subcmd, p_help, p_handler);             \
    XINC_SECTION_ITEM_REGISTER(cli_command,                              \
                              xinc_cli_cmd_entry_t const CONCAT_3(xinc_cli_, p_syntax, _const)) = { \
                                .is_dynamic = false,                    \
                                .u = {.p_static = &CONCAT_3(xinc_cli_, p_syntax, _raw)} \
    }; \
    XINC_SECTION_ITEM_REGISTER(cli_sorted_cmd_ptrs, char const * CONCAT_2(p_syntax, _str_ptr))

/**
 * @brief Macro for creating a subcommand set. It must be used outside of any function body.
 *
 * @param[in] name  Name of the subcommand set.
 */
#define XINC_CLI_CREATE_STATIC_SUBCMD_SET(name)                  \
    /*lint -save -e85 -e31*/                                    \
    static xinc_cli_static_entry_t const CONCAT_2(name, _raw)[]; \
    static xinc_cli_cmd_entry_t const name = {                   \
        .is_dynamic = false,                                    \
        .u = {.p_static = CONCAT_2(name, _raw) }                \
    };                                                          \
    static xinc_cli_static_entry_t const CONCAT_2(name, _raw)[] = /*lint -restore*/

/**
 * @brief Define ending subcommands set.
 *
 */
#define XINC_CLI_SUBCMD_SET_END {NULL}

/**
 * @brief Macro for creating a dynamic entry.
 *
 * @param[in] name  Name of the dynamic entry.
 * @param[in] p_get Pointer to the function returning dynamic commands array @ref xinc_cli_dynamic_get.
 */
#define XINC_CLI_CREATE_DYNAMIC_CMD(name, p_get) \
    /*lint -save -e19*/                         \
    static xinc_cli_cmd_entry_t const name = {   \
        .is_dynamic  = true,                    \
        .u = { .p_dynamic_get = p_get }         \
}; /*lint -restore*/

/** @brief Macro for creating subcommands when C++ compiler is used.
 *
 * Example usage:
 * @code
 * XINC_CLI_CPP_CREATE_STATIC_SUBCMD_SET(cmd_syntax,
 * XINC_CLI_CMD(abc, ...),
 * XINC_CLI_CMD(def, ...),
 * XINC_CLI_SUBCMD_SET_END
 * );
 * @endcode
 */
#define XINC_CLI_CPP_CREATE_STATIC_SUBCMD_SET(name, ...)             \
    static xinc_cli_static_entry_t const CONCAT_2(name, _raw)[] = {  \
        __VA_ARGS__                                                 \
    };                                                              \
    static xinc_cli_cmd_entry_t const name = {                       \
        .is_dynamic  = false,                                       \
        .u = { .p_static = CONCAT_2(name, _raw) }                   \
    }

/**
 * @brief Initializes a CLI command (@ref xinc_cli_static_entry).
 *
 * @param[in] _p_syntax  Command syntax (for example: history).
 * @param[in] _p_subcmd  Pointer to a subcommands array.
 * @param[in] _p_help    Pointer to a command help string.
 * @param[in] _p_handler Pointer to a function handler.
 */
#define XINC_CLI_CMD(_p_syntax, _p_subcmd, _p_help, _p_handler) { \
    .p_syntax = (const char *)  STRINGIFY(_p_syntax), \
    .p_help  = (const char *)   _p_help,    \
    .p_subcmd =                 _p_subcmd,  \
    .handler =                  _p_handler  \
}

/**
 * @internal @brief Internal CLI state in response to data received from the terminal.
 */
typedef enum
{
    XINC_CLI_RECEIVE_DEFAULT,
    XINC_CLI_RECEIVE_ESC,
    XINC_CLI_RECEIVE_ESC_SEQ,
    XINC_CLI_RECEIVE_TILDE_EXP
} xinc_cli_receive_t;


/**
 * @internal @brief Internal CLI state.
 */
typedef enum
{
    XINC_CLI_STATE_UNINITIALIZED,      //!< State uninitialized.
    XINC_CLI_STATE_INITIALIZED,        //!< State initialized but not active.
    XINC_CLI_STATE_ACTIVE,             //!< State active.
    XINC_CLI_STATE_PANIC_MODE_ACTIVE,  //!< State panic mode activated.
    XINC_CLI_STATE_PANIC_MODE_INACTIVE //!< State panic mode requested but not supported.
} xinc_cli_state_t;

/**
 * @brief Event type from CLI transport.
 */
typedef enum
{
    XINC_CLI_TRANSPORT_EVT_RX_RDY,
    XINC_CLI_TRANSPORT_EVT_TX_RDY
} xinc_cli_transport_evt_t;

typedef void (*xinc_cli_transport_handler_t)(xinc_cli_transport_evt_t evt_type, void * p_context);

typedef struct xinc_cli_transport_s xinc_cli_transport_t;

/**
 * @brief Unified CLI transport interface.
 */
typedef struct
{
    /**
     * @brief Function for initializing the CLI transport interface.
     *
     * @param[in] p_transport   Pointer to the transfer instance.
     * @param[in] p_config      Pointer to instance configuration.
     * @param[in] evt_handler   Event handler.
     * @param[in] p_context     Pointer to the context passed to event handler.
     *
     * @return Standard error code.
     */
    ret_code_t (*init)(xinc_cli_transport_t const * p_transport,
                       void const *                p_config,
                       xinc_cli_transport_handler_t evt_handler,
                       void *                      p_context);

    /**
     * @brief Function for uninitializing the CLI transport interface.
     *
     * @param[in] p_transport  Pointer to the transfer instance.
     *
     * @return Standard error code.
     */
    ret_code_t (*uninit)(xinc_cli_transport_t const * p_transport);

    /**
     * @brief Function for reconfiguring the transport to work in blocking mode.
     *
     * @param p_transport  Pointer to the transfer instance.
     * @param blocking     If true, the transport is enabled in blocking mode.
     *
     * @return XINC_SUCCESS on successful enabling, error otherwise (also if not supported).
     */
    ret_code_t (*enable)(xinc_cli_transport_t const * p_transport,
                         bool                        blocking);

    /**
     * @brief Function for writing data to the transport interface.
     *
     * @param[in] p_transport  Pointer to the transfer instance.
     * @param[in] p_data       Pointer to the source buffer.
     * @param[in] length       Source buffer length.
     * @param[in] p_cnt        Pointer to the sent bytes counter.
     *
     * @return Standard error code.
     */
    ret_code_t (*write)(xinc_cli_transport_t const * p_transport,
                        const void *                p_data,
                        size_t                      length,
                        size_t *                    p_cnt);

    /**
     * @brief Function for reading data from the transport interface.
     *
     * @param[in] p_transport  Pointer to the transfer instance.
     * @param[in] p_data       Pointer to the destination buffer.
     * @param[in] length       Destination buffer length.
     * @param[in] p_cnt        Pointer to the received bytes counter.
     *
     * @return Standard error code.
     */
    ret_code_t (*read)(xinc_cli_transport_t const * p_transport,
                       void *                      p_data,
                       size_t                      length,
                       size_t *                    p_cnt);

} xinc_cli_transport_api_t;

struct xinc_cli_transport_s
{
    xinc_cli_transport_api_t const * p_api;
};

#if XINC_MODULE_ENABLED(XINC_CLI_HISTORY)
/**
 * @brief CLI history object header.
 */
typedef PACKED_STRUCT
{
    xinc_memobj_t * p_prev;  //!< Pointer to the next object.
    xinc_memobj_t * p_next;  //!< Pointer to the previous object.
    xinc_cli_cmd_len_t cmd_len; //!< Command length.
} xinc_cli_memobj_header_t;
#endif

#if XINC_MODULE_ENABLED(XINC_CLI_STATISTICS)
typedef struct
{
    uint32_t log_lost_cnt;  //!< Lost log counter.
} xinc_cli_statistics_t;
#endif

/**
 * @internal @brief Flags for internal CLI usage.
 */
typedef struct
{
    uint32_t insert_mode    : 1; //!< Enables or disables console insert mode for text introduction.
    uint32_t show_help      : 1; //!< Shows help if the command was called with -h or --help parameter.
    uint32_t use_colors     : 1; //!< Enables or disables colored syntax.
    uint32_t echo           : 1; //!< Enables or disables CLI echo.
    uint32_t processing     : 1; //!< CLI is executing process function.
    uint32_t tx_rdy         : 1;
    uint32_t last_nl        : 8; //!< The last received newline character.
} xinc_cli_flag_t;
STATIC_ASSERT(sizeof(xinc_cli_flag_t) == sizeof(uint32_t));

/**
 * @internal @brief Union for internal CLI usage.
 */
typedef union
{
    uint32_t value;
    xinc_cli_flag_t flag;
} xinc_cli_internal_t;

/**
 * @brief CLI instance context.
 */
typedef struct
{
    xinc_cli_state_t   state;            //!< Internal module state.
    xinc_cli_receive_t receive_state;    //!< Escape sequence indicator.

    xinc_cli_static_entry_t active_cmd;      //!< Currently executed command

    xinc_cli_vt100_ctx_t vt100_ctx;          //!< VT100 color and cursor position, terminal width.

    xinc_cli_cmd_len_t cmd_buff_len;         //!< Command length.
    xinc_cli_cmd_len_t cmd_buff_pos;         //!< Command buffer cursor position.

#if XINC_MODULE_ENABLED(XINC_CLI_WILDCARD)
    xinc_cli_cmd_len_t cmd_tmp_buff_len;     //!< Command length in tmp buffer
#endif

    char cmd_buff[XINC_CLI_CMD_BUFF_SIZE];       //!< Command input buffer.
    char temp_buff[XINC_CLI_CMD_BUFF_SIZE];      //!< Temporary buffer used by various functions.
    char printf_buff[XINC_CLI_PRINTF_BUFF_SIZE]; //!< Printf buffer size.

#if XINC_MODULE_ENABLED(XINC_CLI_STATISTICS)
    xinc_cli_statistics_t statistics;            //!< CLI statistics.
#endif

#if XINC_MODULE_ENABLED(XINC_CLI_USES_TASK_MANAGER)
    task_id_t     task_id;
#endif

#if XINC_MODULE_ENABLED(XINC_CLI_HISTORY)
    xinc_memobj_t * p_cmd_list_head;     //!< Pointer to the head of history list.
    xinc_memobj_t * p_cmd_list_tail;     //!< Pointer to the tail of history list.
    xinc_memobj_t * p_cmd_list_element;  //!< Pointer to an element of history list.
#endif
    volatile xinc_cli_internal_t internal;   //!< Internal CLI data
} xinc_cli_ctx_t;

//extern const xinc_log_backend_api_t xinc_log_backend_cli_api;

typedef struct
{
    xinc_queue_t const * p_queue;
    void *              p_context;
    xinc_cli_t const *   p_cli;
} xinc_cli_log_backend_t;

#if XINC_CLI_LOG_BACKEND && XINC_MODULE_ENABLED(XINC_LOG)
#define XINC_LOG_BACKEND_CLI_DEF(_name_, _queue_size_)                                          \
        XINC_QUEUE_DEF(xinc_log_entry_t,                                                         \
                      CONCAT_2(_name_, _queue),_queue_size_, XINC_QUEUE_MODE_NO_OVERFLOW);      \
        static xinc_cli_log_backend_t CONCAT_2(cli_log_backend,_name_) = {                      \
                .p_queue = &CONCAT_2(_name_, _queue),                                          \
        };                                                                                     \
        XINC_LOG_BACKEND_DEF(_name_, xinc_log_backend_cli_api, &CONCAT_2(cli_log_backend,_name_))

#define XINC_CLI_BACKEND_PTR(_name_) &CONCAT_2(_name_, _log_backend)
#else
#define XINC_LOG_BACKEND_CLI_DEF(_name_, _queue_sz_)
#define XINC_CLI_BACKEND_PTR(_name_) NULL
#endif

#if XINC_MODULE_ENABLED(XINC_CLI_HISTORY)
/* Header consists memory for cmd length and pointer to: prev and next element. */
#define XINC_CLI_HISTORY_HEADER_SIZE (sizeof(xinc_cli_memobj_header_t))

#define XINC_CLI_HISTORY_MEM_OBJ(name)                       \
    XINC_MEMOBJ_POOL_DEF(CONCAT_2(name, _cmd_hist_memobj),   \
                    XINC_CLI_HISTORY_HEADER_SIZE +           \
                    XINC_CLI_HISTORY_ELEMENT_SIZE,           \
                    XINC_CLI_HISTORY_ELEMENT_COUNT)

#define XINC_CLI_MEMOBJ_PTR(_name_) &CONCAT_2(_name_, _cmd_hist_memobj)
#else
#define XINC_CLI_MEMOBJ_PTR(_name_) NULL
#define XINC_CLI_HISTORY_MEM_OBJ(name)
#endif

/**
 * @brief CLI instance internals.
 *
 * @ref xinc_cli_t
 */
struct xinc_cli
{
    char const * const p_name; //!< Terminal name.

    xinc_cli_transport_t const * p_iface;        //!< Transport interface.
    xinc_cli_ctx_t *             p_ctx;          //!< Internal context.
    xinc_log_backend_t const *   p_log_backend;  //!< Logger backend.
    xinc_fprintf_ctx_t *         p_fprintf_ctx;  //!< fprintf context.
    xinc_memobj_pool_t const *   p_cmd_hist_mempool; //!< Memory reserved for commands history.
};

/**
 * @brief Macro for defining a command line interface instance.
 *
 * @param[in] name              Instance name.
 * @param[in] cli_prefix        CLI prefix string.
 * @param[in] p_transport_iface Pointer to the transport interface.
 * @param[in] newline_ch        Deprecated parameter, not used any more. Any uint8_t value can be used.
 * @param[in] log_queue_size    Logger processing queue size.
 */
#define XINC_CLI_DEF(name, cli_prefix, p_transport_iface, newline_ch, log_queue_size)    \
        static xinc_cli_t const name;                                            \
        static xinc_cli_ctx_t CONCAT_2(name, _ctx);                              \
        XINC_FPRINTF_DEF(CONCAT_2(name, _fprintf_ctx),                           \
                        &name,                                                  \
                        CONCAT_2(name, _ctx).printf_buff,                       \
                        XINC_CLI_PRINTF_BUFF_SIZE,                               \
                        false,                                                  \
                        xinc_cli_print_stream);                                  \
        XINC_LOG_BACKEND_CLI_DEF(CONCAT_2(name, _log_backend), log_queue_size);  \
        XINC_CLI_HISTORY_MEM_OBJ(name);                                          \
        /*lint -save -e31*/                                                     \
        static xinc_cli_t const name = {                                         \
            .p_name = cli_prefix,                                               \
            .p_iface = p_transport_iface,                                       \
            .p_ctx = &CONCAT_2(name, _ctx),                                     \
            .p_log_backend = XINC_CLI_BACKEND_PTR(name),                         \
            .p_fprintf_ctx = &CONCAT_2(name, _fprintf_ctx),                     \
            .p_cmd_hist_mempool = XINC_CLI_MEMOBJ_PTR(name),                     \
        } /*lint -restore*/

/**
 * @brief Function for initializing a transport layer and internal CLI state.
 *
 * @param[in] p_cli                 Pointer to CLI instance.
 * @param[in] p_transport_config    Configuration forwarded to the transport during initialization.
 * @param[in] use_colors            Enables colored prints.
 * @param[in] log_backend           If true, the console will be used as logger backend.
 * @param[in] init_lvl              Default severity level for the logger.
 *
 * @return Standard error code.
 */
ret_code_t xinc_cli_init(xinc_cli_t const *   p_cli,
                        void const *        p_transport_config,
                        bool                use_colors,
                        bool                log_backend,
                        xinc_log_severity_t  init_lvl);

ret_code_t xinc_cli_task_create(xinc_cli_t const * p_cli);

/**
 * @brief Function for uninitializing a transport layer and internal CLI state.
 *        If function returns XINC_ERROR_BUSY, you must call @ref xinc_cli_process before calling
 *        xinc_cli_uninit again.
 *
 * @param p_cli Pointer to CLI instance.
 *
 * @return Standard error code.
 */
ret_code_t xinc_cli_uninit(xinc_cli_t const * p_cli);

/**
 * @brief Function for starting CLI processing.
 *
 * @param p_cli Pointer to the CLI instance.
 *
 * @return Standard error code.
 */
ret_code_t xinc_cli_start(xinc_cli_t const * p_cli);

/**
 * @brief Function for stopping CLI processing.
 *
 * @param p_cli Pointer to CLI instance.
 *
 * @return Standard error code.
 */
ret_code_t xinc_cli_stop(xinc_cli_t const * p_cli);

/**
 * @brief CLI colors for @ref xinc_cli_fprintf function.
 */
#define XINC_CLI_DEFAULT  XINC_CLI_VT100_COLOR_DEFAULT    /**< Turn off character attributes. */
#define XINC_CLI_NORMAL   XINC_CLI_VT100_COLOR_WHITE      /**< Normal color printf.           */
#define XINC_CLI_INFO     XINC_CLI_VT100_COLOR_GREEN      /**< Info color printf.             */
#define XINC_CLI_OPTION   XINC_CLI_VT100_COLOR_CYAN       /**< Option color printf.           */
#define XINC_CLI_WARNING  XINC_CLI_VT100_COLOR_YELLOW     /**< Warning color printf.          */
#define XINC_CLI_ERROR    XINC_CLI_VT100_COLOR_RED        /**< Error color printf.            */

/**
 * @brief   Printf-like function which sends formatted data stream to the CLI.
 *          This function shall not be used outside of the CLI module or CLI command context.
 *
 * @param[in] p_cli Pointer to the CLI instance.
 * @param[in] color Printf color.
 * @param[in] p_fmt Format string.
 * @param[in] ...   List of parameters to print.
 */
void xinc_cli_fprintf(xinc_cli_t const *      p_cli,
                     xinc_cli_vt100_color_t  color,
                     char const *           p_fmt,
                                            ...);

/**
 * @brief Print an info message to the CLI.
 *
 * See @ref xinc_cli_fprintf.
 *
 * @param[in] _p_cli    Pointer to the CLI instance.
 * @param[in] _ft       Format string.
 * @param[in] ...       List of parameters to print.
 */
#define xinc_cli_info(_p_cli, _ft, ...) \
        xinc_cli_fprintf(_p_cli, XINC_CLI_INFO, _ft "\n", ##__VA_ARGS__)

/**
 * @brief Print a normal message to the CLI.
 *
 * See @ref xinc_cli_fprintf.
 *
 * @param[in] _p_cli    Pointer to the CLI instance.
 * @param[in] _ft       Format string.
 * @param[in] ...       List of parameters to print.
 */
#define xinc_cli_print(_p_cli, _ft, ...) \
        xinc_cli_fprintf(_p_cli, XINC_CLI_DEFAULT, _ft "\n", ##__VA_ARGS__)

/**
 * @brief Print a warning message to the CLI.
 *
 * See @ref xinc_cli_fprintf.
 *
 * @param[in] _p_cli    Pointer to the CLI instance.
 * @param[in] _ft       Format string.
 * @param[in] ...       List of parameters to print.
 */
#define xinc_cli_warn(_p_cli, _ft, ...) \
        xinc_cli_fprintf(_p_cli, XINC_CLI_WARNING, _ft "\n", ##__VA_ARGS__)

/**
 * @brief Print an error message to the CLI.
 *
 * See @ref xinc_cli_fprintf.
 *
 * @param[in] _p_cli    Pointer to the CLI instance.
 * @param[in] _ft       Format string.
 * @param[in] ...       List of parameters to print.
 */
#define xinc_cli_error(_p_cli, _ft, ...) \
        xinc_cli_fprintf(_p_cli, XINC_CLI_ERROR, _ft "\n", ##__VA_ARGS__)

/**
 * @brief Process function, which should be executed when data is ready in the transport interface.
 *
 * @param[in] p_cli Pointer to the CLI instance.
 */
void xinc_cli_process(xinc_cli_t const * p_cli);


/**
 * @brief Option descriptor.
 */
typedef struct xinc_cli_getopt_option
{
    char const * p_optname;         //!< Option long name.
    char const * p_optname_short;   //!< Option short name.
    char const * p_optname_help;    //!< Option help string.
} xinc_cli_getopt_option_t;


/**
 * @brief Option structure initializer @ref xinc_cli_getopt_option.
 *
 * @param[in] _p_optname    Option name long.
 * @param[in] _p_shortname  Option name short.
 * @param[in] _p_help       Option help string.
 */
#define XINC_CLI_OPT(_p_optname, _p_shortname, _p_help) { \
        .p_optname       = _p_optname,   \
        .p_optname_short = _p_shortname, \
        .p_optname_help  = _p_help,      \
}

/**
 * @brief Informs that a command has been called with -h or --help option.
 *
 * @param[in] p_cli Pointer to the CLI instance.
 *
 * @return          True if help has been requested.
 */
__STATIC_INLINE bool xinc_cli_help_requested(xinc_cli_t const * p_cli);

#ifndef SUPPRESS_INLINE_IMPLEMENTATION
__STATIC_INLINE bool xinc_cli_help_requested(xinc_cli_t const * p_cli)
{
    return p_cli->p_ctx->internal.flag.show_help;
}
#endif

/**
 * @brief       Prints the current command help.
 * @details     Function will print a help string with: the currently entered command, its options,
 *              and subcommands (if they exist).
 *
 * @param[in] p_cli     Pointer to the CLI instance.
 * @param[in] p_opt     Pointer to the optional option array.
 * @param[in] opt_len   Option array size.
 */
void xinc_cli_help_print(xinc_cli_t const *               p_cli,
                        xinc_cli_getopt_option_t const * p_opt,
                        size_t                          opt_len);

/**
 * @internal @brief This function shall not be used directly, it is required by the
 *                  xinc_fprintf module.
 *
 * @param[in] p_user_ctx    Pointer to the context for the CLI instance.
 * @param[in] p_data        Pointer to the data buffer.
 * @param[in] data_len      Data buffer size.
 */
void xinc_cli_print_stream(void const * p_user_ctx, char const * p_data, size_t data_len);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* XINC_CLI_H__ */
