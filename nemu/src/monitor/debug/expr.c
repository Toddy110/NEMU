/* Clean merged implementation */
#include "nemu.h"
#include <sys/types.h>
#include <regex.h>
#include <string.h>

extern uint32_t get_sym_val(char *syc, bool *success);

enum {NOTYPE = 256, DEC_NUMBER, EQ, HEX_NUMBER, NEQ, AND, OR, NOT, UMINUS, REGISTER, DEREF, VARIABLE
};

struct rule {
    char *regex;
    int token_type;
    int priority;
};

static struct rule rules[] = {
    {" +", NOTYPE, 0},
    {"0[xX][0-9a-fA-F]+", HEX_NUMBER, 0},
    {"\\$[a-zA-Z]+", REGISTER, 0},
    {"\\+", '+', 4},
    {"==", EQ, 3},
    {"!=", NEQ, 3},
    {"!", NOT, 6},
    {"[0-9]+", DEC_NUMBER, 0},
    {"\\(", '(', 7},
    {"\\)", ')', 7},
    {"-", '-', 4},
    {"\\*", '*', 5},
    {"/", '/', 5},
    {"&&", AND, 2},
    {"\\|\\|", OR, 1},
    {"[_a-zA-Z][_a-zA-Z0-9]{0,30}", VARIABLE, 0}
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]))
static regex_t re[NR_REGEX];

void init_regex() {
    int i;
    for (i = 0; i < NR_REGEX; ++i) {
        int ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
        if (ret) {
            char msg[128];
            regerror(ret, &re[i], msg, sizeof(msg));
            Assert(0, "regex fail: %s", msg);
        }
    }
}

typedef struct token {
    int type;
    char str[32];
    int priority;
} Token;
static Token tokens[64];
static int nr_token;

static bool make_token(char *e) {
    int pos = 0;
    nr_token = 0;
    regmatch_t pm;
    while (e[pos] != '\0') {
        int i;
        for (i = 0; i < NR_REGEX; i++) {
            if (regexec(&re[i], e + pos, 1, &pm, 0) == 0 && pm.rm_so == 0) {
                int len = pm.rm_eo;
                char *s = e + pos;
                pos += len;
                int tt = rules[i].token_type;
                if (tt == NOTYPE) break;
                {
                    Token *t = &tokens[nr_token];
                    t->type = tt;
                    t->priority = rules[i].priority;
                    if (tt == REGISTER) {
                        if (len - 1 > 31) len = 32;
                        strncpy(t->str, s + 1, len - 1);
                        t->str[len - 1] = '\0';
                    }
                    else {
                        if (len > 31) len = 32;
                        strncpy(t->str, s, len);
                        t->str[len] = '\0';
                    }
                    nr_token++;
                }
                break;
            }
        }
        if (i == NR_REGEX) {
            printf("no match at %d\n%s\n%*.s^\n", pos, e, pos, "");
            return false;
        }
    }
    {
        int i;
        for (i = 0; i < nr_token; i++) {
            if (tokens[i].type == '-') {
                if (i == 0 || (tokens[i - 1].type != DEC_NUMBER && tokens[i - 1].type != HEX_NUMBER &&
                    tokens[i - 1].type != REGISTER && tokens[i - 1].type != VARIABLE && tokens[i - 1].type != ')')) {
                    tokens[i].type = UMINUS;
                    tokens[i].priority = 6;
                }
            }
            else if (tokens[i].type == '*') {
                if (i == 0 || (tokens[i - 1].type != DEC_NUMBER && tokens[i - 1].type != HEX_NUMBER &&
                    tokens[i - 1].type != REGISTER && tokens[i - 1].type != VARIABLE && tokens[i - 1].type != ')')) {
                    tokens[i].type = DEREF;
                    tokens[i].priority = 6;
                }
            }
        }
    }
    return true;
}

static bool check_parentheses(int p, int q) {
    int par = 0;
    int i;
    if (tokens[p].type != '(' || tokens[q].type != ')') return false;
    for (i = p + 1; i < q; i++) {
        if (tokens[i].type == '(') par++;
        else if (tokens[i].type == ')') par--;
        if (par < 0) return false;
    }
    return par == 0;
}

static int dominant_operator(int p, int q) {
    int par = 0, minp = 20, op = -1;
    int i;
    for (i = p; i <= q; i++) {
        int tp = tokens[i].type;
        if (tp == '(') par++;
        else if (tp == ')') par--;
        else if (par == 0) {
            if (tp == UMINUS || tp == DEREF || tp == NOT) continue;
            if (tokens[i].priority >= 1) {
                if (op == -1 || tokens[i].priority <= minp) {
                    minp = tokens[i].priority;
                    op = i;
                }
            }
        }
    }
    return op;
}

static uint32_t get_reg_val(const char *r) {
    int i;
    size_t len = strlen(r);
    if (len == 3) {
        for (i = R_EAX; i <= R_EDI; i++) if (strcmp(r, regsl[i]) == 0) return reg_l(i);
        if (strcmp(r, "eip") == 0) return cpu.eip;
        Assert(0, "No reg");
    }
    else if (len == 2) {
        if (r[1] == 'x' || r[1] == 'p' || r[1] == 'i') {
            for (i = R_AX; i <= R_DI; i++) if (strcmp(r, regsw[i]) == 0) return reg_w(i);
            Assert(0, "No reg");
        }
        else if (r[1] == 'l' || r[1] == 'h') {
            for (i = R_AL; i <= R_BH; i++) if (strcmp(r, regsb[i]) == 0) return reg_b(i);
            Assert(0, "No reg");
        }
        else Assert(0, "No reg");
    }
    else Assert(0, "No reg");
}

static uint32_t eval(int p, int q) {
    if (p > q) Assert(0, "bad range");
    if (p == q) {
        uint32_t v = 0;
        switch (tokens[p].type) {
        case DEC_NUMBER:
            sscanf(tokens[p].str, "%d", &v);
            return v;
        case HEX_NUMBER:
            sscanf(tokens[p].str, "%x", &v);
            return v;
        case REGISTER:
            return get_reg_val(tokens[p].str);
        case VARIABLE: {
            bool s = true;
            v = get_sym_val(tokens[p].str, &s);
            if (!s) {
                printf("No this variable!\n");
                Assert(0, "var");
            }
            return v;
        }
        default:
            Assert(0, "leaf");
        }
    }
    if (check_parentheses(p, q)) return eval(p + 1, q - 1);
    {
        int op = dominant_operator(p, q);
        if (op == -1) {
            switch (tokens[p].type) {
            case UMINUS: return -eval(p + 1, q);
            case DEREF: return swaddr_read(eval(p + 1, q), 4);
            case NOT: return !eval(p + 1, q);
            default: Assert(0, "unary");
            }
        }
        else {
            uint32_t l = eval(p, op - 1), r = eval(op + 1, q);
            switch (tokens[op].type) {
            case '+': return l + r;
            case '-': return l - r;
            case '*': return l * r;
            case '/': return l / r;
            case EQ: return l == r;
            case NEQ: return l != r;
            case AND: return l && r;
            case OR: return l || r;
            default: Assert(0, "bin");
            }
        }
    }
}

uint32_t expr(char *e, bool *success) {
    if (!make_token(e)) {
        *success = false;
        return 0;
    }
    return eval(0, nr_token - 1);
}
