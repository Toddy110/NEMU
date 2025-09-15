#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;
	char expr[32];
	uint32_t value;
	/* TODO: Add more members if necessary */


} WP;

WP* new_wp();
void free_wp(WP *wp);
void check_watchpoints();
void print_watchpoints();
WP* create_watchpoint(char *expr_str);
void delete_watchpoint(int no);
#endif
