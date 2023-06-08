#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */

  char* expr;
  int new_val, old_val;
} WP;

void init_wp_pool();
WP* new_wp();
void free_wp(WP*);
WP* find_wp(int);
void list_watchpoint();
bool scan_watchpoint();

#endif
