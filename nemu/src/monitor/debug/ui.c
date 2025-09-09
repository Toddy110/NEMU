#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_si(char *args) {
	int step_count = 1;

	if (args != NULL){
		char *arg = strtok(args, " ");
		if (arg != NULL){
			char *endptr;
			long parsed_count = strtol(arg, &endptr, 10);
			if (*endptr == '\0' && parsed_count > 0){
				step_count = (int)parsed_count;
			}
			else{
				printf("Invalid step count\n");
			}
		}
	}
	if (step_count < 10){
		cpu_exec(step_count);
	}
	else{
		cpu_exec(9);
	}
	return 0;
}

static int cmd_info(char *args){
	if (args == NULL){
		return 0;
	}
	
	char *subcmd = strtok(args, " ");
	if (subcmd == NULL){
		return 0;
	}
	if (strcmp(subcmd, "r") == 0){
		printf("eax            0x%08x\t%d\n", cpu.eax, cpu.eax);
		printf("ecx            0x%08x\t%d\n", cpu.ecx, cpu.ecx);
		printf("edx            0x%08x\t%d\n", cpu.edx, cpu.edx);
		printf("ebx            0x%08x\t%d\n", cpu.ebx, cpu.ebx);
		printf("esp            0x%08x\t%d\n", cpu.esp, cpu.esp);
		printf("ebp            0x%08x\t%d\n", cpu.ebp, cpu.ebp);
		printf("esi            0x%08x\t%d\n", cpu.esi, cpu.esi);
		printf("edi            0x%08x\t%d\n", cpu.edi, cpu.edi);
		printf("eip            0x%08x\t%d\n", cpu.eip, cpu.eip);
		printf("eflags         0x%08x\t%d\n", cpu.eflags.val, cpu.eflags.val);
		printf("CF=%d PF=%d AF=%d ZF=%d SF=%d TF=%d IF=%d DF=%d OF=%d\n",
			cpu.eflags.CF, cpu.eflags.PF, cpu.eflags.AF, cpu.eflags.ZF,
			cpu.eflags.SF, cpu.eflags.TF, cpu.eflags.IF, cpu.eflags.DF, cpu.eflags.OF);
	}
	else{
		return 0;
	}
	return 0;
}

static int cmd_x(char *args){
	if (args == NULL){
		return 0;
	}
	char *count_str = strtok(args, " ");
	if (count_str == NULL){
		return 0;
	}
	char * addr_str = strtok(NULL, " ");
	if (addr_str == NULL){
		return 0;
	}

	char *endptr = NULL;
	long count = strtol(count_str, &endptr, 10);
	if (*endptr != '\0' || count <= 0){
		return 0;
	}

	endptr = NULL;
	uint32_t addr = (uint32_t)strtoul(addr_str, &endptr, 16);
	if (*endptr != '\0'){
		return 0;
	}
	uint32_t current_addr = addr;
	for (uint32_t i = 0; i < count; i++){
		if ((i % 16) == 0){
			printf("0x%08x: ", current_addr);
		}
		uint8_t byte_val = (uint8_t)swaddr_read(current_addr, 1);
		printf("0x%02x ", byte_val);
		current_addr += 1;
		if ((i % 16) == 15){
			printf("\n");
		}
	}
	if ((count % 16) != 0){
		printf("\n");
	}
	return 0;
}

static int cmd_help(char *args);

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "si", "Single step execution (si [count])", cmd_si},
	{ "x", "Examine memory: x N EXPR (hex expr only)", cmd_x},
	{ "info", "Display information about the program state", cmd_info},
	{ "q", "Exit NEMU", cmd_q },

	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
