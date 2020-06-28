#include <stdbool.h>

// tokenize.c

typedef enum {
  TK_RESERVED,  // Reserved keywords
  TK_IDENT,     // Identifier
  TK_NUM,       // Integer token
  TK_EOF,       // End of input
  TK_RETURN,    // return
  TK_IF,        // if
  TK_ELSE,      // else
  TK_WHILE,     // while
  TK_FOR,       // for
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind;  // Token kind
  Token *next;     // Next token
  int val;         // value if kind is TK_NUM
  char *str;       // Token string
  int len;         // Token length
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
void tokenize();

// Current token
extern Token *token;

// parse.c

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
  ND_RETURN,  // return
  ND_IF,      // if
  ND_WHILE,   // while
  ND_FOR,     // for
  ND_BLOCK,   // block
  ND_CALL,    // function call
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;  // Node kind
  Node *next;     // Next node
  Node *lhs;      // Left-hand side
  Node *rhs;      // Right-hand side
  Node *init;     // Initialize expression if kind is ND_FOR
  Node *cond;     // Condition expression if kind is ND_IF, ND_WHILE or ND_FOR
  Node *update;   // Update expression if kind is ND_FOR
  int val;        // Value if kind is ND_NUM
  int offset;     // Offset from RBP

  char* name;     // Function name
  int len;        // Length of function name
};

// Input program
extern char *user_input;

extern Node *code;

void program();
void gen(Node *node);
