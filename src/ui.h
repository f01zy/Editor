#ifndef UI_INCLUDED
#define UI_INCLUDED

#include "types.h"

int get_line_number_margin(struct Context *ctx);
int get_statusline_margin(struct Context *ctx);
int get_tabmenu_margin(struct Context *ctx);

int get_buffer_width(struct Context *ctx);
int get_buffer_height(struct Context *ctx);

#endif
