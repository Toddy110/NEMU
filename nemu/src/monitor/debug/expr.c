#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <string.h>

extern uint32_t get_sym_val(char *syc, bool *success);

enum {
	NOTYPE = 256, DEC_NUMBER, EQ, HEX_NUMBER, NEQ, AND, OR, NOT, UMINUS, REGISTER, DEREF, VARIABLE

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
	{"0[xX][0-9a-fA-F]+", HEX_NUMBER, 0},   // hexadecimal number
	{"\\$[a-zA-Z]+", REGISTER, 0},      //register
	{"\\+", '+', 4},					// plus
	{"==", EQ, 3},						// equal
	{"!=", NEQ, 3},				  	    // not equal
	{"!", NOT, 6},    					// logical NOT
	{"[0-9]+", DEC_NUMBER, 0},	    	// decimal number
	{"\\(", '(', 7},					// left parenthesis
	{"\\)", ')', 7},					// right parenthesis
	{"-", '-', 4},						// minus
	{"\\*", '*', 5},					//multiply
	{"/", '/', 5},						//divide
	{"&&", AND, 2},                      //and
	{"\\|\\|", OR, 1},                   //or
	{"[a_zA_Z_]{1,31}", VARIABLE, 0} 	//variable
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
					case NEQ:
					case AND:
					case OR:
					case NOT:
					case DEC_NUMBER:
					case HEX_NUMBER:
					case VARIABLE:
						tokens[nr_token].type = rules[i].token_type;
						tokens[nr_token].priority = rules[i].priority;
						strncpy(tokens[nr_token].str, substr_start, substr_len);
						tokens[nr_token].str[substr_len] = '\0';
						nr_token++;
						break;
					case REGISTER:
						tokens[nr_token].type = rules[i].token_type;
						tokens[nr_token].priority = rules[i].priority;
						strncpy(tokens[nr_token].str, substr_start + 1, substr_len - 1);
						tokens[nr_token].str[substr_len - 1] = '\0';
						nr_token++;
						break;
					default: panic("please implement me");
				}
				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}
	for (i = 0; i < nr_token; i++){
		if (tokens[i].type == '-'){
			if (i == 0 || (tokens[i - 1].type != DEC_NUMBER && tokens[i - 1].type != HEX_NUMBER && tokens[i - 1].type != REGISTER && tokens[i - 1].type != VARIABLE && tokens[i - 1].type != ')')){
				tokens[i].type = UMINUS;
				tokens[i].priority = 6;
			}
		}
		else if (tokens[i].type == '*'){
			if (i == 0 || (tokens[i - 1].type != DEC_NUMBER && tokens[i - 1].type != HEX_NUMBER && tokens[i - 1].type != REGISTER && tokens[i - 1].type != VARIABLE && tokens[i - 1].type != ')')){
				tokens[i].type = DEREF;
				tokens[i].priority = 6;
			}
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
			if (tokens[i].type == UMINUS || tokens[i].type == DEREF || tokens[i].type == NOT){
				continue;
			}
			if (tokens[i].priority >= 1){
				if (op_pos == -1 || tokens[i].priority <= min_priority){
					min_priority = tokens[i].priority;
					op_pos = i;
				}
			}
		}
	}
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

uint32_t get_reg_val(const char *reg_name){
	if (strlen(reg_name) == 3) {
		int i;
		for (i = R_EAX; i <= R_EDI; i++){
			if (strcmp(reg_name, regsl[i]) == 0){
				break;
			}
		}
		if (i > R_EDI){
			if (strcmp(reg_name, "eip") == 0)
				return cpu.eip;
			else{
				Assert(0,"No this register\n");
			}
		} 
		else{
			return reg_l(i);
		}
	} 
	else if (strlen(reg_name) == 2){
		if (reg_name[1] == 'x' || reg_name[1] == 'p' || reg_name[1] == 'i'){
			int i;
			for (i = R_AX; i <= R_DI; i++){
				if (strcmp(reg_name, regsw[i]) == 0){
					break;
				}
			}
			if (i > R_DI){
				Assert(0, "No this register!\n");
			}
			return reg_w(i);
		} 
		else if (reg_name[1] == 'l' || reg_name[1] == 'h'){
			int i;
			for (i = R_AL; i <= R_BH; i++){
				if (strcmp(reg_name, regsb[i]) == 0){
					break;
				}
			}
			if (i > R_BH){
				Assert(0, "No this register!\n");
			}
			return reg_b(i);
		} 
		else{
			Assert(0, "No this register!\n");
		}
	}
	else {
		Assert(0, "No this register!\n");
	}
}

uint32_t eval(int p, int q){
	if (p > q) {
		assert(0);
	}
	else if (p == q){
		uint32_t value = 0;
		if (tokens[p].type == DEC_NUMBER){
			sscanf(tokens[p].str, "%d", &value);
			return value;
		}
		if (tokens[p].type == HEX_NUMBER){
			sscanf(tokens[p].str, "%x", &value);
			return value;
		}
		if (tokens[p].type == REGISTER){
			return get_reg_val(tokens[p].str);
		}
		if (tokens[p].type == VARIABLE){
			bool success = true;
			value = get_sym_val(tokens[p].str, &success);
			if (success){
				return value;
			}
			else{
				printf("No this variable!\n");
				assert(0);
			}
		}
	}
	if (check_parentheses(p, q) == true){
		return eval(p + 1, q - 1);
	}
	int op = dominant_operator(p, q);
	if (op == -1){
		if (tokens[p].type == UMINUS){
			return -eval(p + 1, q);
		}
		if (tokens[p].type == DEREF){
			return swaddr_read(eval(p + 1, q), 4);
		}
		if (tokens[p].type == NOT){
			return !eval(p + 1, q);
		}
		assert(0);
	}
	uint32_t val1 = eval(p, op - 1);
	uint32_t val2 = eval(op + 1, q);
	switch (tokens[op].type){
		case '+': return val1 + val2;
		case '-': return val1 - val2;
		case '*': return val1 * val2;
		case '/': return val1 / val2;
		case EQ: return val1 == val2;
		case NEQ: return val1 != val2;	
		case AND: return val1 && val2;
		case OR: return val1 || val2;
		default: assert(0);
	}
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

