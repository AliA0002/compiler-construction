/* File: ast_stmt.h
 * ----------------
 * The Stmt class and its subclasses are used to represent
 * statements in the parse tree.  For each statment in the
 * language (for, if, return, etc.) there is a corresponding
 * node class for that construct.
 *
 * pp3: You will need to extend the Stmt classes to implement
 * semantic analysis for rules pertaining to statements.
 */

#ifndef _H_ast_stmt
#define _H_ast_stmt

#include "list.h"
#include "ast.h"
#include "hashtable.h"

class Decl;
class VarDecl;
class Expr;
class LoopStmt;
class ClassDecl;
class FnDecl;

class Scope
{
private:
  Scope *parent;

public:
  Hashtable<Decl *> *hashTable;
  ClassDecl *classDecl;
  FnDecl *fnDecl;
  LoopStmt *loopStmt;

  // ctor
  Scope() : hashTable(new Hashtable<Decl *>), classDecl(NULL), fnDecl(NULL) {}

  // Make parents
  void SetParent(Scope *par)
  {
    parent = par;
  }

  void SetClass(ClassDecl *cl)
  {
    classDecl = cl;
  }

  void SetFunction(FnDecl *fun)
  {
    fnDecl = fun;
  }

  void SetLoop(LoopStmt *loop)
  {
    loopStmt = loop;
  }

  int NewDecl(Decl *dec);

  // Return parents
  Scope *GetParent()
  {
    return parent;
  }

  ClassDecl *GetClass()
  {
    return classDecl;
  }

  FnDecl *GetFunction()
  {
    return fnDecl;
  }

  LoopStmt *GetLoop()
  {
    return loopStmt;
  }
};

class Program : public Node
{
protected:
  List<Decl *> *decls;

public:
  static Scope *globalScp;
  Program(List<Decl *> *declList);
  void Check();
  void ConstructScp();
};

class Stmt : public Node
{
protected:
  Scope *scp;

public:
  Stmt() : Node(), scp(new Scope) {}
  Stmt(yyltype loc) : Node(loc), scp(new Scope) {}
  virtual void ConstructScp(Scope *par);
  virtual void Check() = 0;
};

class StmtBlock : public Stmt
{
protected:
  List<VarDecl *> *decls;
  List<Stmt *> *stmts;

public:
  StmtBlock(List<VarDecl *> *variableDeclarations, List<Stmt *> *statements);
  void ConstructScp(Scope *par);
  void Check();
};

class ConditionalStmt : public Stmt
{
protected:
  Expr *test;
  Stmt *body;

public:
  ConditionalStmt(Expr *testExpr, Stmt *body);
  virtual void ConstructScp(Scope *par);
  virtual void Check();
};

class LoopStmt : public ConditionalStmt
{
public:
  LoopStmt(Expr *testExpr, Stmt *body)
      : ConditionalStmt(testExpr, body) {}
  virtual void ConstructScp(Scope *par);
};

class ForStmt : public LoopStmt
{
protected:
  Expr *init, *step;

public:
  ForStmt(Expr *init, Expr *test, Expr *step, Stmt *body);
};

class WhileStmt : public LoopStmt
{
public:
  WhileStmt(Expr *test, Stmt *body) : LoopStmt(test, body) {}
};

class IfStmt : public ConditionalStmt
{
protected:
  Stmt *elseBody;

public:
  IfStmt(Expr *test, Stmt *thenBody, Stmt *elseBody);
  void ConstructScp(Scope *par);
  void Check();
};

class BreakStmt : public Stmt
{
public:
  BreakStmt(yyltype loc) : Stmt(loc) {}
  void Check();
};

class ReturnStmt : public Stmt
{
protected:
  Expr *expr;

public:
  ReturnStmt(yyltype loc, Expr *expr);
  void ConstructScp(Scope *par);
  void Check();
};

class PrintStmt : public Stmt
{
protected:
  List<Expr *> *args;

public:
  PrintStmt(List<Expr *> *arguments);
  void ConstructScp(Scope *par);
  void Check();
};

#endif
