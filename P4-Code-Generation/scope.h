/* File: scope.h
 * -------------
 * The Scope class will be used to manage scopes, sort of
 * table used to map identifier names to Declaration objects.
 */

#ifndef _H_scope
#define _H_scope

#include <list>
#include <vector>
#include "hashtable.h"

class Decl;
class Identifier;

class Scope
{
protected:
  Hashtable<Decl *> *ht;
  const char *parent;
  std::list<const char *> *interface;
  const char *owner;

public:
  Scope()
  {
    ht = NULL;
    parent = NULL;
    interface = new std::list<const char *>;
    interface->clear();
    owner = NULL;
  }

  bool ContainsHT() { return ht == NULL ? false : true; }
  void BuildHT() { ht = new Hashtable<Decl *>; }
  Hashtable<Decl *> *GetHT() { return ht; }

  bool HasParent() { return parent == NULL ? false : true; }
  void SetParent(const char *p) { parent = p; }
  const char *GetParent() { return parent; }

  bool HasInterface() { return !interface->empty(); }
  void AddInterface(const char *p) { interface->push_back(p); }
  std::list<const char *> *GetInterface() { return interface; }

  bool HasOwner() { return owner == NULL ? false : true; }
  void SetOwner(const char *o) { owner = o; }
  const char *GetOwner() { return owner; }
};

class STable
{
protected:
  std::vector<Scope *> *scopes;
  std::vector<int> *act_scopes;
  int cur_scopes;
  int scope_count;
  int id_count;

  int SearchScopeOwner(const char *owner);

public:
  STable();
  void GenerateScope();
  void GenerateScope(const char *key);
  void EnterScope();
  Decl *Lookup(Identifier *id);
  Decl *FindParent(Identifier *id);
  Decl *FindInterface(Identifier *id);
  Decl *LookForField(Identifier *base, Identifier *field);
  Decl *FindThis();
  int InsertSymbol(Decl *decl);
  bool LocalLookup(Identifier *id);
  void ExitScope();
  void SetScopeParent(const char *key);
  void SetInterface(const char *key);
  void ResetSymbolTable();
  void Print();
};

extern STable *symbol_table;

#endif
