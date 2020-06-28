#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

Token *token;

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
 * Isidentchar returns true if the given char can be used in identifier.
 */
bool isidentchar(char c) {
  return isalnum(c) || c == '_';
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

    if (startswith(p, "return") && !isidentchar(*(p + 6))) {
      cur = new_token(TK_RETURN, cur, p, 6);
      p += 6;
      continue;
    }

    if (startswith(p, "if") && !isidentchar(*(p + 2))) {
      cur = new_token(TK_IF, cur, p, 2);
      p += 2;
      continue;
    }

    if (startswith(p, "else") && !isidentchar(*(p + 4))) {
      cur = new_token(TK_ELSE, cur, p, 4);
      p += 4;
      continue;
    }

    if (startswith(p, "while") && !isidentchar(*(p + 5))) {
      cur = new_token(TK_WHILE, cur, p, 5);
      p += 5;
      continue;
    }

    if (startswith(p, "for") && !isidentchar(*(p + 3))) {
      cur = new_token(TK_FOR, cur, p, 3);
      p += 3;
      continue;
    }

    if (isalpha(*p) || *p == '_') {
      cur = new_token(TK_IDENT, cur, p, 0);
      char *q = p;
      while (isidentchar(*p)) {
        p++;
      }
      cur->len = p - q;
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

    if (strchr("+-*/(){}<>=;,", *p)) {
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
