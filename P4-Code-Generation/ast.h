/* File: ast.h
 * -----------
 * This file defines the abstract base class Node and the concrete
 * Identifier and Error node subclasses that are used through the tree as
 * leaf nodes. A parse tree is a hierarchical collection of ast nodes (or,
 * more correctly, of instances of concrete subclassses such as VarDecl,
 * ForStmt, and AssignExpr).
 *
 * Location: Each node maintains its lexical location (line and columns in
 * file), that location can be NULL for those nodes that don't care/use
 * locations. The location is typcially set by the node constructor.  The
 * location is used to provide the context when reporting semantic errors.
 *
 * Parent: Each node has a pointer to its parent. For a Program node, the
 * parent is NULL, for all other nodes it is the pointer to the node one level
 * up in the parse tree.  The parent is not set in the constructor (during a
 * bottom-up parse we don't know the parent at the time of construction) but
 * instead we wait until assigning the children into the parent node and then
 * set up links in both directions. The parent link is typically not used
 * during parsing, but is more important in later phases.

 */

#ifndef _H_ast
#define _H_ast

#include <stdlib.h> // for NULL
#include "location.h"
#include <iostream>
#include "scope.h"
#include "errors.h"
#include "codegen.h"

extern CodeGenerator *CodeGen;

class Node
{
protected:
  yyltype *location;
  Node *parent;
  Type *type_of_expr;
  Location *emit_loc;

public:
  Node(yyltype loc);
  Node();

  yyltype *GetLocation() { return location; }
  void SetParent(Node *p) { parent = p; }
  Node *GetParent() { return parent; }

  virtual const char *ReturnNodeName() = 0;
  void Print(int indentLevel, const char *label = NULL);
  virtual void ShowChildNodes(int indentLevel) {}

  virtual void GenerateST() {}
  virtual void Check(checkT c) {}
  virtual Type *ReturnType() { return type_of_expr; }
  virtual bool IsLoop() { return false; }

  virtual void Emit() {}
  virtual Location *GetEmitLoc() { return emit_loc; }
};

class Identifier : public Node
{
protected:
  char *name;
  Decl *cache;
  void CheckDecl();

public:
  Identifier(yyltype loc, const char *name);
  const char *ReturnNodeName() { return "Identifier"; }
  void ShowChildNodes(int indentLevel);
  friend std::ostream &operator<<(std::ostream &out, Identifier *id)
  {
    return out << id->name;
  }

  char *ReturnIdenName() { return name; }
  void Check(checkT c);
  bool Equivalent(Identifier *other);
  void SetCache(Decl *d) { cache = d; }
  Decl *ReturnCache() { return cache; }

  void Emit();
  void SetPrefix(const char *prefix);
  Location *ReturnEmitLocD() { return GetEmitLoc(); }
};

// This node class is designed to represent a portion of the tree that
// encountered syntax errors during parsing. The partial completed tree
// is discarded along with the states being popped, and an instance of
// the Error class can stand in as the placeholder in the parse tree
// when your parser can continue after an error.
class Error : public Node
{
public:
  Error() : Node() {}
  const char *ReturnNodeName() { return "Error"; }
};

#endif
