#include "nemu.h"
#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "monitor/monitor.h"

#define NR_WP 33

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp(){
	WP *f_wp, *current_wp;
	f_wp = free_;
	free_ = free_->next;
	f_wp->next = NULL;
	current_wp = head;
	if (current_wp == NULL){
		head = f_wp;
		head->next = NULL;
	} 
	else{
		while (current_wp->next != NULL){
			current_wp = current_wp->next;
		}
		current_wp->next = f_wp;
	}
	return f_wp;
}

void free_wp(WP *wp){
	WP *f_wp, *current_wp;
	current_wp = free_;
	if (current_wp == NULL){
		free_ = wp;
		free_->next = NULL;
	}
	else{
		while (current_wp->next != NULL){
			current_wp = current_wp->next;
		}
		current_wp->next = wp;
	}
	f_wp = head;
	if (head == NULL){
		assert(0);
	}
	if (head->NO == wp->NO){
		head = head->next;
	}
	else{
		while (f_wp->next != NULL && f_wp->next->NO != wp->NO){
			f_wp = f_wp->next;
		}
		if (f_wp->next != NULL && f_wp->next->NO == wp->NO){
			f_wp->next = f_wp->next->next;
		}
		else{
			assert(0);
		}
	}
	wp->next = NULL;
}

void check_watchpoints() {
	WP *wp = head;
	while (wp != NULL) {
		bool success = true;
		uint32_t new_val = expr(wp->expr, &success);
		if (success && new_val != wp->value) {
			printf("\nHint watchpoint %d at address 0x%08x\n", wp->NO, cpu.eip);
			printf("Old value = 0x%08x (%d)\n", wp->value, wp->value);
			printf("New value = 0x%08x (%d)\n", new_val, new_val);
			nemu_state = STOP;
			return;
		}
		wp->value = new_val;
		wp = wp->next;
	}
}

void print_watchpoints() {
	WP *wp = head;
	if (wp == NULL) {
		printf("No watchpoints.\n");
		return;
	}
	printf("Num     Type           Disp Enb Address            What\n");
	while (wp != NULL) {
		printf("%-8d%-15s%-4s%-4s%-20s%s\n", 
			wp->NO, "watchpoint", "keep", "y", "", wp->expr);
		wp = wp->next;
	}
}

WP* create_watchpoint(char *expr_str){
	if (expr_str == NULL || strlen(expr_str) == 0){
		return NULL;
	}
	bool success = true;
	uint32_t value = expr(expr_str, &success);
	if (!success){
		return NULL;
	}
	WP *wp = new_wp();
	if (wp == NULL){
		return NULL;
	}

	strncpy(wp->expr, expr_str, sizeof(wp->expr) - 1);
	wp->expr[sizeof(wp->expr) - 1] = '\0';
	wp->value = value;
	printf("Watchpoint %d: %s\n", wp->NO, wp->expr);
	return wp;
}

void delete_watchpoint(int no){
	WP *wp = head;
	if (wp == NULL){
		return;
	}
	if (wp->NO == no){
		head = head->next;
		free_wp(wp);
		printf ("Watchpoint %d deleted.\n", no);
		return;
	}
	while (wp->next != NULL && wp->next->NO != no){
		wp = wp->next;
	}
	if (wp->next != NULL && wp->next->NO == no){
		WP *to_delete = wp->next;
		wp->next = wp->next->next;
		free_wp(to_delete);
		printf("Watchpoint %d deleted.\n", no);
	}
	else{
		printf("No watchpoint number %d.\n", no);
	}
}