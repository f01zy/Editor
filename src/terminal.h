#ifndef TERMINAL_INCLUDED
#define TERMINAL_INCLUDED

#include "types.h"

void set_cursor_style(enum CursorStyle type);
void set_render_mode(enum RenderMode mode);
void move_cursor_yx(int y, int x);

#endif
