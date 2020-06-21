#include <stdio.h>

#include "9cc.h"

char *user_input;
Node *code;

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Invalid number of arguments\n");
    return 1;
  }

  // Tokenize then parse user inputs.
  // Parse result is assigned to `code`.
  user_input = argv[1];
  tokenize();
  program();

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // Prologue
  // Allocate the stack area to 26 local variables.
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n"); // 8 * 26

  gen(code);

  // Epilogue
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  return 0;
}
