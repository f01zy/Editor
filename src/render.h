#ifndef RENDER_INCLUDED
#define RENDER_INCLUDED

#include "types.h"

void render_line(char *buf, size_t len, int y);
void render_statusline(struct Context *ctx);
void render_buf(struct Context *ctx);
void render(struct Context *ctx);

#endif
