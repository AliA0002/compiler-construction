/* File: ast_expr.h
 * ----------------
 * The Expr class and its subclasses are used to represent
 * expressions in the parse tree.  For each expression in the
 * language (add, call, New, etc.) there is a corresponding
 * node class for that construct.
 */

#ifndef _H_ast_expr
#define _H_ast_expr

#include "ast.h"
#include "ast_stmt.h"
#include "list.h"
#include "ast_type.h"

class NamedType;
class Type;

class Expr : public Stmt
{
public:
  Expr(yyltype loc) : Stmt(loc) { type_of_expr = NULL; }
  Expr() : Stmt() { type_of_expr = NULL; }

  virtual Location *ReturnEmitLocD() { return GetEmitLoc(); }
  virtual bool AccessibleArray() { return false; }
  virtual bool ExprIsEmpty() { return false; }
};

class EmptyExpr : public Expr
{
public:
  const char *ReturnNodeName() { return "Empty"; }
  void ShowChildNodes(int indentLevel);
  void Check(checkT c);

  bool ExprIsEmpty() { return true; }
};

class IntConstant : public Expr
{
protected:
  int value;

public:
  IntConstant(yyltype loc, int val);
  const char *ReturnNodeName() { return "IntConstant"; }
  void ShowChildNodes(int indentLevel);

  void Check(checkT c);

  void Emit();
};

class DoubleConstant : public Expr
{
protected:
  double value;

public:
  DoubleConstant(yyltype loc, double val);
  const char *ReturnNodeName() { return "DoubleConstant"; }
  void ShowChildNodes(int indentLevel);

  void Check(checkT c);

  void Emit();
};

class BoolConstant : public Expr
{
protected:
  bool value;

public:
  BoolConstant(yyltype loc, bool val);
  const char *ReturnNodeName() { return "BoolConstant"; }
  void ShowChildNodes(int indentLevel);

  void Check(checkT c);

  void Emit();
};

class StringConstant : public Expr
{
protected:
  char *value;

public:
  StringConstant(yyltype loc, const char *val);
  const char *ReturnNodeName() { return "StringConstant"; }
  void ShowChildNodes(int indentLevel);

  void Check(checkT c);

  void Emit();
};

class NullConstant : public Expr
{
public:
  NullConstant(yyltype loc) : Expr(loc) {}
  const char *ReturnNodeName() { return "NullConstant"; }
  void ShowChildNodes(int indentLevel);
  void Check(checkT c);

  void Emit();
};

class Operator : public Node
{
protected:
  char tokenString[4];

public:
  Operator(yyltype loc, const char *tok);
  const char *ReturnNodeName() { return "Operator"; }
  void ShowChildNodes(int indentLevel);
  friend std::ostream &operator<<(std::ostream &out, Operator *o)
  {
    return out << o->tokenString;
  }

  const char *GetOpStr() { return tokenString; }
};

class CompoundExpr : public Expr
{
protected:
  Operator *op;
  Expr *left, *right; // left will be NULL if unary

public:
  CompoundExpr(Expr *lhs, Operator *op, Expr *rhs); // for binary
  CompoundExpr(Operator *op, Expr *rhs);            // for unary
  void ShowChildNodes(int indentLevel);
};

class ArithmeticExpr : public CompoundExpr
{
protected:
  void ConfirmType();

public:
  ArithmeticExpr(Expr *lhs, Operator *op, Expr *rhs) : CompoundExpr(lhs, op, rhs) {}
  ArithmeticExpr(Operator *op, Expr *rhs) : CompoundExpr(op, rhs) {}
  const char *ReturnNodeName() { return "ArithmeticExpr"; }
  void Check(checkT c);

  void Emit();
};

class RelationalExpr : public CompoundExpr
{
protected:
  void ConfirmType();

public:
  RelationalExpr(Expr *lhs, Operator *op, Expr *rhs) : CompoundExpr(lhs, op, rhs) {}
  const char *ReturnNodeName() { return "RelationalExpr"; }
  void Check(checkT c);

  void Emit();
};

class EqualityExpr : public CompoundExpr
{
protected:
  void ConfirmType();

public:
  EqualityExpr(Expr *lhs, Operator *op, Expr *rhs) : CompoundExpr(lhs, op, rhs) {}
  const char *ReturnNodeName() { return "EqualityExpr"; }
  void Check(checkT c);

  void Emit();
};

class LogicalExpr : public CompoundExpr
{
protected:
  void ConfirmType();

public:
  LogicalExpr(Expr *lhs, Operator *op, Expr *rhs) : CompoundExpr(lhs, op, rhs) {}
  LogicalExpr(Operator *op, Expr *rhs) : CompoundExpr(op, rhs) {}
  const char *ReturnNodeName() { return "LogicalExpr"; }
  void Check(checkT c);

  void Emit();
};

class AssignExpr : public CompoundExpr
{
protected:
  void ConfirmType();

public:
  AssignExpr(Expr *lhs, Operator *op, Expr *rhs) : CompoundExpr(lhs, op, rhs) {}
  const char *ReturnNodeName() { return "AssignExpr"; }
  void Check(checkT c);

  void Emit();
};

class LValue : public Expr
{
public:
  LValue(yyltype loc) : Expr(loc) {}
};

class This : public Expr
{
protected:
  void ConfirmType();

public:
  This(yyltype loc) : Expr(loc) {}
  const char *ReturnNodeName() { return "This"; }
  void ShowChildNodes(int indentLevel);
  void Check(checkT c);

  void Emit();
};

class ArrayAccess : public LValue
{
protected:
  Expr *base, *subscript;
  void ConfirmType();

public:
  ArrayAccess(yyltype loc, Expr *base, Expr *subscript);
  const char *ReturnNodeName() { return "ArrayAccess"; }
  void ShowChildNodes(int indentLevel);

  void Check(checkT c);

  void Emit();
  bool AccessibleArray() { return true; }
  Location *ReturnEmitLocD();
};

class FieldAccess : public LValue
{
protected:
  Expr *base; // will be NULL if no explicit base
  Identifier *field;
  void CheckDecl();
  void ConfirmType();

public:
  FieldAccess(Expr *base, Identifier *field); // ok to pass NULL base
  const char *ReturnNodeName() { return "FieldAccess"; }
  void ShowChildNodes(int indentLevel);

  void Check(checkT c);

  // code generation
  void Emit();
  Location *ReturnEmitLocD();
};

class Call : public Expr
{
protected:
  Expr *base; // will be NULL if no explicit base
  Identifier *field;
  List<Expr *> *actuals;
  void CheckDecl();
  void ConfirmType();
  void CheckFuncArgs();

public:
  Call(yyltype loc, Expr *base, Identifier *field, List<Expr *> *args);
  const char *ReturnNodeName() { return "Call"; }
  void ShowChildNodes(int indentLevel);

  void Check(checkT c);

  void Emit();
};

class NewExpr : public Expr
{
protected:
  NamedType *cType;
  void CheckDecl();
  void ConfirmType();

public:
  NewExpr(yyltype loc, NamedType *clsType);
  const char *ReturnNodeName() { return "NewExpr"; }
  void ShowChildNodes(int indentLevel);

  void Check(checkT c);

  void Emit();
};

class NewArrayExpr : public Expr
{
protected:
  Expr *size;
  Type *elemType;
  void ConfirmType();

public:
  NewArrayExpr(yyltype loc, Expr *sizeExpr, Type *elemType);
  const char *ReturnNodeName() { return "NewArrayExpr"; }
  void ShowChildNodes(int indentLevel);

  void Check(checkT c);

  void Emit();
};

class ReadIntegerExpr : public Expr
{
public:
  ReadIntegerExpr(yyltype loc) : Expr(loc) {}
  const char *ReturnNodeName() { return "ReadIntegerExpr"; }
  void Check(checkT c);

  void Emit();
};

class ReadLineExpr : public Expr
{
public:
  ReadLineExpr(yyltype loc) : Expr(loc) {}
  const char *ReturnNodeName() { return "ReadLineExpr"; }
  void Check(checkT c);

  void Emit();
};

#endif
