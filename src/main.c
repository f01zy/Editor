#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "defines.h"
#include "filesystem.h"
#include "render.h"

struct Context ctx;

void handle_normal_mode(struct Context *ctx, struct Document *doc, int ch) {
  struct Line *line = doc->buf[doc->y];
  size_t len        = get_max_x(line);

  switch (ch) {
  case 'h':
    if (!doc->x && !doc->y) break;
    if (!doc->x) {
      doc->x = get_max_x(doc->buf[doc->y - 1]);
      doc->y--;
    } else {
      doc->x--;
    }
    break;

  case 'l':
    if (doc->x == len && doc->y == doc->len - 1) break;
    if (doc->x == len) {
      doc->x = 0;
      doc->y++;
    } else {
      doc->x++;
    }
    break;

  case 'j':
    if (doc->y == doc->len - 1) break;
    doc->y++;
    doc->x = MIN(doc->x, doc->buf[doc->y]->len);
    break;

  case 'k':
    if (!doc->y) break;
    doc->y--;
    doc->x = MIN(doc->x, doc->buf[doc->y]->len);
    break;

  case '$':
    doc->x = len;
    break;

  case '0':
    doc->x = 0;
    break;

  case 'i':
    change_mode(ctx, MODE_INSERT);
    break;

  case 'a':
    if (line->len > 0) doc->x++;
    change_mode(ctx, MODE_INSERT);
    break;

  case ':':
    change_mode(ctx, MODE_COMMAND);
    break;
  }
}

void handle_insert_mode(struct Context *ctx, struct Document *doc, int ch) {
  switch (ch) {
  case KEY_ENTER:
    line_break(doc);
    doc->y++;
    doc->x = 0;
    break;

  case KEY_BACKSPACE:;
    int temp              = doc->y > 0 ? doc->buf[doc->y - 1]->len : 0;
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

void handle_command(struct Context *ctx, struct Document *doc) {
  if (!ctx->cmd && !ctx->cmd->len) return;

  char *cmd = (char *)xmalloc(ctx->cmd->len + 1);
  memcpy(cmd, ctx->cmd->buf, ctx->cmd->len);
  cmd[ctx->cmd->len] = '\0';
  char *token        = strtok(cmd, " ");

  if (!strcmp(token, "quit")) {
    ctx->is_exit = true;
  }

  else if (!strcmp(token, "save")) {
    int size = save_doc(doc);
    if (size == -1) {
      set_status(ctx, "Failed to save file", STATUS_ERROR);
      free(cmd);
      return;
    }
    char buf[MAX_BUFFER_SIZE];
    snprintf(buf, sizeof(buf), "\"%s\", %dB written", doc->path, size);
    set_status(ctx, buf, STATUS_INFO);
  }

  else if (!strcmp(token, "open")) {
    token = strtok(NULL, " ");
    if (!token) {
      set_status(ctx, "Incorrent command format", STATUS_ERROR);
      free(cmd);
      return;
    }
    bool is_opened = load_doc_data(doc, token);
    if (!is_opened) {
      set_status(ctx, "Failed to open file", STATUS_ERROR);
      free(cmd);
      return;
    }
  }

  else {
    char buf[MAX_BUFFER_SIZE];
    int len = snprintf(buf, sizeof(buf), "Not an editor command: %s", cmd);
    set_status(ctx, buf, STATUS_ERROR);
  }
  free(cmd);
}

void handle_command_mode(struct Context *ctx, struct Document *doc, char ch) {
  switch (ch) {
  case KEY_ESCAPE:
    clear_cmd(ctx);
    change_mode(ctx, MODE_NORMAL);
    break;

  case KEY_ENTER:
    handle_command(ctx, doc);
    clear_cmd(ctx);
    change_mode(ctx, MODE_NORMAL);
    break;

  case KEY_BACKSPACE:
    if (ctx->cmd->len > 0) ctx->cmd->buf[--ctx->cmd->len] = '\0';
    break;

  default:
    if (ch >= 32 && ch <= 126 && ctx->cmd->len < ctx->cmd->size - 1) {
      ctx->cmd->buf[ctx->cmd->len++] = ch;
      ctx->cmd->buf[ctx->cmd->len]   = '\0';
    }
    break;
  }
}

int main(int argc, char **argv) {
  if (argc > 2) {
    printf("Invalid input data\n");
    return 1;
  }

  init_editor(&ctx);
  char *path           = argc == 2 ? argv[1] : NULL;
  struct Document *doc = create_doc(&ctx);
  load_doc_data(doc, path);
  if (!doc->len) add_line(doc, NULL, 0);
  render(&ctx);

  int ch;
  while (!ctx.is_exit) {
    struct Document *doc = ctx.docs[ctx.curr_doc];

    ch = getchar();
    if (ctx.mode == MODE_NORMAL) {
      handle_normal_mode(&ctx, doc, ch);
    } else if (ctx.mode == MODE_INSERT) {
      handle_insert_mode(&ctx, doc, ch);
    } else if (ctx.mode == MODE_COMMAND) {
      handle_command_mode(&ctx, doc, ch);
    }

    check_offset(&ctx, doc);
    render(&ctx);
    clear_status(&ctx);
  }

  quit_editor(&ctx);
  return 0;
}
