#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE     1024

#define KEY_ENTER           13
#define KEY_BACKSPACE       127
#define KEY_ESCAPE          27

#define ANSI_MOVE_CURSOR_YX "\x1b[%d;%dH"
#define ANSI_RESET_SCREEN   "\x1b[2J"
#define ANSI_RESET_LINE     "\x1b[2K"

enum Mode { MODE_NORMAL, MODE_INSERT };

struct Line {
  char *buf;
  size_t len;
};

struct Context {
  int x, y;
  int offsetX, offsetY;
  struct Line **buf;
  size_t len;
  struct termios conf;
  struct termios backup;
  struct winsize size;
  enum Mode mode;
};

struct Context ctx;

void add_line(struct Context *ctx) {
  ctx->len++;
  ctx->buf               = (struct Line **)realloc(ctx->buf, ctx->len * sizeof(struct Line *));
  struct Line *line      = (struct Line *)malloc(sizeof(struct Line));
  line->buf              = NULL;
  line->len              = 0;
  ctx->buf[ctx->len - 1] = line;
}

void write_line(struct Context *ctx, char ch) {
  struct Line *line = ctx->buf[ctx->y];
  line->len++;
  line->buf = (char *)realloc(line->buf, line->len * sizeof(char));
  for (int i = ctx->len; i > ctx->x; i--) {
    line->buf[i] = line->buf[i - 1];
  }
  line->buf[ctx->x++] = ch;
  line->buf[ctx->x]   = '\0';
}

void pop_line(struct Context *ctx) {
  struct Line *line = ctx->buf[ctx->y];
  if (line->len == 0) return;
  for (int i = ctx->x; i < line->len - 1; i++) {
    line->buf[i] = line->buf[i + 1];
  }
  line->buf[--line->len] = '\0';
}

void reset_line() { write(STDOUT_FILENO, ANSI_RESET_LINE, 4); }
void reset_screen() { write(STDOUT_FILENO, ANSI_RESET_SCREEN, 4); }

void move_cursor_yx(int y, int x) {
  char buf[MAX_BUFFER_SIZE];
  int len = snprintf(buf, sizeof(buf), ANSI_MOVE_CURSOR_YX, y, x);
  write(STDOUT_FILENO, buf, len);
}

void configure_context(struct Context *ctx) {
  ioctl(STDIN_FILENO, TIOCGWINSZ, ctx->size);
  tcgetattr(STDIN_FILENO, &ctx->backup);
  ctx->x             = 0;
  ctx->y             = 0;
  ctx->offsetX       = 0;
  ctx->offsetY       = 0;
  ctx->buf           = NULL;
  ctx->len           = 0;
  ctx->mode          = MODE_NORMAL;
  ctx->conf          = ctx->backup;
  ctx->conf.c_iflag |= IXOFF;
  ctx->conf.c_iflag &= ~ICRNL;
  ctx->conf.c_lflag &= ~ECHO;
  ctx->conf.c_lflag &= ~ISIG;
  ctx->conf.c_lflag &= ~ICANON;
  add_line(ctx);
  tcsetattr(STDIN_FILENO, TCSANOW, &ctx->conf);
}

void handle_normal_mode(struct Context *ctx, int ch) {
  if (ch == 104) {
    if (ctx->x == 0 && ctx->y == 0) return;
    if (ctx->x == 0) {
      ctx->y--;
      struct Line *line = ctx->buf[ctx->y];
      ctx->x            = line->len;
    } else {
      ctx->x--;
    }
  }

  else if (ch == 105) {
    ctx->mode = MODE_INSERT;
  }

  else if (ch == 106) {
    if (ctx->y == ctx->len) return;
    ctx->y++;
  }

  else if (ch == 107) {
    if (ctx->y == 0) return;
    ctx->y--;
  }

  else if (ch == 108) {
    struct Line *line = ctx->buf[ctx->y];
    if (ctx->x == line->len && ctx->y == ctx->len) return;
    if (ctx->x == line->len) {
      ctx->x = 0;
      ctx->y++;
    } else {
      ctx->x++;
    }
  }
}

void handle_insert_mode(struct Context *ctx, int ch) {
  if (ch == KEY_ESCAPE) {
    ctx->mode = MODE_NORMAL;
    return;
  }

  if (ch == KEY_BACKSPACE) pop_line(ctx);
  if (ch >= 32 && ch <= 126) write_line(ctx, ch);
  move_cursor_yx(ctx->y, 0);
  reset_line();
  struct Line *line = ctx->buf[ctx->y];
  write(STDOUT_FILENO, line->buf, line->len);
}

int main() {
  configure_context(&ctx);
  reset_screen();
  move_cursor_yx(0, 0);

  int ch;
  while ((ch = getchar()) != VEOF) {
    if (ctx.mode == MODE_NORMAL) {
      handle_normal_mode(&ctx, ch);
    } else if (ctx.mode == MODE_INSERT) {
      handle_insert_mode(&ctx, ch);
    }
  }

  tcsetattr(STDIN_FILENO, TCSANOW, &ctx.backup);
  return 0;
}
