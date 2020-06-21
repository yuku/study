#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

// Tokenizer

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
 * New_token creates a new token and connects it to the `cur`. 
 */
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

bool startswith(char *p, char *q) {
  return memcmp(p, q, strlen(q)) == 0;
}

/**
 * Tokenize tokenizes the given `p` and returns it.
 */
void tokenize() {
  char *p = user_input;
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    // Skip whitespace chars
    if (isspace(*p)) {
      p++;
      continue;
    }

    if ('a' <= *p && *p <= 'z') {
      cur = new_token(TK_IDENT, cur, p++, 1);
      continue;
    }

    if (startswith(p, ">=") ||
        startswith(p, "<=") ||
        startswith(p, "!=") ||
        startswith(p, "==")) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if (strchr("+-*/()<>=;", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    error_at(p, "Cannot tokenize");
  }

  new_token(TK_EOF, cur, p, 0);
  token = head.next;
}

// Parser

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
bool consume(char *op) {
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {
    return false;
  }
  token = token->next;
  return true;
}

/**
 * Consume_ident returns the current token if it is an identifier.
 */
Token *consume_ident() {
  Token *tok;
  if (token->kind == TK_IDENT) {
    tok = token;
    token = token->next;
    return tok;
  }
  return NULL;
}

/**
 * Expect moves the `token` to next one if the given char equals the
 * current string. Otherwise it reports an error then exits.
 */
void expect(char *op) {
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {
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

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

Node *new_lvar(int offset) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_LVAR;
  node->offset = offset;
  return node;
}

Node *expr();

// primary = "(" expr ")" | num | ident
Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }
  Token *tok = consume_ident();
  if (tok) {
    Node *node = new_lvar((tok->str[0] - 'a' + 1) * 8);
    return node;
  }
  return new_num(expect_number());
}

// unary = ("+" | "-")? unary | primary
Node *unary() {
  if (consume("+")) {
    return primary();
  } else if (consume("-")) {
    return new_binary(ND_SUB, new_num(0), primary());
  } else {
    return primary();
  }
}


// mul = unary ("*" unary | "/" unary)*
Node *mul() {
  Node *node = unary();

  while (true) {
    if (consume("*")) {
      node = new_binary(ND_MUL, node, unary());
    } else if (consume("/")) {
      node = new_binary(ND_DIV, node, unary());
    } else {
      return node;
    }
  }
}

// add = mul ("+" mul | "-" mul)*
Node *add() {
  Node *node = mul();

  while (true) {
    if (consume("+")) {
      node = new_binary(ND_ADD, node, mul());
    } else if (consume("-")) {
      node = new_binary(ND_SUB, node, mul());
    } else {
      return node;
    }
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
  Node *node = add();

  while (true) {
    if (consume("<")) {
      node = new_binary(ND_LT, node, add());
    } else if (consume("<=")) {
      node = new_binary(ND_LE, node, add());
    } else if (consume(">")) {
      node = new_binary(ND_LT, add(), node);
    } else if (consume(">=")) {
      node = new_binary(ND_LE, add(), node);
    } else {
      return node;
    }
  }
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality() {
  Node *node = relational();

  while (true) {
    if (consume("==")) {
      node = new_binary(ND_EQ, node, relational());
    } else if (consume("!=")) {
      node = new_binary(ND_NE, node, relational());
    } else {
      return node;
    }
  }
}

// assign = equality ("=" assign)?
Node *assign() {
  Node *node = equality();

  while (true) {
    if (consume("=")) {
      node = new_binary(ND_ASSIGN, node, assign());
    } else {
      return node;
    }
  }
}

// expr = assign
Node *expr() {
  return assign();
}

// stmt = expr ";"
Node *stmt() {
  Node *node = expr();
  expect(";");
  return node;
}

// program = stmt*
void program() {
  int i = 0;
  while (!at_eof()) {
    code[i++] = stmt();
  }
  code[i] = NULL;
}
