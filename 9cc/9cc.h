#include <stdbool.h>

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
  ND_EQ,   // ==
  ND_NE,   // !=
  ND_LT,   // <
  ND_LE,   // <=
  ND_NUM,  // number
} NodeKind;

typedef struct Token Token;
typedef struct Node Node;

struct Token {
  TokenKind kind;  // Token kind
  Token *next;     // Next token
  int val;         // value if kind is TK_NUM
  char *str;       // Token string
  int len;         // Token length
};

struct Node {
  NodeKind kind;  // Node kind
  Node *lhs;      // Left-hand side
  Node *rhs;      // Right-hand side
  int val;        // Valud if kind is ND_NUM
};

// Current token
extern Token *token;

// Input program
extern char *user_input;

void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);
bool consume(char *op);
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
Token *tokenize(char *p);
Node *new_binary(NodeKind kind, Node *lhs, Node *rhs);
Node *new_num(int val);
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();
void gen(Node *node);
