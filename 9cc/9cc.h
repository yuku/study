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
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;  // Node kind
  Node *lhs;      // Left-hand side
  Node *rhs;      // Right-hand side
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
