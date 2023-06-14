#include "nemu.h"
#include "stdlib.h"
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */
  TK_NEQ, TK_NOT, TK_AND, TK_OR,
  TK_HEX, TK_DEC, TK_REG,
  TK_MIN, TK_PTR
};

static struct rule
{
  char *regex;
  int token_type;
  } rules[] = {

      /* TODO: Add more rules.
       * Pay attention to the precedence level of different rules.
       */

      {"0[xX][a-fA-F0-9]+", TK_HEX},
      {"0|[1-9][0-9]*", TK_DEC},   //不考虑负数,用dominated_op实现负数
      {"\\$[a-dA-D][hlHL]|\\$[eE]?(ax|dx|cx|bx|bp|si|di|sp|ip)", TK_REG},
      {"==", TK_EQ},         // equal
      {"!=", TK_NEQ},
      {"&&", TK_AND},
      {"\\|\\|", TK_OR}, 
      {"\\(", '('}, 
      {"\\)", ')'}, 
      {"\\*", '*'},         
      {"\\/", '/'},        
      {"\\+", '+'},        
      {"\\-", '-'}, 
      {"!", TK_NOT},      
      {" +", TK_NOTYPE}    // spaces
      //按照最长前缀原则，长的在前，短的在后
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

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;
        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE:break;
          case TK_REG:
          case TK_DEC:
          case TK_HEX:
            for(int j = 0;j < substr_len; j++)
              tokens[nr_token].str[j] = substr_start[j];
            tokens[nr_token].str[substr_len] = '\0';
          default:
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
        }
	//对于数字和寄存器要存数据，其他存类型即可
        break;
      }
    }
    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

int priority_table(int type){
  switch(type){
    case TK_NOT:case TK_MIN:case TK_PTR:return 1;
    case '*':case '/':return 2;
    case '+':case '-':return 3;
    case TK_EQ:case TK_NEQ:return 4;
    case TK_AND:return 5;
    case TK_OR:return 6;
    default:return -1;
  }
}
int find_dominated_op(int p, int q, bool *success){
  //将不被括号包围的运算符从左到右排列，找到其中优先级最低且最右的
  //单目运算符要左结合
  int cnt = 0, index = -1, max = 0;
  for(int i = p; i <= q; i++){
     if(tokens[i].type == '(')cnt++;
     else if(tokens[i].type == ')')cnt--;
     else{
       if(cnt != 0)continue;
       if(priority_table(tokens[i].type) >= max){
	      if(max == 1 && priority_table(tokens[i].type) == 1)continue;
	      else{
          max = priority_table(tokens[i].type);
          index = i;
	      }
       }
     }
  }
  if(index < 0) *success = false;
  return index;
}
bool check_parentheses(int p, int q, bool *success){
    //1.被括号包围；2.括号是匹配的
    int cnt = 0;//栈计数器
    bool flag = true;
    if(tokens[p].type != '(' || tokens[q].type != ')') flag=false;
    if(tokens[p].type == '(' && tokens[q].type == ')'){
      cnt = 1;
      for(int i = p + 1; i < q; i++){
        if(tokens[i].type == '(')cnt++;
        else if(tokens[i].type == ')'){
	        cnt--;
	        if(cnt == 0)flag=false;
	        else if(cnt < 0)break;
	      }else continue;
      }
    }
    if(cnt < 0)*success = false;
    return (cnt == 1) && flag;
}

uint32_t eval(int p, int q, bool *success) {
    if (p > q) {
        /* Bad expression */
        *success = false;
        return 0;
    }
    else if (p == q) {
        /* Single token.
        * For now this token should be a number.
        * Return the value of the number.
        */
	    switch(tokens[p].type){
	      case TK_DEC:case TK_HEX:
	        return strtoul(tokens[p].str, 0, 0);
	      case TK_REG:
	        if(strcmp(&tokens[p].str[1], "eip") == 0) return cpu.eip;
          else{
	          for(int i = 0;i < 8; i++)
		          if(strcmp(&tokens[p].str[1], regsl[i]) == 0) return cpu.gpr[i]._32;
	        }
	      default: *success = false; return 0;
	    }
    }
    else if (check_parentheses(p, q, success) == true) {
        /* The expression is surrounded by a matched pair of parentheses.
        * If that is the case, just throw away the parentheses.
        */
        return eval(p + 1, q - 1, success);
    }
    else {
        /* We should do more things here. */
        if(!*success)return 0;
        int op = find_dominated_op(p, q, success);
        uint32_t val2 = eval(op + 1, q, success);
        if(tokens[op].type == TK_NOT){
	        if(*success)return (!val2);
	        else return 0;  
	      }else if(tokens[op].type == TK_MIN){
	        if(*success)return -1*val2;
	        else return 0;  
	      }else if(tokens[op].type == TK_PTR){
	        if(*success)return vaddr_read(val2, 4);
	        else return 0;  
	      }
        uint32_t val1 = eval(p, op - 1, success);
        switch (tokens[op].type) {
            case '+': return val1 + val2;
            case '-': 
		          if(val1 < val2)printf("[Warning] We use unsigned subtraction here!\n");
		          return val1 - val2;
            case '*': return val1 * val2;
            case '/': 
		          if(val2 == 0){
		            *success = false;
		            printf("[Error] ZeroDivisionError happened!\n");
		            return 0;
		          }
              return val1 / val2;
            case TK_EQ: return val1 == val2;
            case TK_NEQ: return val1 != val2;
            case TK_AND: return val1 && val2;
            case TK_OR: return val1 || val2;
            default: assert(0);
        }
    }
}

uint32_t expr(char *e, bool *success) {
  //需要判断1.词法可识别 2.表达式可计算
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  for (int i = 0; i < nr_token; i++) {
    if (tokens[i].type == '-' && (i == 0 || (tokens[i - 1].type != TK_DEC && tokens[i - 1].type  != TK_HEX && tokens[i - 1].type  != TK_REG && tokens[i - 1].type != ')'))) {
      tokens[i].type = TK_MIN;
    }
    else if (tokens[i].type == '*' && (i == 0 || (tokens[i - 1].type != TK_DEC && tokens[i - 1].type  != TK_HEX && tokens[i - 1].type  != TK_REG && tokens[i - 1].type != ')'))) {
      tokens[i].type = TK_PTR;
    }
  }
  return eval(0, nr_token - 1, success);
}
