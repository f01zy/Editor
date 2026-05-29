#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

#include "defines.h"
#include "render.h"
#include "service.h"
#include "types.h"

struct Context ctx;

void handle_normal_mode(struct Context *ctx, int ch) {
  struct Line *line = ctx->buf[ctx->y];
  size_t len        = get_max_x(line);

  switch (ch) {
  case 'h':
    if (ctx->x == 0 && ctx->y == 0) break;
    if (ctx->x == 0) {
      ctx->y--;
      ctx->x = get_max_x(ctx->buf[ctx->y - 1]);
    } else {
      ctx->x--;
    }
    break;

  case 'l':
    if (ctx->x == len && ctx->y == ctx->len - 1) break;
    if (ctx->x == len) {
      ctx->x = 0;
      ctx->y++;
    } else {
      ctx->x++;
    }
    break;

  case 'j':
    if (ctx->y == ctx->len - 1) break;
    ctx->y++;
    ctx->x = MIN(ctx->x, ctx->buf[ctx->y]->len);
    break;

  case 'k':
    if (ctx->y == 0) break;
    ctx->y--;
    ctx->x = MIN(ctx->x, ctx->buf[ctx->y]->len);
    break;

  case 'i':
    change_mode(ctx, MODE_INSERT);
    break;

  case 'a':
    if (line->len > 0) ctx->x++;
    change_mode(ctx, MODE_INSERT);
    break;

  case '$':
    ctx->x = len;
    break;
  }

  move_cursor_yx(ctx->y, ctx->x);
}

void handle_insert_mode(struct Context *ctx, int ch) {
  bool handled = true;
  switch (ch) {
  case KEY_ENTER:
    line_break(ctx);
    ctx->y++;
    ctx->x = 0;
    break;

  case KEY_BACKSPACE:;
    int temp              = ctx->y > 0 ? ctx->buf[ctx->y - 1]->len : 0;
    enum RemoveResult res = remove_from_line(ctx, ctx->y, ctx->x);
    if (res == REMOVE_CHAR) {
      ctx->x--;
    } else if (res == REMOVE_LINE) {
      ctx->x = temp;
      ctx->y--;
    }
    break;

  case KEY_ESCAPE:
    if (ctx->x) ctx->x--;
    change_mode(ctx, MODE_NORMAL);
    break;

  default:
    if (ch >= 32 && ch <= 126) {
      write_to_line(ctx, ctx->y, ctx->x, ch);
      ctx->x++;
    } else {
      handled = false;
    }
    break;
  }

  if (handled) render(ctx);
  move_cursor_yx(ctx->y, ctx->x);
}

int main() {
  configure_context(&ctx);
  ANSI_RESET_SCREEN;
  render(&ctx);

  int ch;
  while ((ch = getchar()) != KEY_TAB) {
    if (ctx.mode == MODE_NORMAL) {
      handle_normal_mode(&ctx, ch);
    } else if (ctx.mode == MODE_INSERT) {
      handle_insert_mode(&ctx, ch);
    }
  }

  tcsetattr(STDIN_FILENO, TCSANOW, &ctx.backup);
  return 0;
}
