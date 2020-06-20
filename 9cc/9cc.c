#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  TK_RESERVED,  // Symbol
  TK_NUM,       // Integer token
  TK_EOF,       // End of input
} TokenKind;

typedef enum {
  ND_ADD,  // +
  ND_SUB,  // -
  ND_MUL,  // *
  ND_DIV,  // /
  ND_NUM,  // number
} NodeKind;

typedef struct Token Token;
typedef struct Node Node;

// Current token
Token *token;

// Input program
char *user_input;

void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);
bool consume(char op);
void expect(char op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str);
Token *tokenize(char *p);
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *expr();
Node *mul();
Node *unary();
Node *primary();
void gen(Node *node);

struct Token {
  TokenKind kind;  // Token kind
  Token *next;     // Next token
  int val;         // value if kind is TK_NUM
  char *str;       // Token string
};

struct Node {
  NodeKind kind;  // Node kind
  Node *lhs;      // Left-hand side
  Node *rhs;      // Right-hand side
  int val;        // Valud if kind is ND_NUM
};

void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, "");  // write pos whitespaces.
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

/**
 * Error function reports error.
 *
 * It takes the same arguments as printf.
 */
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

/**
 * Consume returns true and move the `token` to next one if the given
 * char equals the current string. Otherwise it returns false.
 */
bool consume(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op) {
    return false;
  }
  token = token->next;
  return true;
}

/**
 * Expect moves the `token` to next one if the given char equals the
 * current string. Otherwise it reports an error then exits.
 */
void expect(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op) {
    error_at(token->str, "Current char is not equal to '%c", op);
  }
  token = token->next;
}

/**
 * Expect_number returns the number if the `token` is number and moves
 * it to the next one. Otherwise it reports an error then exits.
 */
int expect_number() {
  if (token->kind != TK_NUM) {
    error_at(token->str, "It is not a number");
  }
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

/**
 * New_token creates a new token and connects it to the `cur`. 
 */
Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str,
  cur->next = tok;
  return tok;
}

/**
 * Tokenize tokenizes the given `p` and returns it.
 */
Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    // Skip whitespace chars
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error_at(p, "Cannot tokenize");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

Node *expr() {
  Node *node = mul();

  while (true) {
    if (consume('+')) {
      node = new_node(ND_ADD, node, mul());
    } else if (consume('-')) {
      node = new_node(ND_SUB, node, mul());
    } else {
      return node;
    }
  }
}

Node *mul() {
  Node *node = unary();

  while (true) {
    if (consume('*')) {
      node = new_node(ND_MUL, node, unary());
    } else if (consume('/')) {
      node = new_node(ND_DIV, node, unary());
    } else {
      return node;
    }
  }
}

Node *unary() {
  if (consume('+')) {
    return primary();
  } else if (consume('-')) {
    return new_node(ND_SUB, new_node_num(0), primary());
  } else {
    return primary();
  }
}

Node *primary() {
  if (consume('(')) {
    Node *node = expr();
    expect(')');
    return node;
  }
  return new_node_num(expect_number());
}

void gen(Node *node) {
  if (node->kind == ND_NUM) {
    printf("  push %d\n", node->val);
    return ;
  }

  gen(node->lhs);
  gen(node->rhs);

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
  }

  printf("  push rax\n");
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Invalid number of arguments\n");
    return 1;
  }

  user_input = argv[1];
  token = tokenize(user_input);
  Node *node = expr();

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");
  
  gen(node);

  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
