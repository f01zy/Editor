#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "buffer.h"

size_t get_max_x(struct Line *line) { return line->len ? line->len - 1 : 0; }

void add_line(struct Context *ctx, char *data, int y) {
  if (y > ctx->len) return;
  if (++ctx->len > ctx->size) {
    ctx->size = ctx->len + ADDITIONAL_REALLOCATION;
    ctx->buf  = (struct Line **)xrealloc(ctx->buf, ctx->size * sizeof(struct Line *));
  }
  struct Line *line = (struct Line *)xmalloc(sizeof(struct Line));
  if (!data) {
    line->buf  = (char *)xmalloc(1);
    line->len  = 0;
    line->size = 1;
  } else {
    int len    = strlen(data);
    line->buf  = (char *)xmalloc(len + 1);
    line->len  = len;
    line->size = len + 1;
    memcpy(line->buf, data, len);
  }
  for (int i = ctx->len - 1; i > y; i--) {
    ctx->buf[i] = ctx->buf[i - 1];
  }
  ctx->buf[y] = line;
}

void remove_line(struct Context *ctx, int y) {
  if (y >= ctx->len) return;
  struct Line *line = ctx->buf[y];
  free(line->buf);
  free(line);
  for (int i = y; i < --ctx->len; i++) {
    ctx->buf[i] = ctx->buf[i + 1];
  }
}

void write_to_line(struct Context *ctx, int y, int x, char ch) {
  if (y >= ctx->len) return;
  struct Line *line = ctx->buf[y];
  if (x > line->len) return;
  if (++line->len + 1 > line->size) {
    line->size = line->len + ADDITIONAL_REALLOCATION;
    line->buf  = (char *)xrealloc(line->buf, line->size);
  }
  for (int i = line->len - 1; i > x; i--) {
    line->buf[i] = line->buf[i - 1];
  }
  line->buf[x]         = ch;
  line->buf[line->len] = '\0';
}

enum RemoveResult remove_from_line(struct Context *ctx, int y, int x) {
  if (y >= ctx->len) return REMOVE_NOTHING;
  struct Line *line = ctx->buf[y];
  if (x > line->len || (!x && !y)) return REMOVE_NOTHING;
  if (!x) {
    struct Line *prev = ctx->buf[y - 1];
    if (line->len > 0) {
      int len = prev->len + line->len;
      if (prev->size - prev->len - 1 < line->len) {
        prev->size = len;
        prev->buf  = (char *)xrealloc(prev->buf, prev->size);
      }
      memcpy(prev->buf + prev->len, line->buf, line->len);
      prev->len            = len;
      prev->buf[prev->len] = '\0';
    }
    remove_line(ctx, y);
    return REMOVE_LINE;
  } else {
    for (int i = x - 1; i < --line->len; i++) {
      line->buf[i] = line->buf[i + 1];
    }
    line->buf[line->len] = '\0';
    return REMOVE_CHAR;
  }
}

void line_break(struct Context *ctx) {
  add_line(ctx, NULL, ctx->y + 1);
  struct Line *line = ctx->buf[ctx->y];
  struct Line *next = ctx->buf[ctx->y + 1];
  int diff          = line->len - ctx->x;
  if (diff > 0) {
    next->size = diff + 1;
    next->buf  = (char *)xrealloc(next->buf, next->size);
    next->len  = diff;
    memcpy(next->buf, line->buf + ctx->x, diff);
    line->len            = ctx->x;
    line->buf[line->len] = '\0';
    next->buf[next->len] = '\0';
  }
}
