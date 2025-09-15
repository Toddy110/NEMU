#include "monitor/watchpoint.h"
#include "monitor/expr.h"

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
	WP *current_wp = head;
	WP *prev_wp = NULL;
	while (current_wp != NULL && current_wp != wp){
		prev_wp = current_wp;
		current_wp = current_wp->next;
	}
	if (current_wp == NULL){
		assert(0);
	}
	if (prev_wp == NULL){
		head = head->next;
	}
	else{
		prev_wp->next = current_wp->next;
	}
	wp->next = free_;
	free_ = wp;
}


