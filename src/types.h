#ifndef TYPES_INCLUDED
#define TYPES_INCLUDED

#include <stdbool.h>
#include <stddef.h>
#include <sys/ioctl.h>
#include <termios.h>

enum RenderMode {
  RENDER_DEFAULT,
  RENDER_BOLD,
  RENDER_DIM,
  RENDER_ITALIC,
  RENDER_UNDERLINE,
  RENDER_BLINKING,
  RENDER_INVERSE,
  RENDER_HIDDEN,
  RENDER_STRIKETHROUGH,
};

enum CursorStyle {
  CURSOR_BLOCK_BLINKING = 1,
  CURSOR_BLOCK_STATIC,
  CURSOR_UNDERLINE_BLINKING,
  CURSOR_UNDERLINE_STATIC,
  CURSOR_LINE_BLINKING,
  CURSOR_LINE_STATIC,
};

enum Mode { MODE_NORMAL, MODE_INSERT, MODE_COMMAND };
enum RemoveResult { REMOVE_NOTHING, REMOVE_CHAR, REMOVE_LINE };
enum StatusType { STATUS_INFO, STATUS_WARNING, STATUS_ERROR };

struct UI {
  bool is_line_numbers;
};

struct Line {
  char *buf;
  size_t len, size;
};

struct Cell {
  char ch;
  enum RenderMode mode;
};

struct Status {
  char *msg;
  enum StatusType type;
};

struct Document {
  int x, y;
  int offsetX, offsetY;
  size_t len, size;
  char *path;
  struct Line **buf;
};

struct Context {
  bool is_exit;
  size_t len, size, curr_doc;
  struct Document **docs;
  struct Cell **prev_frame;
  struct Line *cmd;
  struct Status *status;
  struct termios conf;
  struct termios backup;
  struct winsize win;
  struct UI ui;
  enum Mode mode;
};

#endif
