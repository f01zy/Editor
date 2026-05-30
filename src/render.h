#ifndef RENDER_INCLUDED
#define RENDER_INCLUDED

#include "types.h"

void render_line_number(struct Context *ctx, char **frame);
void render_line(struct Context *ctx, char *buf, size_t len, int y);
void render_statusline(struct Context *ctx, char **frame);
void render_buf(struct Context *ctx, char **frame);
void render(struct Context *ctx);

#endif
