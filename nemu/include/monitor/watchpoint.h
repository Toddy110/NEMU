#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;

	/* TODO: Add more members if necessary */
	char expr[128];
	uint32_t value;

} WP;

/* Watchpoint interfaces */
void init_wp_pool(void);
void check_watchpoints(void);
void print_watchpoints(void);
WP *new_wp(void);
void free_wp(WP *wp);
WP *create_watchpoint(char *expr_str);
void delete_watchpoint(int no);

#endif
