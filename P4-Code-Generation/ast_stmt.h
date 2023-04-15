/* File: ast_stmt.h
 * ----------------
 * The Stmt class and its subclasses are used to represent
 * statements in the parse tree.  For each statment in the
 * language (for, if, return, etc.) there is a corresponding
 * node class for that construct.
 */

#ifndef _H_ast_stmt
#define _H_ast_stmt

#include "list.h"
#include "ast.h"

class Decl;
class VarDecl;
class Expr;

class Program : public Node
{
protected:
  List<Decl *> *decls;

public:
  Program(List<Decl *> *declList);
  const char *ReturnNodeName() { return "Program"; }
  void ShowChildNodes(int indentLevel);

  void GenerateST();
  void Check();
  void Check(checkT c) { Check(); }

  void Emit();
};

class Stmt : public Node
{
public:
  Stmt() : Node() {}
  Stmt(yyltype loc) : Node(loc) {}
};

class StmtBlock : public Stmt
{
protected:
  List<VarDecl *> *decls;
  List<Stmt *> *stmts;

public:
  StmtBlock(List<VarDecl *> *variableDeclarations, List<Stmt *> *statements);
  const char *ReturnNodeName() { return "StmtBlock"; }
  void ShowChildNodes(int indentLevel);

  void GenerateST();
  void Check(checkT c);

  void Emit();
};

class ConditionalStmt : public Stmt
{
protected:
  Expr *test;
  Stmt *body;

public:
  ConditionalStmt(Expr *testExpr, Stmt *body);
};

class LoopStmt : public ConditionalStmt
{
protected:
  const char *LoopEndLabel;

public:
  LoopStmt(Expr *testExpr, Stmt *body)
      : ConditionalStmt(testExpr, body) {}
  bool IsLoop() { return true; }

  virtual const char *ReturnLoopLabel() { return LoopEndLabel; }
};

class ForStmt : public LoopStmt
{
protected:
  Expr *init, *step;
  void ConfirmType();

public:
  ForStmt(Expr *init, Expr *test, Expr *step, Stmt *body);
  const char *ReturnNodeName() { return "ForStmt"; }
  void ShowChildNodes(int indentLevel);

  void GenerateST();
  void Check(checkT c);

  void Emit();
};

class WhileStmt : public LoopStmt
{
protected:
  void ConfirmType();

public:
  WhileStmt(Expr *test, Stmt *body) : LoopStmt(test, body) {}
  const char *ReturnNodeName() { return "WhileStmt"; }
  void ShowChildNodes(int indentLevel);

  void GenerateST();
  void Check(checkT c);

  void Emit();
};

class IfStmt : public ConditionalStmt
{
protected:
  Stmt *elseBody;
  void ConfirmType();

public:
  IfStmt(Expr *test, Stmt *thenBody, Stmt *elseBody);
  const char *ReturnNodeName() { return "IfStmt"; }
  void ShowChildNodes(int indentLevel);

  void GenerateST();
  void Check(checkT c);

  void Emit();
};

class BreakStmt : public Stmt
{
public:
  BreakStmt(yyltype loc) : Stmt(loc) {}
  const char *ReturnNodeName() { return "BreakStmt"; }
  void Check(checkT c);

  void Emit();
};

class ReturnStmt : public Stmt
{
protected:
  Expr *expr;

public:
  ReturnStmt(yyltype loc, Expr *expr);
  const char *ReturnNodeName() { return "ReturnStmt"; }
  void ShowChildNodes(int indentLevel);

  void Check(checkT c);

  void Emit();
};

class PrintStmt : public Stmt
{
protected:
  List<Expr *> *args;

public:
  PrintStmt(List<Expr *> *arguments);
  const char *ReturnNodeName() { return "PrintStmt"; }
  void ShowChildNodes(int indentLevel);

  void Check(checkT c);

  void Emit();
};

#endif
