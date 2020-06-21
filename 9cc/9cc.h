#include <stdbool.h>

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
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;  // Node kind
  Node *lhs;      // Left-hand side
  Node *rhs;      // Right-hand side
  Node *init;     // Initialize expression if kind is ND_FOR
  Node *cond;     // Condition expression if kind is ND_IF, ND_WHILE or ND_FOR
  Node *update;   // Update expression if kind is ND_FOR
  int val;        // Valud if kind is ND_NUM
  int offset;     // Offset from RBP
};

// Input program
extern char *user_input;

extern Node *code[100];

void error(char *fmt, ...);
void tokenize();
void program();
void gen(Node *node);
