#ifndef HANDLERS_INCLUDED
#define HANDLERS_INCLUDED

#include "filesystem.h"
#include "render.h"

void handle_command_mode(struct Context *ctx, struct Document *doc, char ch);
void handle_command(struct Context *ctx, struct Document *doc);
void handle_insert_mode(struct Context *ctx, struct Document *doc, int ch);
void handle_normal_mode(struct Context *ctx, struct Document *doc, int ch);

#endif
