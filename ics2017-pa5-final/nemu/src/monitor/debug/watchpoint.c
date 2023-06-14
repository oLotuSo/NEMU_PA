#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp(){
  if(free_ == NULL){
    printf("[Error] No watchpoint remain!\n");
    assert(0);
  }
  WP* temp = free_;
  free_ = free_->next;
  temp->next = head;
  head = temp;
  return temp; 
}

void free_wp(WP *wp){
  if(wp == NULL) assert(0);
  int n = wp->NO;
  WP* temp = head, *before = NULL;
  while(temp){
    if(temp->NO == n){
      if(before) before->next = temp->next;
      else head = head->next;
      temp->next = free_;
      free_ = temp;
      break;
    }
    before = temp;
    temp = temp->next;
  }
  if(!temp){
    printf("[Error] Fail to find the watchpoint!\n");
    assert(0);
  }
}

WP* find_wp(int n){
  if(n >= 0 && n < NR_WP)return &wp_pool[n];
  return NULL;
}

void list_watchpoint(){
  if(!head){ 
      printf("No watchpoint exists!\n");
      return;
  }
  WP* fast, *slow, *fast_head = head->next;
  printf("NO Expr         Old Value\n");
  while(fast_head){
    slow = head;
    fast = fast_head;
    while(fast) {
       slow = slow->next;
       fast = fast->next;
    }
    printf("%d  %s\t\t%#010x\n",slow->NO, slow->expr, slow->old_val);
    fast_head = fast_head->next;
  }
  printf("%d  %s\t\t%#010x\n",head->NO, head->expr, head->old_val);
  //这里用双指针逆序打印不可改变链表，参考leetcode 1265
}

bool scan_watchpoint(int eip){
  if(!head) return false;
  WP* temp = head;
  bool success = true, flag = false;
  while(temp){
    temp->new_val = expr(temp->expr, &success);
    if(!success){
      printf("[Error] Invalid expreesion!\n");
      continue;
    }
    if(temp->new_val != temp->old_val){
      flag = true;
      printf("Hit watchpoint %d at address %#010x\nexpr\t  = %s\nold value = %#010x\nnew value = %#010x\n",
      temp->NO, eip, temp->expr, temp->old_val, temp->new_val);
      printf("program paused\n");
      temp->old_val = temp->new_val;
    }
    temp = temp->next;
  }
  return flag;
}
