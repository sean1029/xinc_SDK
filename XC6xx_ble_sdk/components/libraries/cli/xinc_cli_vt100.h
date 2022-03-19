/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#ifndef XINC_CLI_VT100_H__
#define XINC_CLI_VT100_H__

#ifdef __cplusplus
extern "C" {
#endif

#define XINC_CLI_VT100_ASCII_ESC     (0x1b)
#define XINC_CLI_VT100_ASCII_DEL     (0x7F)
#define XINC_CLI_VT100_ASCII_BSPACE  (0x08)
#define XINC_CLI_VT100_ASCII_CTRL_A  (0x1)
#define XINC_CLI_VT100_ASCII_CTRL_C  (0x03)
#define XINC_CLI_VT100_ASCII_CTRL_E  (0x5)
#define XINC_CLI_VT100_ASCII_CTRL_L  (0x0C)
#define XINC_CLI_VT100_ASCII_CTRL_U  (0x15)
#define XINC_CLI_VT100_ASCII_CTRL_W  (0x17)


#define XINC_CLI_VT100_SETNL                                             \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '2', '0', 'h', '\0'               \
    } /* Set new line mode */
#define XINC_CLI_VT100_SETAPPL                                           \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '?', '1', 'h', '\0'               \
    } /* Set cursor key to application */
#define XINC_CLI_VT100_SETCOL_132                                        \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '?', '3', 'h', '\0'               \
    } /* Set number of columns to 132 */
#define XINC_CLI_VT100_SETSMOOTH                                         \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '?', '4', 'h', '\0'               \
    } /* Set smooth scrolling */
#define XINC_CLI_VT100_SETREVSCRN                                        \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '?', '5', 'h', '\0'               \
    } /* Set reverse video on screen */
#define XINC_CLI_VT100_SETORGREL                                         \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '?', '6', 'h', '\0'               \
    } /* Set origin to relative */
#define XINC_CLI_VT100_SETWRAP_ON                                        \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '?', '7', 'h', '\0'               \
    } /* Set auto-wrap mode */
#define XINC_CLI_VT100_SETWRAP_OFF                                       \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '?', '7', 'l', '\0'               \
    } /* Set auto-wrap mode */

#define XINC_CLI_VT100_SETREP                                            \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '?', '8', 'h', '\0'               \
    } /* Set auto-repeat mode */
#define XINC_CLI_VT100_SETINTER                                          \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '?', '9', 'h', '\0'               \
    } /* Set interlacing mode */

#define XINC_CLI_VT100_SETLF                                             \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '2', '0', 'l', '\0'               \
    } /* Set line feed mode */
#define XINC_CLI_VT100_SETCURSOR                                         \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '?', '1', 'l', '\0'               \
    } /* Set cursor key to cursor */
#define XINC_CLI_VT100_SETVT52                                           \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '?', '2', 'l', '\0'               \
    } /* Set VT52 (versus ANSI) */
#define XINC_CLI_VT100_SETCOL_80                                         \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '?', '3', 'l', '\0'               \
    } /* Set number of columns to 80 */
#define XINC_CLI_VT100_SETJUMP                                           \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '?', '4', 'l', '\0'               \
    } /* Set jump scrolling */
#define XINC_CLI_VT100_SETNORMSCRN                                       \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '?', '5', 'l', '\0'               \
    } /* Set normal video on screen */
#define XINC_CLI_VT100_SETORGABS                                         \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '?', '6', 'l', '\0'               \
    } /* Set origin to absolute */
#define XINC_CLI_VT100_RESETWRAP                                         \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '?', '7', 'l', '\0'               \
    } /* Reset auto-wrap mode */
#define XINC_CLI_VT100_RESETREP                                          \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '?', '8', 'l', '\0'               \
    } /* Reset auto-repeat mode */
#define XINC_CLI_VT100_RESETINTER                                        \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '?', '9', 'l', '\0'               \
    } /* Reset interlacing mode */

#define XINC_CLI_VT100_ALTKEYPAD                                         \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '=', '\0'                              \
    } /* Set alternate keypad mode */
#define XINC_CLI_VT100_NUMKEYPAD                                         \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '>', '\0'                              \
    } /* Set numeric keypad mode */

#define XINC_CLI_VT100_SETUKG0                                           \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '(', 'A', '\0'                         \
    } /* Set United Kingdom G0 character set */
#define XINC_CLI_VT100_SETUKG1                                           \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, ')', 'A', '\0'                         \
    } /* Set United Kingdom G1 character set */
#define XINC_CLI_VT100_SETUSG0                                           \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '(', 'B', '\0'                         \
    } /* Set United States G0 character set */
#define XINC_CLI_VT100_SETUSG1                                           \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, ')', 'B', '\0'                         \
    } /* Set United States G1 character set */
#define XINC_CLI_VT100_SETSPECG0                                         \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '(', '0', '\0'                         \
    } /* Set G0 special chars. & line set */
#define XINC_CLI_VT100_SETSPECG1                                         \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, ')', '0', '\0'                         \
    } /* Set G1 special chars. & line set */
#define XINC_CLI_VT100_SETALTG0                                          \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '(', '1', '\0'                         \
    } /* Set G0 alternate character ROM */
#define XINC_CLI_VT100_SETALTG1                                          \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, ')', '1', '\0'                         \
    } /* Set G1 alternate character ROM */
#define XINC_CLI_VT100_SETALTSPECG0                                      \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '(', '2', '\0'                         \
    } /* Set G0 alt char ROM and spec. graphics */
#define XINC_CLI_VT100_SETALTSPECG1                                      \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, ')', '2', '\0'                         \
    } /* Set G1 alt char ROM and spec. graphics */

#define XINC_CLI_VT100_SETSS2                                            \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'N', '\0'                              \
    } /* Set single shift 2 */
#define XINC_CLI_VT100_SETSS3                                            \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'O', '\0'                              \
    } /* Set single shift 3 */

#define XINC_CLI_VT100_MODESOFF                                          \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', 'm', '\0'                         \
    } /* Turn off character attributes */
#define XINC_CLI_VT100_MODESOFF_                                         \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '0', 'm', '\0'                    \
    } /* Turn off character attributes */
#define XINC_CLI_VT100_BOLD                                              \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '1', 'm', '\0'                    \
    } /* Turn bold mode on */
#define XINC_CLI_VT100_LOWINT                                            \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '2', 'm', '\0'                    \
    } /* Turn low intensity mode on */
#define XINC_CLI_VT100_UNDERLINE                                         \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '4', 'm', '\0'                    \
    } /* Turn underline mode on */
#define XINC_CLI_VT100_BLINK                                             \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '5', 'm', '\0'                    \
    } /* Turn blinking mode on */
#define XINC_CLI_VT100_REVERSE                                           \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '7', 'm', '\0'                    \
    } /* Turn reverse video on */
#define XINC_CLI_VT100_INVISIBLE                                         \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '8', 'm', '\0'                    \
    } /* Turn invisible text mode on */

#define XINC_CLI_VT100_SEI2CN(t, b)                                      \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', (t), ';', (b), 'r', '\0'          \
    } /* Set top and bottom line#s of a window */

#define XINC_CLI_VT100_CURSORUP(n)                                       \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', (n), 'A', '\0'                    \
    } /* Move cursor up n lines */
#define XINC_CLI_VT100_CURSORDN(n)                                       \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', (n), 'B', '\0'                    \
    } /* Move cursor down n lines */
#define XINC_CLI_VT100_CURSORRT(n)                                       \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', (n), 'C', '\0'                    \
    } /* Move cursor right n lines */
#define XINC_CLI_VT100_CURSORLF(n)                                       \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', (n), 'D', '\0'                    \
    } /* Move cursor left n lines */
#define XINC_CLI_VT100_CURSORHOME                                        \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', 'H', '\0'                         \
    } /* Move cursor to upper left corner */
#define XINC_CLI_VT100_CURSORHOME_                                       \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', ';', 'H', '\0'                    \
    } /* Move cursor to upper left corner */
#define XINC_CLI_VT100_CURSORPOS(v, h)                                   \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', (v), ';', (h), 'H', '\0'          \
    } /* Move cursor to screen location v,h */

#define XINC_CLI_VT100_HVHOME                                            \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', 'f', '\0'                         \
    } /* Move cursor to upper left corner */
#define XINC_CLI_VT100_HVHOME_                                           \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', ';', 'f', '\0'                    \
    } /* Move cursor to upper left corner */
#define XINC_CLI_VT100_HVPOS(v, h)                                       \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', (v), ';', (h), 'f', '\0'          \
    } /* Move cursor to screen location v,h */
#define XINC_CLI_VT100_INDEX                                             \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'D', '\0'                              \
    } /* Move/scroll window up one line */
#define XINC_CLI_VT100_REVINDEX                                          \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'M', '\0'                              \
    } /* Move/scroll window down one line */
#define XINC_CLI_VT100_NEXTLINE                                          \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'E', '\0'                              \
    } /* Move to next line */
#define XINC_CLI_VT100_SAVECURSOR                                        \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '7', '\0'                              \
    } /* Save cursor position and attributes */
#define XINC_CLI_VT100_RESTORECURSOR                                     \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '8', '\0'                              \
    } /* Restore cursor position and attribute */

#define XINC_CLI_VT100_TABSET                                            \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'H', '\0'                              \
    } /* Set a tab at the current column */
#define XINC_CLI_VT100_TABCLR                                            \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', 'g', '\0'                         \
    } /* Clear a tab at the current column */
#define XINC_CLI_VT100_TABCLR_                                           \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '0', 'g', '\0'                    \
    } /* Clear a tab at the current column */
#define XINC_CLI_VT100_TABCLRALL                                         \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '3', 'g', '\0'                    \
    } /* Clear all tabs */

#define XINC_CLI_VT100_DHTOP                                             \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '#', '3', '\0'                         \
    } /* Double-height letters, top half */
#define XINC_CLI_VT100_DHBOT                                             \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '#', '4', '\0'                         \
    } /* Double-height letters, bottom hal */
#define XINC_CLI_VT100_SWSH                                              \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '#', '5', '\0'                         \
    } /* Single width, single height letters */
#define XINC_CLI_VT100_DWSH                                              \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '#', '6', '\0'                         \
    } /* Double width, single height letters */

#define XINC_CLI_VT100_CLEAREOL                                          \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', 'K', '\0'                         \
    } /* Clear line from cursor right */
#define XINC_CLI_VT100_CLEAREOL_                                         \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '0', 'K', '\0'                    \
    } /* Clear line from cursor right */
#define XINC_CLI_VT100_CLEARBOL                                          \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '1', 'K', '\0'                    \
    } /* Clear line from cursor left */
#define XINC_CLI_VT100_CLEARLINE                                         \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '2', 'K', '\0'                    \
    } /* Clear entire line */

#define XINC_CLI_VT100_CLEAREOS                                          \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', 'J', '\0'                         \
    } /* Clear screen from cursor down */
#define XINC_CLI_VT100_CLEAREOS_                                         \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '0', 'J', '\0'                    \
    } /* Clear screen from cursor down */
#define XINC_CLI_VT100_CLEARBOS                                          \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '1', 'J', '\0'                    \
    } /* Clear screen from cursor up */
#define XINC_CLI_VT100_CLEARSCREEN                                       \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '2', 'J', '\0'                    \
    } /* Clear entire screen */

#define XINC_CLI_VT100_DEVSTAT                                           \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '5', 'n', '\0'                         \
    } /* Device status report */
#define XINC_CLI_VT100_TERMOK                                            \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '0', 'n', '\0'                         \
    } /* Response: terminal is OK */
#define XINC_CLI_VT100_TERMNOK                                           \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '3', 'n', '\0'                         \
    } /* Response: terminal is not OK */

#define XINC_CLI_VT100_GETCURSOR                                         \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '6', 'n', '\0'                    \
    } /* Get cursor position */
#define XINC_CLI_VT100_CURSORPOSAT                                       \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, (v), ';', (h), 'R', '\0'               \
    } /* Response: cursor is at v,h */

#define XINC_CLI_VT100_IDENT                                             \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', 'c', '\0'                         \
    } /* Identify what terminal type */
#define XINC_CLI_VT100_IDENT_                                            \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '0', 'c', '\0'                    \
    } /* Identify what terminal type */
#define XINC_CLI_VT100_GETTYPE                                           \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '?', '1', ';', (n), '0', 'c', '\0'\
    } /* Response: terminal type code n */

#define XINC_CLI_VT100_RESET                                             \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'c', '\0'                              \
    } /*  Reset terminal to initial state */

#define XINC_CLI_VT100_ALIGN                                             \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '#', '8', '\0'                         \
    } /* Screen alignment display */
#define XINC_CLI_VT100_TESTPU                                            \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '2', ';', '1', 'y', '\0'          \
    } /* Confidence power up test */
#define XINC_CLI_VT100_TESTLB                                            \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '2', ';', '2', 'y', '\0'          \
    } /* Confidence loopback test */
#define XINC_CLI_VT100_TESTPUREP                                         \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '2', ';', '9', 'y', '\0'          \
    } /* Repeat power up test */
#define XINC_CLI_VT100_TESTLBREP                                         \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '2', ';', '1', '0', 'y', '\0'     \
    } /* Repeat loopback test */

#define XINC_CLI_VT100_LEDSOFF                                           \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '0', 'q', '\0'                    \
    } /* Turn off all four leds */
#define XINC_CLI_VT100_LED1                                              \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '1', 'q', '\0'                    \
    } /* Turn on LED #1 */
#define XINC_CLI_VT100_LED2                                              \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '2', 'q', '\0'                    \
    } /* Turn on LED #2 */
#define XINC_CLI_VT100_LED3                                              \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '3', 'q', '\0'                    \
    } /* Turn on LED #3 */
#define XINC_CLI_VT100_LED4                                              \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, '[', '4', 'q', '\0'                    \
    } /* Turn on LED #4 */

/* Function Keys */

#define XINC_CLI_VT100_PF1                                               \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'O', 'P', '\0'                         \
    }
#define XINC_CLI_VT100_PF2                                               \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'O', 'Q', '\0'                         \
    }
#define XINC_CLI_VT100_PF3                                               \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'O', 'R', '\0'                         \
    }
#define XINC_CLI_VT100_PF4                                               \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'O', 'S', '\0'                         \
    }

/* Arrow keys */

#define XINC_CLI_VT100_UP_RESET                                          \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'A', '\0'                              \
    }
#define XINC_CLI_VT100_UP_SET                                            \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'O', 'A', '\0'                         \
    }
#define XINC_CLI_VT100_DOWN_RESET                                        \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'B', '\0'                              \
    }
#define XINC_CLI_VT100_DOWN_SET                                          \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'O', 'B', '\0'                         \
    }
#define XINC_CLI_VT100_RIGHT_RESET                                       \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'C', '\0'                              \
    }
#define XINC_CLI_VT100_RIGHT_SET                                         \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'O', 'C', '\0'                         \
    }
#define XINC_CLI_VT100_LEFT_RESET                                        \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'D', '\0'                              \
    }
#define XINC_CLI_VT100_LEFT_SET                                          \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'O', 'D', '\0'                         \
    }

/* Numeric Keypad Keys */

#define XINC_CLI_VT100_NUMERIC_0                                         \
    {                                                                   \
        '0', '\0'                                                       \
    }
#define XINC_CLI_VT100_ALT_0                                             \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'O', 'p', '\0'                         \
    }
#define XINC_CLI_VT100_NUMERIC_1                                         \
    {                                                                   \
        '1', '\0'                                                       \
    }
#define XINC_CLI_VT100_ALT_1                                             \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'O', 'q', '\0'                         \
    }
#define XINC_CLI_VT100_NUMERIC_2                                         \
    {                                                                   \
        '2', '\0'                                                       \
    }
#define XINC_CLI_VT100_ALT_2                                             \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'O', 'r', '\0'                         \
    }
#define XINC_CLI_VT100_NUMERIC_3                                         \
    {                                                                   \
        '3', '\0'                                                       \
    }
#define XINC_CLI_VT100_ALT_3                                             \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'O', 's', '\0'                         \
    }
#define XINC_CLI_VT100_NUMERIC_4                                         \
    {                                                                   \
        '4', '\0'                                                       \
    }
#define XINC_CLI_VT100_ALT_4                                             \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'O', 't', '\0'                         \
    }
#define XINC_CLI_VT100_NUMERIC_5                                         \
    {                                                                   \
        '5', '\0'                                                       \
    }
#define XINC_CLI_VT100_ALT_5                                             \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'O', 'u', '\0'                         \
    }
#define XINC_CLI_VT100_NUMERIC_6                                         \
    {                                                                   \
        '6', '\0'                                                       \
    }
#define XINC_CLI_VT100_ALT_6                                             \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'O', 'v', '\0'                         \
    }
#define XINC_CLI_VT100_NUMERIC_7                                         \
    {                                                                   \
        '7', '\0'                                                       \
    }
#define XINC_CLI_VT100_ALT_7                                             \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'O', 'w', '\0'                         \
    }
#define XINC_CLI_VT100_NUMERIC_8                                         \
    {                                                                   \
        '8', '\0'                                                       \
    }
#define XINC_CLI_VT100_ALT_8                                             \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'O', 'x', '\0'                         \
    }
#define XINC_CLI_VT100_NUMERIC_9                                         \
    {                                                                   \
        '9', '\0'
#define XINC_CLI_VT100_ALT_9                                             \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'O', 'y'                               \
    }
#define XINC_CLI_VT100_NUMERIC_MINUS                                     \
    {                                                                   \
        '-', '\0'                                                       \
    }
#define XINC_CLI_VT100_ALT_MINUS                                         \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'O', 'm', '\0'                         \
    }
#define XINC_CLI_VT100_NUMERIC_COMMA                                     \
    {                                                                   \
        ',', '\0'                                                       \
    }
#define XINC_CLI_VT100_ALT_COMMA                                         \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'O', 'l', '\0'                         \
    }
#define XINC_CLI_VT100_NUMERIC_PERIOD                                    \
    {                                                                   \
        '.', '\0'                                                       \
    }
#define XINC_CLI_VT100_ALT_PERIOD                                        \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'O', 'n', '\0'                         \
    }
#define XINC_CLI_VT100_NUMERIC_ENTER                                     \
    {                                                                   \
        ASCII_CR                                                        \
    }
#define XINC_CLI_VT100_ALT_ENTER                                         \
    {                                                                   \
        XINC_CLI_VT100_ASCII_ESC, 'O', 'M', '\0'                         \
    }

#define XINC_CLI_VT100_COLOR(__col)                                            \
    {                                                                         \
        XINC_CLI_VT100_ASCII_ESC, '[', '1', ';', '3', '0' + (__col), 'm', '\0' \
    }
#define XINC_CLI_VT100_BGCOLOR(__col)                                          \
    {                                                                         \
        XINC_CLI_VT100_ASCII_ESC, '[', '4', '0' + (__col), 'm', '\0'           \
    }

#ifdef __cplusplus
}
#endif

#endif /* XINC_CLI_VT100_H__ */

