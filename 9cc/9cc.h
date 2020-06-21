#include <stdbool.h>

typedef enum {
  TK_RESERVED,  // Reserved keywords
  TK_IDENT,     // Identifier
  TK_NUM,       // Integer token
  TK_EOF,       // End of input
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind;  // Token kind
  Token *next;     // Next token
  int val;         // value if kind is TK_NUM
  char *str;       // Token string
  int len;         // Token length
};

typedef enum {
  ND_ADD,     // +
  ND_SUB,     // -
  ND_MUL,     // *
  ND_DIV,     // /
  ND_EQ,      // ==
  ND_NE,      // !=
  ND_LT,      // <
  ND_LE,      // <=
  ND_ASSIGN,  // =
  ND_LVAR,    // Local variable
  ND_NUM,     // Number
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;  // Node kind
  Node *lhs;      // Left-hand side
  Node *rhs;      // Right-hand side
  int val;        // Valud if kind is ND_NUM
  int offset;     // Offset from RBP
};

// Current token
extern Token *token;

// Input program
extern char *user_input;

extern Node *code[100];

void error(char *fmt, ...);
void tokenize();
void program();
void gen(Node *node);
