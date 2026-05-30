#ifndef SERVICE_INCLUDED
#define SERVICE_INCLUDED

#include "buffer.h"
#include "terminal.h"

void set_status(struct Context *ctx, char *status);
void clear_status(struct Context *ctx);

void configure_context(struct Context *ctx, char *file_path);
void free_resources(struct Context *ctx);
void clear_cmd(struct Context *ctx);
void check_offset(struct Context *ctx);
void change_mode(struct Context *ctx, enum Mode mode);
int get_line_number_margin(struct Context *ctx);

#endif
