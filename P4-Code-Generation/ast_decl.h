/* File: ast_decl.h
 * ----------------
 * In our parse tree, Decl nodes are used to represent and
 * manage declarations. There are 4 subclasses of the base class,
 * specialized for declarations of variables, functions, classes,
 * and interfaces.
 */

#ifndef _H_ast_decl
#define _H_ast_decl

#include "ast.h"
#include "list.h"
#include "ast_type.h"

class Type;
class NamedType;
class Identifier;
class Stmt;
class FnDecl;

class Decl : public Node
{
protected:
  Identifier *id;
  int idx;

public:
  Decl(Identifier *name);
  friend std::ostream &operator<<(std::ostream &out, Decl *d)
  {
    return out << d->id;
  }

  Identifier *GetId() { return id; }
  int GetIndex() { return idx; }
  virtual bool IsVarDecl() { return false; }
  virtual bool IsClassDecl() { return false; }
  virtual bool IsInterfaceDecl() { return false; }
  virtual bool FnIsDecl() { return false; }
  virtual void OffsetAssign() {}
  virtual void OffsetForMember(bool inClass, int offset) {}
  virtual void PrefixForMember() {}
};

class VarDecl : public Decl
{
protected:
  Type *type;
  bool is_global;
  int class_member_offset;
  void CheckDecl();
  bool IsGlobalVar() { return this->GetParent()->GetParent() == NULL; }
  bool MemberOfClass()
  {
    Decl *d = dynamic_cast<Decl *>(this->GetParent());
    return d ? d->IsClassDecl() : false;
  }

public:
  VarDecl(Identifier *name, Type *type);
  const char *ReturnNodeName() { return "VarDecl"; }
  void ShowChildNodes(int indentLevel);
  Type *ReturnType() { return type; }
  bool IsVarDecl() { return true; }
  void GenerateST();
  void Check(checkT c);
  void OffsetAssign();
  void OffsetForMember(bool inClass, int offset);
  void Emit();
  void SetEmitLoc(Location *l) { emit_loc = l; }
};

class ClassDecl : public Decl
{
protected:
  List<Decl *> *members;
  NamedType *extends;
  List<NamedType *> *implements;
  int inst_size;
  int vtable_size;
  List<VarDecl *> *var_members;
  List<FnDecl *> *fn_members;
  void CheckDecl();
  void CheckInherit();

public:
  ClassDecl(Identifier *name, NamedType *extends,
            List<NamedType *> *implements, List<Decl *> *members);
  const char *ReturnNodeName() { return "ClassDecl"; }
  void ShowChildNodes(int indentLevel);

  bool IsClassDecl() { return true; }
  void GenerateST();
  void Check(checkT c);
  bool IsChildOf(Decl *other);
  NamedType *GetExtends() { return extends; }

  void OffsetAssign();
  void Emit();
  int GetInstanceSize() { return inst_size; }
  int GetVTableSize() { return vtable_size; }
  void MembersForList(List<VarDecl *> *vars, List<FnDecl *> *fns);
  void PrefixForMember();
};

class InterfaceDecl : public Decl
{
protected:
  List<Decl *> *members;

public:
  InterfaceDecl(Identifier *name, List<Decl *> *members);
  const char *ReturnNodeName() { return "InterfaceDecl"; }
  void ShowChildNodes(int indentLevel);

  bool IsInterfaceDecl() { return true; }
  void GenerateST();
  void Check(checkT c);
  List<Decl *> *GetMembers() { return members; }

  void Emit();
};

class FnDecl : public Decl
{
protected:
  List<VarDecl *> *formals;
  Type *returnType;
  Stmt *body;
  int vtable_ofst;
  void CheckDecl();

public:
  FnDecl(Identifier *name, Type *returnType, List<VarDecl *> *formals);
  void SetBodyOfFunction(Stmt *b);
  const char *ReturnNodeName() { return "FnDecl"; }
  void ShowChildNodes(int indentLevel);

  Type *ReturnReturnType() { return returnType; }
  List<VarDecl *> *GetFormals() { return formals; }

  bool FnIsDecl() { return true; }
  bool Equivalent(Decl *fn);

  void GenerateST();
  void Check(checkT c);

  void PrefixForMember();
  void OffsetForMember(bool inClass, int offset);
  void Emit();
  int ReturnVTableOfst() { return vtable_ofst; }
  bool HasReturnValue() { return returnType != Type::voidType; }
  bool MemberOfClass()
  {
    Decl *d = dynamic_cast<Decl *>(this->GetParent());
    return d ? d->IsClassDecl() : false;
  }
};

#endif
