#include "mappings.h"
#include <string.h>

struct Mapping maps_list[] = {
    {"h", map_left},         {"l", map_right},      {"j", map_down},     {"k", map_up},         {"i", map_insert_mode_prev}, {"a", map_insert_mode_next},
    {":", map_command_mode}, {"0", map_line_start}, {"$", map_line_end}, {"gg", map_doc_start}, {"G", map_doc_end},
};

void map_up(struct Context *ctx) {
  struct Document *doc = ctx->docs[ctx->curr_doc];
  if (!doc->y) return;
  doc->x = MIN(doc->x, get_max_x(doc->buf[doc->y - 1]));
  doc->y--;
}

void map_down(struct Context *ctx) {
  struct Document *doc = ctx->docs[ctx->curr_doc];
  if (doc->y == doc->len - 1) return;
  doc->x = MIN(doc->x, get_max_x(doc->buf[doc->y + 1]));
  doc->y++;
}

void map_left(struct Context *ctx) {
  struct Document *doc = ctx->docs[ctx->curr_doc];
  if (!doc->x && !doc->y) return;
  if (!doc->x) {
    doc->x = get_max_x(doc->buf[doc->y - 1]);
    doc->y--;
  } else {
    doc->x--;
  }
}

void map_right(struct Context *ctx) {
  struct Document *doc = ctx->docs[ctx->curr_doc];
  size_t len = get_max_x(doc->buf[doc->y]);
  if (doc->x == len && doc->y == doc->len - 1) return;
  if (doc->x == len) {
    doc->x = 0;
    doc->y++;
  } else {
    doc->x++;
  }
}

void map_command_mode(struct Context *ctx) { change_mode(ctx, MODE_COMMAND); }
void map_insert_mode_prev(struct Context *ctx) { change_mode(ctx, MODE_INSERT); }

void map_insert_mode_next(struct Context *ctx) {
  struct Document *doc = ctx->docs[ctx->curr_doc];
  if (doc->buf[doc->y]->len > 0) doc->x++;
  change_mode(ctx, MODE_INSERT);
}

void map_line_start(struct Context *ctx) {
  struct Document *doc = ctx->docs[ctx->curr_doc];
  doc->x = 0;
}

void map_line_end(struct Context *ctx) {
  struct Document *doc = ctx->docs[ctx->curr_doc];
  doc->x = get_max_x(doc->buf[doc->y]);
}

void map_doc_start(struct Context *ctx) {
  struct Document *doc = ctx->docs[ctx->curr_doc];
  doc->x = MIN(doc->x, get_max_x(doc->buf[0]));
  doc->y = 0;
}

void map_doc_end(struct Context *ctx) {
  struct Document *doc = ctx->docs[ctx->curr_doc];
  doc->x = MIN(doc->x, get_max_x(doc->buf[doc->len - 1]));
  doc->y = doc->len - 1;
}

void add_map_node(struct Context *ctx, struct MappingNode *head, struct Mapping map) {
  struct MappingNode *curr = head;
  size_t len = strlen(map.cmd);
  for (int i = 0; i < len; i++) {
    bool is_found = false;
    for (int j = 0; j < curr->len; j++) {
      if (curr->nodes[j]->ch == map.cmd[i]) {
        is_found = true;
        curr = curr->nodes[j];
        break;
      }
    }
    if (!is_found) {
      struct MappingNode *node = (struct MappingNode *)xcalloc(1, sizeof(struct MappingNode));
      node->ch = map.cmd[i];
      curr->len++;
      curr->nodes = (struct MappingNode **)xrealloc(curr->nodes, curr->len * sizeof(struct MappingNode *));
      curr->nodes[curr->len - 1] = node;
      curr = node;
    }
    if (i == len - 1) curr->act = map.act;
  }
}

void init_maps(struct Context *ctx) {
  struct MappingNode *head = (struct MappingNode *)xcalloc(1, sizeof(struct MappingNode));
  for (int i = 0; i < sizeof(maps_list) / sizeof(maps_list[0]); i++) {
    struct Mapping map = maps_list[i];
    add_map_node(ctx, head, map);
  }
  ctx->map_head = ctx->map_curr = head;
}
