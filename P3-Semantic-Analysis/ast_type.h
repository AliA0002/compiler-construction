/* File: ast_type.h
 * ----------------
 * In our parse tree, Type nodes are used to represent and
 * store type information. The base Type class is used
 * for built-in types, the NamedType for classes and interfaces,
 * and the ArrayType for arrays of other types.
 *
 * pp3: You will need to extend the Type classes to implement
 * the type system and rules for type equivalency and compatibility.
 */

#ifndef _H_ast_type
#define _H_ast_type

#include "ast.h"
#include "list.h"
#include "errors.h"
#include <iostream>

class Type : public Node
{
protected:
  char *typeName;

public:
  static Type *intType, *doubleType, *boolType, *voidType,
      *nullType, *stringType, *errorType;

  Type(yyltype loc) : Node(loc) {}
  Type(const char *str);
  Type() : Node() {}

  virtual void PrintToStream(std::ostream &out) { out << typeName; }
  friend std::ostream &operator<<(std::ostream &out, Type *t)
  {
    t->PrintToStream(out);
    return out;
  }
  virtual bool IsEquivalentTo(Type *other);
  virtual bool IsEqual(Type *t)
  {
    return this == t;
  }
  virtual void ReportNotDeclIdent(reasonT r){};
  virtual const char *ReturnName()
  {
    return typeName;
  }
  virtual bool IsPrim()
  {
    return true;
  }
};

class NamedType : public Type
{
protected:
  Identifier *id;

public:
  NamedType(Identifier *i);

  void PrintToStream(std::ostream &out) { out << id; }
  bool IsEqual(Type *t);
  bool IsEquivalentTo(Type *t);
  void ReportNotDeclIdent(reasonT r);
  const char *ReturnName()
  {
    return id->ReturnName();
  }
  bool IsPrim()
  {
    return false;
  }
};

class ArrayType : public Type
{
protected:
  Type *elemType;

public:
  ArrayType(yyltype loc, Type *elemType);
  ArrayType(Type *elemType);

  void PrintToStream(std::ostream &out) { out << elemType << "[]"; }
  bool IsEqual(Type *t);
  bool IsEquivalentTo(Type *t);
  void ReportNotDeclIdent(reasonT r);
  Type *GetElem()
  {
    return elemType;
  }
  const char *ReturnName()
  {
    return elemType->ReturnName();
  }
  bool IsPrim()
  {
    return false;
  }
};

#endif
