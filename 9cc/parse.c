#include <stdlib.h>
#include <string.h>

#include "9cc.h"


typedef struct LVar LVar;

struct LVar {
  LVar *next;  // Next local variable or NULL
  char *name;  // Variable name
  int len;     // Length of the name
  int offset;  // Offset from RBP
};

LVar *locals;

/**
 * Consume returns true and move the `token` to next one if the given
 * char equals the current string. Otherwise it returns false.
 */
bool consume(char *op) {
  if (token->kind == TK_IDENT ||
      token->kind == TK_NUM ||
      token->kind == TK_EOF) {
    return false;
  }
  if (strlen(op) != token->len ||
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
  if (!consume(op)) {
    error_at(token->str, "Current char is not equal to '%s'", op);
  }
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
 * Find_lvar returns LVar which has the specified name. Otherwise, returns NULL.
 */
LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next) {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
      return var;
    }
  }
  return NULL;
}

Node *new_node(NodeKind kind) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}


Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_num(int val) {
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

Node *expr();
Node *stmt();

// args = expr ("," args)
Node *args() {
  Node *node = new_node(ND_ARGS);
  node->lhs = expr();

  if (consume(",")) {
    node->rhs = args();
  }
  return node;
}

// primary = "(" expr ")"
//         | num
//         | ident ("(" args? ")")?
Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }
  Token *tok = consume_ident();
  if (tok) {
    if (consume("(")) {
      // Function call.
      Node *node = new_node(ND_CALL);
      node->name = tok->str;
      node->len = tok->len;
      if (tok->len > 32) {
        error_at(token->str - 2, "Too long function name");
      }

      if (consume(")")) {
        // Without arguments
        return node;
      }

      node->lhs = args();
      expect(")");
      return node;
    }

    // Local variable.
    Node *node = new_node(ND_LVAR);

    LVar *lvar = find_lvar(tok);
    if (!lvar) {
      lvar = calloc(1, sizeof(LVar));
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;
      lvar->offset = locals->offset + 8;
      locals = lvar;
    }
    node->offset = lvar->offset;
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
//      | "{" stmt* "}"
//      | "return" expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
Node *stmt() {
  Node *node;

  if (consume("{")) {
    node = new_node(ND_BLOCK);

    Node head;
    head.next = NULL;
    Node *cur = &head;
    while (!consume("}")) {
      cur->next = stmt();
      cur = cur->next;
    }
    node->lhs = cur == &head ? NULL : head.next;
    return node;
  }
  
  if (consume("return")) {
    node = new_node(ND_RETURN);
    node->lhs = expr();
    expect(";");
    return node;
  }
  
  if (consume("if")) {
    node = new_node(ND_IF);
    expect("(");
    node->cond = expr();
    expect(")");
    node->lhs = stmt();
    if (consume("else")) {
      node->rhs = stmt();
    }
    return node;
  }

  if (consume("while")) {
    node = new_node(ND_WHILE);
    expect("(");
    node->cond = expr();
    expect(")");
    node->lhs = stmt();
    return node;
  }

  if (consume("for")) {
    node = new_node(ND_FOR);
    expect("(");
    if (!consume(";")) {
      node->init = expr();
      expect(";");
    }
    if (!consume(";")) {
      node->cond = expr();
      expect(";");
    }
    if (!consume(")")) {
      node->update = expr();
      expect(")");
    }
    node->lhs = stmt();
    return node;
  }

  node = expr();
  expect(";");
  return node;
}

// program = stmt*
void program() {
  locals = calloc(1, sizeof(LVar));

  Node head;
  head.next = NULL;
  Node *cur = &head;

  while (!at_eof()) {
    cur->next = stmt();
    cur = cur->next;
  }
  code = head.next;
}
