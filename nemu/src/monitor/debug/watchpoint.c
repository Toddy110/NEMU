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
		if (f_wp->next == NULL && f_wp->NO != wp->NO){
			assert(0);
		}
		else if (f_wp->next->NO == wp->NO){
			f_wp->next = f_wp->next->next;
		}
		else{
			assert(0);
		}
	}
	wp->next = NULL;
}


