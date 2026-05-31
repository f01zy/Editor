#include "ui.h"

int get_line_number_margin(struct Context *ctx) {
  if (!ctx->ui.is_line_numbers) return 0;
  struct Document *doc = ctx->docs[ctx->curr_doc];
  int len = doc->len, margin = 0;
  while (len) {
    margin++;
    len /= 10;
  }
  return margin + 1;
}

int get_statusline_margin(struct Context *ctx) {
  if (!ctx->ui.is_statusline) return 0;
  return 1;
}

int get_tabmenu_margin(struct Context *ctx) {
  if (!ctx->ui.is_tabmenu) return 0;
  return 1;
}

int get_buffer_width(struct Context *ctx) { return ctx->win.ws_col - get_line_number_margin(ctx); }
int get_buffer_height(struct Context *ctx) { return ctx->win.ws_row - get_tabmenu_margin(ctx) - get_statusline_margin(ctx); }
