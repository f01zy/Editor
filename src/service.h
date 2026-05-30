#ifndef SERVICE_INCLUDED
#define SERVICE_INCLUDED

#include "types.h"

void check_offset(struct Context *ctx);
size_t get_max_x(struct Line *line);
void change_mode(struct Context *ctx, enum Mode mode);
void set_cursor_style(enum CursorStyle type);
void move_cursor_yx(int y, int x);

void configure_context(struct Context *ctx);
void configure_frame(struct Context *ctx, char **frame);

enum RemoveResult remove_from_line(struct Context *ctx, int y, int x);
void add_line(struct Context *ctx, int y);
void remove_line(struct Context *ctx, int y);
void write_to_line(struct Context *ctx, int y, int x, char ch);
void line_break(struct Context *ctx);

#endif
