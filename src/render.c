#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "defines.h"
#include "render.h"
#include "service.h"

void render_line(struct Context *ctx, char *buf, size_t len, int y) {
  move_cursor_yx(y, 0);
  ANSI_RESET_LINE_FROM_CURSOR;
  write(STDOUT_FILENO, buf, len);
}

void render_line_number(struct Context *ctx, char **frame) {
  int margin = get_line_number_margin(ctx);
  char buf[MAX_BUFFER_SIZE];
  for (int i = 0; i < ctx->win.ws_row; i++) {
    int y = i + ctx->offsetY;
    if (y >= ctx->len) break;
    int len = snprintf(buf, sizeof(buf), "%d", y + 1);
    memcpy(frame[i], buf, len);
    for (int j = len; j < margin; j++) {
      frame[i][j] = ' ';
    }
  }
}

void render_statusline(struct Context *ctx, char **frame) {
  char buf[MAX_BUFFER_SIZE];
  char *mode_label;
  if (ctx->mode == MODE_NORMAL) mode_label = "NORMAL";
  if (ctx->mode == MODE_INSERT) mode_label = "INSERT";
  int len = snprintf(buf, sizeof(buf), "-- %s -- %d/%d", mode_label, ctx->y + 1, ctx->x + 1);
  int y   = ctx->win.ws_row - 1;
  memcpy(frame[y], buf, len);
  for (int i = len; i < ctx->win.ws_col; i++) {
    frame[y][i] = ' ';
  }
}

void render_buf(struct Context *ctx, char **frame) {
  int margin = get_line_number_margin(ctx);
  for (int i = 0; i < ctx->win.ws_row; i++) {
    int y = ctx->offsetY + i;
    if (y >= ctx->len) {
      for (int j = margin; j < ctx->win.ws_col; j++) {
        frame[i][j] = ' ';
      }
      continue;
    }
    struct Line *line = ctx->buf[y];
    for (int j = margin; j < ctx->win.ws_col; j++) {
      int x       = ctx->offsetX + j - margin;
      frame[i][j] = x < line->len ? line->buf[x] : ' ';
    }
  }
}

void render(struct Context *ctx) {
  char **frame = (char **)malloc(ctx->win.ws_row * sizeof(char *));
  for (int i = 0; i < ctx->win.ws_row; i++) {
    frame[i] = (char *)malloc(ctx->win.ws_col);
  }
  render_line_number(ctx, frame);
  render_buf(ctx, frame);
  render_statusline(ctx, frame);
  for (int i = 0; i < ctx->win.ws_row; i++) {
    for (int j = 0; j < ctx->win.ws_col; j++) {
      if (ctx->prev_frame == NULL || frame[i][j] != ctx->prev_frame[i][j]) {
        render_line(ctx, frame[i], ctx->win.ws_col, i);
        break;
      }
    }
  }
  free(ctx->prev_frame);
  ctx->prev_frame = frame;
  move_cursor_yx(ctx->y - ctx->offsetY, ctx->x - ctx->offsetX + get_line_number_margin(ctx));
}
