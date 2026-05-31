#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "defines.h"
#include "render.h"
#include "types.h"

void render_line(struct Context *ctx, struct Cell *buf, size_t len, int y) {
  move_cursor_yx(y, 0);
  ANSI_RESET_LINE_FROM_CURSOR;
  enum RenderMode mode = buf[0].mode;
  char temp[MAX_BUFFER_SIZE];
  int curr = 0;
  for (int i = 0; i < len; i++) {
    if (buf[i].mode != mode || i == len - 1) {
      set_render_mode(mode);
      write(STDOUT_FILENO, temp, curr);
      mode = buf[i].mode;
      curr = 0;
    }
    temp[curr++] = buf[i].ch;
  }
  if (curr > 0) {
    set_render_mode(mode);
    write(STDOUT_FILENO, temp, curr);
  }
}

void render_tabmenu(struct Context *ctx, struct Cell **frame) {
  int max_len = 0;
  for (int i = 0; i < ctx->len; i++) {
    const char *filename = get_file_name(ctx->docs[i]->path);
    max_len              = MAX(max_len, strlen(filename));
  }
  int max_docs_count = MAX(ctx->win.ws_col / max_len, 1);
  int offset         = MAX((int)ctx->curr_doc - max_docs_count + 1, 0);
  for (int i = offset; i < offset + max_docs_count && i < ctx->len; i++) {
    const char *filename = get_file_name(ctx->docs[i]->path);
    int len              = strlen(filename);
    int tab_start_x      = (i - offset) * max_len;
    for (int j = 0; j < len; j++) {
      enum RenderMode mode      = i == ctx->curr_doc ? RENDER_DEFAULT : RENDER_DIM;
      frame[0][tab_start_x + j] = (struct Cell){filename[j], mode};
    }
  }
}

void render_line_numbers(struct Context *ctx, struct Document *doc, struct Cell **frame) {
  int offsetY = get_tabmenu_margin(ctx);
  int width   = get_line_number_margin(ctx);
  int height  = get_buffer_height(ctx);

  char buf[MAX_BUFFER_SIZE];
  for (int i = 0; i < height; i++) {
    int y = doc->offsetY + i;
    if (y >= doc->len) {
      for (int j = 0; j < width; j++) {
        frame[i + offsetY][j] = (struct Cell){' ', RENDER_DEFAULT};
      }
      continue;
    }
    enum RenderMode mode = doc->y == y ? RENDER_DEFAULT : RENDER_DIM;
    int len              = snprintf(buf, sizeof(buf), "%d", y + 1);
    for (int j = 0; j < len; j++) {
      frame[i + offsetY][j] = (struct Cell){buf[j], mode};
    }
    for (int j = len; j < width; j++) {
      frame[i + offsetY][j] = (struct Cell){' ', mode};
    }
  }
}

void render_statusline(struct Context *ctx, struct Document *doc, struct Cell **frame) {
  char buf[MAX_BUFFER_SIZE];
  char *mode_label;
  int len;
  if (ctx->status) {
    len = snprintf(buf, sizeof(buf), "%s", ctx->status->msg);
  } else if (ctx->mode == MODE_COMMAND) {
    len = snprintf(buf, sizeof(buf), ":%s", ctx->cmd->buf);
  } else {
    if (ctx->mode == MODE_NORMAL) mode_label = "NORMAL";
    if (ctx->mode == MODE_INSERT) mode_label = "INSERT";
    len = snprintf(buf, sizeof(buf), "-- %s -- %d/%d", mode_label, doc->y + 1, doc->x + 1);
  }
  int y = ctx->win.ws_row - 1;
  for (int i = 0; i < len; i++) {
    frame[y][i] = (struct Cell){buf[i], RENDER_DEFAULT};
  }
  for (int i = len; i < ctx->win.ws_col; i++) {
    frame[y][i] = (struct Cell){' ', RENDER_DEFAULT};
  }
}

void render_buf(struct Context *ctx, struct Document *doc, struct Cell **frame) {
  int offsetX = get_line_number_margin(ctx);
  int offsetY = get_tabmenu_margin(ctx);
  int width   = get_buffer_width(ctx);
  int height  = get_buffer_height(ctx);

  for (int i = 0; i < height; i++) {
    int y = doc->offsetY + i;
    if (y >= doc->len) {
      for (int j = 0; j < width; j++) {
        frame[i + offsetY][j + offsetX] = (struct Cell){' ', RENDER_DEFAULT};
      }
      continue;
    }
    struct Line *line = doc->buf[y];
    for (int j = 0; j < width; j++) {
      int x                           = doc->offsetX + j;
      frame[i + offsetY][j + offsetX] = (struct Cell){x < line->len ? line->buf[x] : ' ', RENDER_DEFAULT};
    }
  }
}

void render(struct Context *ctx) {
  struct Document *doc = ctx->docs[ctx->curr_doc];
  if (ctx->ui.is_tabmenu) render_tabmenu(ctx, ctx->curr_frame);
  if (ctx->ui.is_line_numbers) render_line_numbers(ctx, doc, ctx->curr_frame);
  if (ctx->ui.is_statusline) render_statusline(ctx, doc, ctx->curr_frame);
  render_buf(ctx, doc, ctx->curr_frame);
  for (int i = 0; i < ctx->win.ws_row; i++) {
    for (int j = 0; j < ctx->win.ws_col; j++) {
      if (!ctx->prev_frame || ctx->curr_frame[i][j].ch != ctx->prev_frame[i][j].ch || ctx->curr_frame[i][j].mode != ctx->prev_frame[i][j].mode) {
        render_line(ctx, ctx->curr_frame[i], ctx->win.ws_col, i);
        break;
      }
    }
  }
  for (int i = 0; i < ctx->win.ws_row; i++) {
    memcpy(ctx->prev_frame[i], ctx->curr_frame[i], ctx->win.ws_col * sizeof(struct Cell));
  }
  if (ctx->mode == MODE_COMMAND) {
    move_cursor_yx(ctx->win.ws_row - 1, ctx->cmd->len + 1);
  } else {
    int offsetX = get_line_number_margin(ctx);
    int offsetY = get_tabmenu_margin(ctx);
    move_cursor_yx(doc->y - doc->offsetY + offsetY, doc->x - doc->offsetX + offsetX);
  }
}
