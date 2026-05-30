#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "defines.h"
#include "render.h"
#include "service.h"

void render_line(char *buf, size_t len, int y) {
  move_cursor_yx(y, 0);
  ANSI_RESET_LINE;
  write(STDOUT_FILENO, buf, len);
}

void render_statusline(struct Context *ctx) {
  char buf[MAX_BUFFER_SIZE];
  char *mode_label;
  if (ctx->mode == MODE_NORMAL) {
    mode_label = "NORMAL";
  } else if (ctx->mode == MODE_INSERT) {
    mode_label = "INSERT";
  }
  int len = snprintf(buf, sizeof(buf), "-- %s -- %d/%d", mode_label, ctx->y, ctx->x);
  move_cursor_yx(ctx->win.ws_row - 1, 0);
  ANSI_RESET_LINE;
  write(STDOUT_FILENO, buf, len);
}

void render_buf(struct Context *ctx) {
  char **frame = (char **)malloc(ctx->win.ws_row * sizeof(char *));
  for (int i = 0; i < ctx->win.ws_row; i++) {
    frame[i] = (char *)malloc(ctx->win.ws_col);
  }
  configure_frame(ctx, frame);
  if (ctx->prev_frame != NULL) {
    for (int i = 0; i < ctx->win.ws_row; i++) {
      for (int j = 0; j < ctx->win.ws_col; j++) {
        if (frame[i][j] != ctx->prev_frame[i][j]) {
          render_line(frame[i], ctx->win.ws_col, i);
          break;
        }
      }
    }
    free(ctx->prev_frame);
  }
  ctx->prev_frame = frame;
}

void render(struct Context *ctx) {
  render_buf(ctx);
  render_statusline(ctx);
  move_cursor_yx(ctx->y - ctx->offsetY, ctx->x - ctx->offsetX);
}
