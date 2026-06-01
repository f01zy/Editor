#ifndef MAPPINGS_INCLUDED
#define MAPPINGS_INCLUDED

#include "service.h"

void map_up(struct Context *ctx);
void map_down(struct Context *ctx);
void map_left(struct Context *ctx);
void map_right(struct Context *ctx);
void map_command_mode(struct Context *ctx);
void map_insert_mode_prev(struct Context *ctx);
void map_insert_mode_next(struct Context *ctx);
void map_line_start(struct Context *ctx);
void map_line_end(struct Context *ctx);
void map_doc_start(struct Context *ctx);
void map_doc_end(struct Context *ctx);

void add_map_node(struct Context *ctx, struct MappingNode *head, struct Mapping map);
void init_maps(struct Context *ctx);

#endif
