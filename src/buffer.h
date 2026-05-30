#ifndef BUFFER_INCLUDED
#define BUFFER_INCLUDED

#include "defines.h"
#include "memory.h"
#include "types.h"

size_t get_max_x(struct Line *line);

enum RemoveResult remove_from_line(struct Context *ctx, int y, int x);
void add_line(struct Context *ctx, char *data, int y);
void remove_line(struct Context *ctx, int y);
void write_to_line(struct Context *ctx, int y, int x, char ch);
void line_break(struct Context *ctx);

#endif
