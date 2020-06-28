#include <stdio.h>
#include <string.h>

#include "9cc.h"

int counter = 0;
char *arg_registers[] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };

void gen_node(Node *node);

void gen_lval(Node *node) {
  if (node->kind != ND_LVAR) {
    error("Left value of assignment is not a variable");
  }

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

void gen_node(Node *node) {
  if (!node) {
    return;
  }

  int c = counter++;

  switch (node->kind) {
  case ND_NUM:
    printf("  push %d\n", node->val);
    return;
  case ND_LVAR:
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  case ND_ASSIGN:
    gen_lval(node->lhs);
    gen_node(node->rhs);
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    return;
  case ND_RETURN:
    gen_node(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  case ND_IF:
    gen_node(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    if (node->rhs) {
      printf("  je .Lelse%d\n", c);
      gen_node(node->lhs);
      printf("  jmp .Lendif%d\n", c);
      printf(".Lelse%d:\n", c);
      gen_node(node->rhs);
    } else {
      printf("  je .Lendif%d\n", c);
      gen_node(node->lhs);
    }
    printf(".Lendif%d:\n", c);
    return;
  case ND_WHILE:
    printf(".Lbeginwhile%d:\n", c);
    gen_node(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lendwhile%d\n", c);
    gen_node(node->lhs);
    printf("  jmp .Lbeginwhile%d\n", c);
    printf(".Lendwhile%d:\n", c);
    return;
  case ND_FOR:
    gen_node(node->init);
    printf(".Lbeginfor%d:\n", c);
    if (node->cond) {
      gen_node(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .Lendfor%d\n", c);
    }
    gen_node(node->lhs);
    gen_node(node->update);
    printf("  jmp .Lbeginfor%d\n", c);
    printf(".Lendfor%d:\n", c);
    return;
  case ND_CALL: {
    int arg_num = 0;
    for (Node *arg = node->lhs; arg; arg = arg->next) {
      gen_node(arg);
      arg_num++;
    }

    for (int i = arg_num - 1; i >= 0; i--) {
      printf("  pop %s\n", arg_registers[i]);
    }

    char name[33]; // funcname is shorter than 33 chars.
    strncpy(name, node->name, node->len);
    name[node->len] = '\0';
    printf("  call %s\n", name);
    return;
  }
  case ND_BLOCK:
    gen(node->lhs);
    return;
  }

  gen_node(node->lhs);
  gen_node(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  }

  printf("  push rax\n");
}

/**
 * Gen outputs assembly to stdout for given node. 
 */
void gen(Node *node) {
  for (; node; node = node->next) {
    gen_node(node);
    // After evaluating expression, a value is left on the
    // top of the stack. Pop it to avoid stack overflow.
    printf("  pop rax\n");
  }
}
