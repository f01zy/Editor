#ifndef DEFINES_INCLUDED
#define DEFINES_INCLUDED

#define ADDITIONAL_REALLOCATION     16
#define MAX_BUFFER_SIZE             8192
#define KEY_ENTER                   13
#define KEY_BACKSPACE               127
#define KEY_ESCAPE                  27
#define KEY_TAB                     9
#define ANSI_RESET_SCREEN           write(STDOUT_FILENO, "\x1b[2J", 4)
#define ANSI_RESET_LINE_FROM_CURSOR write(STDOUT_FILENO, "\x1b[0K", 4)
#define ANSI_RESET_LINE_TO_CURSOR   write(STDOUT_FILENO, "\x1b[0K", 4)
#define ANSI_MOVE_CURSOR_YX         "\x1b[%d;%dH"
#define ANSI_CURSOR_TYPE            "\x1b[%d q"
#define ANSI_RENDER_MODE            "\x1b[%dm"
#define MIN(A, B)                   ((A) > (B) ? (B) : (A))
#define MAX(A, B)                   ((A) < (B) ? (B) : (A))

#endif
