#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#define KEY_ENTER       13
#define KEY_BACKSPACE   127
#define KEY_ESCAPE      27
#define MAX_BUFFER_SIZE 1024

struct Line {
  char *buf;
  size_t len;
};

struct Context {
  int x, y;
  int offsetX, offsetY;
  struct Line **buf;
  size_t len;
  struct termios *conf;
};

struct termios prev;
struct winsize size;
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
  line->buf                = (char *)realloc(line->buf, line->len * sizeof(char));
  line->buf[line->len - 1] = ch;
}

void pop_line(struct Context *ctx) {
  struct Line *line = ctx->buf[ctx->y];
  if (line->len == 0) return;
  line->buf[--line->len] = '\0';
}

void move_cursor_yx(int y, int x) {
  char buf[MAX_BUFFER_SIZE];
  int len = snprintf(buf, sizeof(buf), "\x1b[%d;%dH", y, x);
  write(STDOUT_FILENO, buf, len);
}

void reset_line() {
  char buf[MAX_BUFFER_SIZE];
  int len = snprintf(buf, sizeof(buf), "\x1b[2K");
  write(STDOUT_FILENO, buf, len);
}

void reset_screen() {
  char buf[MAX_BUFFER_SIZE];
  int len = snprintf(buf, sizeof(buf), "\x1b[2J");
  write(STDOUT_FILENO, buf, len);
}

void configure_context(struct Context *ctx, struct termios *conf) {
  ctx->x       = 0;
  ctx->y       = 0;
  ctx->offsetX = 0;
  ctx->offsetY = 0;
  ctx->buf     = NULL;
  ctx->len     = 0;
  add_line(ctx);
  ctx->conf = (struct termios *)malloc(sizeof(struct termios));
  memcpy(ctx->conf, conf, sizeof(struct termios));
  ctx->conf->c_iflag |= IXOFF;
  ctx->conf->c_iflag &= ~ICRNL;
  ctx->conf->c_lflag &= ~ECHO;
  ctx->conf->c_lflag &= ~ISIG;
  ctx->conf->c_lflag &= ~ICANON;
}

int main() {
  tcgetattr(STDIN_FILENO, &prev);
  configure_context(&ctx, &prev);
  tcsetattr(STDIN_FILENO, TCSANOW, ctx.conf);
  ioctl(STDIN_FILENO, TIOCGWINSZ, &size);
  reset_screen();
  move_cursor_yx(0, 0);

  int ch;
  while ((ch = getchar()) != KEY_ESCAPE) {
    switch (ch) {
    case KEY_BACKSPACE:
      pop_line(&ctx);
      break;
    case KEY_ENTER:
      break;
    default:
      write_line(&ctx, ch);
      break;
    }
    move_cursor_yx(ctx.y, 0);
    reset_line();
    struct Line *line = ctx.buf[ctx.y];
    write(STDOUT_FILENO, line->buf, line->len);
  }

  reset_screen();
  tcsetattr(STDIN_FILENO, TCSANOW, &prev);
  return 0;
}
