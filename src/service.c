#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "buffer.h"
#include "service.h"
#include "terminal.h"
#include "types.h"

void *xmalloc(size_t size) {
  void *ptr = malloc(size);
  if (!ptr) {
    perror("Fatal: memory allocation failed");
    exit(1);
  }
  return ptr;
}

void *xrealloc(void *ptr, size_t size) {
  void *new_ptr = realloc(ptr, size);
  if (!new_ptr && size > 0) {
    perror("Fatal: memory reallocation failed");
    exit(1);
  }
  return new_ptr;
}

void configure_context(struct Context *ctx, char *file_path) {
  if (!file_path) return;
  ioctl(STDIN_FILENO, TIOCGWINSZ, &ctx->win);
  tcgetattr(STDIN_FILENO, &ctx->backup);
  struct UI ui = {
      .is_line_number = true,
  };

  struct Line *cmd = (struct Line *)xmalloc(sizeof(struct Line));
  cmd->size        = ctx->win.ws_col;
  cmd->buf         = (char *)xmalloc(cmd->size);
  cmd->buf[0]      = '\0';

  int path_len   = strlen(file_path);
  ctx->curr_path = (char *)xmalloc(path_len + 1);
  memcpy(ctx->curr_path, file_path, path_len);
  ctx->curr_path[path_len] = '\0';

  ctx->cmd           = cmd;
  ctx->ui            = ui;
  ctx->conf          = ctx->backup;
  ctx->conf.c_iflag |= IXOFF;
  ctx->conf.c_iflag &= ~ICRNL;
  ctx->conf.c_lflag &= ~ECHO;
  ctx->conf.c_lflag &= ~ISIG;
  ctx->conf.c_lflag &= ~ICANON;
  ctx->mode          = MODE_NORMAL;

  open_file(ctx);
  tcsetattr(STDIN_FILENO, TCSANOW, &ctx->conf);
}

void clear_cmd(struct Context *ctx) {
  ctx->cmd->buf[0] = '\0';
  ctx->cmd->len    = 0;
}

int get_line_number_margin(struct Context *ctx) {
  if (!ctx->ui.is_line_number) return 0;
  int len = ctx->len, margin = 0;
  while (len) {
    margin++;
    len /= 10;
  }
  return margin + 1;
}

void free_resources(struct Context *ctx) {
  for (int i = 0; i < ctx->len; i++) {
    struct Line *line = ctx->buf[i];
    free(line->buf);
    free(line);
  }
  for (int i = 0; i < ctx->win.ws_row; i++) {
    free(ctx->prev_frame[i]);
  }
  free(ctx->buf);
  free(ctx->curr_path);
  free(ctx->prev_frame);
  free(ctx->cmd->buf);
  free(ctx->cmd);
}

void check_offset(struct Context *ctx) {
  int margin = get_line_number_margin(ctx);

  if (ctx->x < ctx->offsetX) {
    ctx->offsetX = ctx->x;
  } else if (ctx->x >= ctx->offsetX + ctx->win.ws_col - margin) {
    ctx->offsetX = ctx->x - (ctx->win.ws_col - margin) + 1;
  }

  if (ctx->y < ctx->offsetY) {
    ctx->offsetY = ctx->y;
  } else if (ctx->y >= ctx->offsetY + ctx->win.ws_row) {
    ctx->offsetY = ctx->y - ctx->win.ws_row + 1;
  }
}

void change_mode(struct Context *ctx, enum Mode mode) {
  enum CursorStyle style;
  if (mode == MODE_NORMAL) style = CURSOR_BLOCK_STATIC;
  if (mode == MODE_INSERT) style = CURSOR_LINE_STATIC;
  ctx->mode = mode;
  set_cursor_style(style);
}

void set_status(struct Context *ctx, char *status) {
  if (!status) return;
  int len   = strlen(status);
  char *buf = (char *)xmalloc(len + 1);
  memcpy(buf, status, len);
  buf[len] = '\0';
  free(ctx->status);
  ctx->status = buf;
}

void clear_status(struct Context *ctx) {
  free(ctx->status);
  ctx->status = NULL;
}
