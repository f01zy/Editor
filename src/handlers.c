#include "handlers.h"

void handle_normal_mode(struct Context *ctx, int ch) {
  for (int i = 0; i < ctx->map_curr->len; i++) {
    struct MappingNode *node = ctx->map_curr->nodes[i];
    if (node->ch == ch) {
      ctx->map_curr = node;
      break;
    }
  }
  if (!ctx->map_curr->len) exec_curr_map(ctx);
}

void handle_insert_mode(struct Context *ctx, int ch) {
  struct Document *doc = ctx->docs[ctx->curr_doc];
  switch (ch) {
  case KEY_ENTER:
    line_break(doc);
    doc->y++;
    doc->x = 0;
    break;

  case KEY_BACKSPACE:;
    int temp = doc->y > 0 ? doc->buf[doc->y - 1]->len : 0;
    enum RemoveResult res = remove_from_line(doc, doc->y, doc->x);
    if (res == REMOVE_CHAR) {
      doc->x--;
    } else if (res == REMOVE_LINE) {
      doc->x = temp;
      doc->y--;
    }
    break;

  case KEY_ESCAPE:
    if (doc->x) doc->x--;
    change_mode(ctx, MODE_NORMAL);
    break;

  default:
    if (ch >= 32 && ch <= 126) {
      write_to_line(doc, doc->y, doc->x, ch);
      doc->x++;
    }
    break;
  }
}

void handle_command_mode(struct Context *ctx, char ch) {
  switch (ch) {
  case KEY_ESCAPE:
    clear_cmd(ctx);
    change_mode(ctx, MODE_NORMAL);
    break;

  case KEY_ENTER:
    handle_command(ctx);
    clear_cmd(ctx);
    change_mode(ctx, MODE_NORMAL);
    break;

  case KEY_BACKSPACE:
    if (ctx->cmd->len > 0) ctx->cmd->buf[--ctx->cmd->len] = '\0';
    break;

  default:
    if (ch >= 32 && ch <= 126 && ctx->cmd->len < ctx->cmd->size - 1) {
      ctx->cmd->buf[ctx->cmd->len++] = ch;
      ctx->cmd->buf[ctx->cmd->len] = '\0';
    }
    break;
  }
}
