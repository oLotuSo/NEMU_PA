#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

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

static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_p(char *args);

static int cmd_x(char *args);

static int cmd_w(char *args);

static int cmd_d(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  { "si", "Single step execution", cmd_si},
  { "info", "Print status of registers or watchpoints", cmd_info },
  { "p", "Expression evaluation", cmd_p},
  { "x", "Scan the memory", cmd_x },
  { "w", "Set watchpoints", cmd_w},
  { "d", "Delete watchpoints", cmd_d}
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char* args){
  //use "strtok" to get "N"
  char* buf = strtok(NULL, " ");
  //N可能为空
  if(buf == NULL){
    cpu_exec(1);//缺省为1
  }else{//cpu_exec(N)
    if(strtok(NULL, " ") != NULL){
      printf("[Error] Too many arguments in si!\n");
      return 0;
    }
    int N = atoi(buf);
    if(N >= -1){
      cpu_exec(N);
    }else{
      printf("[Error] Argument needs to be positive(except -1)!\n");
    }
  }
  return 0;
}

static int cmd_info(char* args){
  char* arg = strtok(NULL, " ");
  if(arg == NULL){
    printf("[Error] Info needs an argument r or w!\n");
  }
  if (strcmp(arg, "r") == 0) {
    // 依次打印所有寄存器
    // 三列分别为：寄存器名、十六进制形式、十进制形式
    printf("RegName\tHEX    \t\tDEC\t\n");
    // 32位寄存器
    for (int i = 0; i < 8; i++) {
        printf("%s:\t%#010x\t%d\t\n", regsl[i], cpu.gpr[i]._32, cpu.gpr[i]._32);
    }
    printf("eip:\t%#010x\t%d\t\n",cpu.eip, cpu.eip); //eip
    // 16位寄存器
    for (int i = 0; i < 8; i++) {
        printf("%s:\t%#010x\t%d\t\n", regsw[i], cpu.gpr[i]._16, cpu.gpr[i]._16);
    }
    // 8位寄存器,分h&l
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 2; j++) {
            printf("%s:\t%#010x\t%d\t\n", regsb[i], cpu.gpr[i]._8[j], cpu.gpr[i]._8[j]);
        }
    }
  }else if(strcmp(arg, "w") == 0){
    list_watchpoint();
  }else{
    printf("[Error] Info argument invalid!\n");
  }
  return 0;
}

static int cmd_x(char *args){
  char* arg1 = strtok(NULL, " ");// get first argument N
  if(arg1 == NULL){
    printf("[Error] x needs an argument N!\n");
    return 0;
  }
  int len = atoi(arg1);
  vaddr_t addr;
  char *arg2 = strtok(NULL, " ");// get second argument expr
  if(arg2 == NULL){
    printf("[Error] x needs an argument expr!\n");
    return 0;
  }
  printf("Address     Dword block ... Byte sequence\n");
  sscanf(arg2,"%x",&addr);
  for(int i = 0; i < len; i++){
    printf("0x%08x  ",addr);
    printf("0x%08x  ... ",vaddr_read(addr,4));
    uint32_t byte = vaddr_read(addr,4);
    for(int j = 0; j < 3; j++) {
      printf("%02x ", byte & 0xff);
      byte >>=8;
    }
    printf("%02x\n", byte & 0xff);
    addr+=4;
  }
  printf("\n");
  return 0;
}

static int cmd_p(char *args){
  char* arg = strtok(NULL, " ");
  if(arg == NULL){
    printf("[Error] Info needs an argument expr!\n");
    return 0;
  }
  char* next = strtok(NULL, " ");
  while (next != NULL) {
    strcat(arg, next);
    next = strtok(NULL, " ");
  }
  bool success = true;
  uint32_t ans = expr(arg,&success);
  if(success){
    printf("Unsigned_val = %u, Integer_val = %d \n",ans, ans);
  }
  else
  {
    printf("[Error] Lexical analysis failed!\n");
  }
  return 0;
}


static int cmd_w(char* args){
  char* arg = strtok(NULL, " ");
  if(arg == NULL){
    printf("[Error] Setting watchpoints needs an argument expr!\n");
    return 0;
  }
  char* next = strtok(NULL, " ");
  while (next != NULL) {
    strcat(arg, next);
    next = strtok(NULL, " ");
  }
  bool success = true;
  uint32_t ans = expr(arg,&success);
  if(!success){
    printf("[Error] Incorrect expression!\n");
    return 0;
  }
  WP* wp = new_wp();
  wp->old_val = ans;
  wp->expr = (char *)malloc(strlen(arg) * sizeof(char));
  memset(wp->expr, 0, strlen(arg));
  strcpy(wp->expr, arg);
  printf("Set watchpoint #%d\nexpr\t  = %s\nold value = %#010x\n",wp->NO, wp->expr, wp->old_val);
  return 0;
}

static int cmd_d(char* args){
  char* arg = strtok(NULL, " ");
  if(arg == NULL){
    printf("[Error] Deleting watchpoints needs an argument n!\n");
    return 0;
  }
  free_wp(find_wp(atoi(arg)));
  printf("Watchpoint %d delete!\n",atoi(arg));
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
