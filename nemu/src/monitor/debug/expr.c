#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
	NOTYPE = 256, NUMBER, EQ

	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
	int priority;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +",	NOTYPE, 0},					// spaces
	{"\\+", '+', 2},					// plus
	{"==", EQ, 1},						// equal
	{"[0-9]+", NUMBER, 0},	    // decimal number
	{"\\(", '(', 4},					// left parenthesis
	{"\\)", ')', 4},					// right parenthesis
	{"-", '-', 2},						// minus
	{"\\*", '*', 3},					//multiply
	{"/", '/', 3}						//divide
	
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
	int priority;
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */

				switch(rules[i].token_type) {
					case NOTYPE: 
						break;
					case '+':
					case '-':
					case '*':
					case '/':
					case '(':
					case ')':
					case EQ:
					case NUMBER:
						tokens[nr_token].type = rules[i].token_type;
						tokens[nr_token].priority = rules[i].priority;
						strncpy(tokens[nr_token].str, substr_start, substr_len);
						tokens[nr_token].str[substr_len] = '\0';
						nr_token++;
						break;
					default: panic("please implement me");
				}
				position += substr_len;
				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true; 
}

int dominant_operator(int p, int q){
	int min_priority = 20;
	int op_pos = -1;
	int parentheses = 0;
	int i;
	for (i = p; i <= q; i++){
		if (tokens[i].type == '('){
			parentheses++;
		}
		else if (tokens[i].type == ')'){
			parentheses--;
		}
		else if (parentheses == 0){
			if (tokens[i].priority <= min_priority){
				min_priority = tokens[i].priority;
				op_pos = i;
			}
		}
	}
	assert(op_pos != -1);
	return op_pos;
}

bool check_parentheses(int p, int q){
	if (tokens[p].type != '(' || tokens[q].type != ')')
		return false;
	int count = 0;
	int pos;
	for (pos = p + 1; pos <= q - 1; pos++){
		if (tokens[pos].type == '('){
			count++;
		}
		else if (tokens[pos].type == ')'){
			count--;
		}
		if (count < 0){
			return false;
		}
	}
	if (count){
		return false;
	}
	return true;
}

uint32_t eval(int p, int q){
	if (p > q) {
		assert(0);
	}
	else if (p == q){
		uint32_t value = 0;
		if (tokens[p].type == NUMBER){
			sscanf(tokens[p].str, "%d", &value);
			return value;
		}
	}
	else if (check_parentheses(p, q) == true){
		return eval(p + 1, q - 1);
	}

	else{
		int op = dominant_operator(p, q);
		uint32_t val1 = eval(p,op - 1);
		uint32_t val2 = eval(op + 1, q);

		switch (tokens[op].type){
			case '+': return val1 + val2;
			case '-': return val1 - val2;
			case '*': return val1 * val2;
			case '/': return val1 / val2;
			default: assert(0);
		}
	}
	assert (1);
	return 0;
}

uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}
	
	/* TODO: Insert codes to evaluate the expression. */
	return eval(0, nr_token - 1);
}

