/* File: ast_decl.h
 * ----------------
 * In our parse tree, Decl nodes are used to represent and
 * manage declarations. There are 4 subclasses of the base class,
 * specialized for declarations of variables, functions, classes,
 * and interfaces.
 *
 * pp3: You will need to extend the Decl classes to implement
 * semantic processing including detection of declaration conflicts
 * and managing scoping issues.
 */

#ifndef _H_ast_decl
#define _H_ast_decl

#include "ast.h"
#include "ast_type.h"
#include "list.h"
#include "errors.h"

class Identifier;
class Stmt;
class Scope;
class Type;
class NamedType;

class Decl : public Node
{
protected:
  Identifier *id;
  Scope *scp;

public:
  Decl(Identifier *name);
  friend std::ostream &operator<<(std::ostream &out, Decl *d) { return out << d->id; }
  const char *ReturnName()
  {
    return id->ReturnName();
  }
  Scope *GetScp()
  {
    return scp;
  }
  virtual bool IsEquivalentTo(Decl *d)
  {
    return true;
  }
  virtual void ConstructScp(Scope *par);
  virtual void Check() = 0;
};

class VarDecl : public Decl
{
protected:
  Type *type;

public:
  VarDecl(Identifier *name, Type *type);
  bool IsEquivalentTo(Decl *d);
  Type *ReturnType()
  {
    return type;
  }
  void Check();
};

class ClassDecl : public Decl
{
protected:
  List<Decl *> *members;
  NamedType *extends;
  List<NamedType *> *implements;

private:
  void CheckExtends();
  void CheckImplements();
  void CheckExtendsMems(NamedType *ex);
  void CheckImplementsMems(NamedType *imp);
  void CheckImplementsIntfs(NamedType *intf);
  void CheckScp(Scope *s);

public:
  ClassDecl(Identifier *name, NamedType *extends,
            List<NamedType *> *implements, List<Decl *> *members);
  Type *ReturnType()
  {
    return new NamedType(id);
  }
  List<Decl *> *GetMembers()
  {
    return members;
  }
  NamedType *GetExtends()
  {
    return extends;
  }
  List<NamedType *> *GetImplements()
  {
    return implements;
  }
  void ConstructScp(Scope *par);
  void Check();
};

class InterfaceDecl : public Decl
{
protected:
  List<Decl *> *members;

public:
  InterfaceDecl(Identifier *name, List<Decl *> *members);
  Type *ReturnType()
  {
    return new NamedType(id);
  }
  List<Decl *> *GetMembers()
  {
    return members;
  }
  void ConstructScp(Scope *par);
  void Check();
};

class FnDecl : public Decl
{
protected:
  List<VarDecl *> *formals;
  Type *returnType;
  Stmt *body;

public:
  FnDecl(Identifier *name, Type *returnType, List<VarDecl *> *formals);
  void SetFunctionBody(Stmt *b);
  Type *ReturnType()
  {
    return returnType;
  }
  bool IsEquivalentTo(Decl *d);
  List<VarDecl *> *GetFormals()
  {
    return formals;
  }
  Type *GetReturn()
  {
    return returnType;
  }
  void ConstructScp(Scope *par);
  void Check();
};

#endif
