#include <stdio.h>
#include <unistd.h>

#include "terminal.h"

void move_cursor_yx(int y, int x) {
  char buf[MAX_BUFFER_SIZE];
  int len = snprintf(buf, sizeof(buf), ANSI_MOVE_CURSOR_YX, y + 1, x + 1);
  write(STDOUT_FILENO, buf, len);
}

void set_cursor_style(enum CursorStyle type) {
  char buf[MAX_BUFFER_SIZE];
  int len = snprintf(buf, sizeof(buf), ANSI_CURSOR_TYPE, type);
  write(STDOUT_FILENO, buf, len);
}

void set_render_mode(enum RenderMode mode) {
  char buf[MAX_BUFFER_SIZE];
  int len = snprintf(buf, sizeof(buf), ANSI_RENDER_MODE, mode);
  write(STDOUT_FILENO, buf, len);
}
