#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE        1024

#define KEY_ENTER              13
#define KEY_BACKSPACE          127
#define KEY_ESCAPE             27
#define KEY_TAB                9

#define ANSI_MOVE_CURSOR_UP    write(STDOUT_FILENO, "\x1b[1A", 4);
#define ANSI_MOVE_CURSOR_DOWN  write(STDOUT_FILENO, "\x1b[1B", 4);
#define ANSI_MOVE_CURSOR_RIGHT write(STDOUT_FILENO, "\x1b[1C", 4);
#define ANSI_MOVE_CURSOR_LEFT  write(STDOUT_FILENO, "\x1b[1D", 4);
#define ANSI_RESET_SCREEN      write(STDOUT_FILENO, "\x1b[2J", 4);
#define ANSI_RESET_LINE        write(STDOUT_FILENO, "\x1b[2K", 4);
#define ANSI_MOVE_CURSOR_YX    "\x1b[%d;%dH"

enum Mode { MODE_NORMAL, MODE_INSERT };

struct Line {
  char *buf;
  size_t len;
  size_t size;
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
  line->buf              = malloc(1);
  line->buf[0]           = '\0';
  line->len              = 0;
  line->size             = 1;
  ctx->buf[ctx->len - 1] = line;
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

void move_cursor_yx(int y, int x) {
  char buf[MAX_BUFFER_SIZE];
  int len = snprintf(buf, sizeof(buf), ANSI_MOVE_CURSOR_YX, y, x);
  write(STDOUT_FILENO, buf, len);
}

void write_line(struct Context *ctx, char ch) {
  struct Line *line = ctx->buf[ctx->y];
  if (line->len + 2 > line->size) {
    line->size = line->len + 16;
    line->buf  = (char *)realloc(line->buf, line->size);
  }
  for (int i = line->len; i > ctx->x; i--) {
    line->buf[i] = line->buf[i - 1];
  }
  line->buf[ctx->x++]    = ch;
  line->buf[++line->len] = '\0';
}

void pop_line(struct Context *ctx) {
  struct Line *line = ctx->buf[ctx->y];
  if (line->len == 0 || ctx->x == 0) return;
  for (int i = ctx->x; i < line->len; i++) {
    line->buf[i] = line->buf[i + 1];
  }
  line->buf[line->len--] = '\0';
  ctx->x--;
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
    if (ctx->y == ctx->len - 1) return;
    ctx->y++;
  }

  else if (ch == 107) {
    if (ctx->y == 0) return;
    ctx->y--;
  }

  else if (ch == 108) {
    struct Line *line = ctx->buf[ctx->y];
    if (ctx->x == line->len && ctx->y == ctx->len - 1) return;
    if (ctx->x == line->len) {
      ctx->x = 0;
      ctx->y++;
    } else {
      ctx->x++;
    }
  }
  move_cursor_yx(ctx->y, ctx->x);
}

void handle_insert_mode(struct Context *ctx, int ch) {
  if (ch == KEY_ESCAPE) {
    ctx->mode = MODE_NORMAL;
    return;
  }

  if (ch == KEY_BACKSPACE) {
    pop_line(ctx);
  } else if (ch == KEY_ENTER) {
    // Line break
  } else if (ch >= 32 && ch <= 126) {
    write_line(ctx, ch);
  }

  move_cursor_yx(ctx->y, 0);
  ANSI_RESET_LINE;
  struct Line *line = ctx->buf[ctx->y];
  write(STDOUT_FILENO, line->buf, line->len);
  move_cursor_yx(ctx->y, ctx->x);
}

int main() {
  configure_context(&ctx);
  move_cursor_yx(0, 0);
  ANSI_RESET_SCREEN;

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
