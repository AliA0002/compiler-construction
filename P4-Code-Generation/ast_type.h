/* File: ast_type.h
 * ----------------
 * In our parse tree, Type nodes are used to represent and
 * store type information. The base Type class is used
 * for built-in types, the NamedType for classes and interfaces,
 * and the ArrayType for arrays of other types.
 */

#ifndef _H_ast_type
#define _H_ast_type

#include "ast.h"
#include "list.h"
#include <iostream>

class Type : public Node
{
protected:
  char *typeName;

public:
  static Type *intType, *doubleType, *boolType, *voidType,
      *nullType, *stringType, *errorType;

  Type(yyltype loc) : Node(loc) { type_of_expr = NULL; }
  Type(const char *str);

  const char *ReturnNodeName() { return "Type"; }
  void ShowChildNodes(int indentLevel);

  virtual void StreamPrint(std::ostream &out) { out << typeName; }
  friend std::ostream &operator<<(std::ostream &out, Type *t)
  {
    t->StreamPrint(out);
    return out;
  }
  virtual bool Equivalent(Type *other) { return this == other; }
  virtual bool IsCompatibleWith(Type *other) { return this == other; }

  virtual bool IsBasicType()
  {
    return !this->Type_NamedType() && !this->Type_ArrayType();
  }
  virtual bool Type_NamedType() { return false; }
  virtual bool Type_ArrayType() { return false; }
  void Check(checkT c);
  virtual void Check(checkT c, reasonT r) { Check(c); }
  virtual void SetSelfType() { type_of_expr = this; }

  virtual int ReturnTypeSize() { return 4; }
};

class NamedType : public Type
{
protected:
  Identifier *id;
  void CheckDecl(reasonT r);

public:
  NamedType(Identifier *i);

  const char *ReturnNodeName() { return "NamedType"; }
  void ShowChildNodes(int indentLevel);

  void StreamPrint(std::ostream &out) { out << id; }

  Identifier *GetId() { return id; }

  bool Equivalent(Type *other);
  bool IsCompatibleWith(Type *other);

  bool Type_NamedType() { return true; }
  void Check(checkT c, reasonT r);
  void Check(checkT c) { Check(c, LookingForType); }
};

class ArrayType : public Type
{
protected:
  Type *elemType;
  void CheckDecl();

public:
  ArrayType(yyltype loc, Type *elemType);

  const char *ReturnNodeName() { return "ArrayType"; }
  void ShowChildNodes(int indentLevel);

  void StreamPrint(std::ostream &out) { out << elemType << "[]"; }

  Type *GetElemType() { return elemType; }
  bool Equivalent(Type *other);
  bool IsCompatibleWith(Type *other);

  bool Type_ArrayType() { return true; }
  void Check(checkT c);
};

#endif
