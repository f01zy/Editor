#include <stdio.h>
#include <string.h>

#include "filesystem.h"

void load_curr_file(struct Context *ctx) {
  FILE *file = fopen(ctx->curr_path, "r");
  if (!file) {
    set_status(ctx, "Failed to open file");
    return;
  }
  char buf[MAX_BUFFER_SIZE];
  while (fgets(buf, sizeof(buf), file)) {
    buf[strcspn(buf, "\n")] = '\0';
    add_line(ctx, buf, ctx->len);
  }
  fclose(file);
}

void save_curr_file(struct Context *ctx) {
  char buf[MAX_BUFFER_SIZE];
  FILE *file = fopen(ctx->curr_path, "w");
  if (!file) {
    set_status(ctx, "Failed to open file");
    return;
  }
  int size = 0;
  for (int i = 0; i < ctx->len; i++) {
    struct Line *line = ctx->buf[i];
    int len           = snprintf(buf, sizeof(buf), "%s\n", line->buf);
    fwrite(buf, len, 1, file);
    size += len;
  }
  snprintf(buf, sizeof(buf), "\"%s\", %dB written", ctx->curr_path, size);
  set_status(ctx, buf);
  fclose(file);
}
