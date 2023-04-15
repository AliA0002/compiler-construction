/* File: scope.cc
 * ---------------
 * Build the symbol table.
 */

#include <iostream>
#include "string.h"

#include "scope.h"
#include "ast.h"
#include "ast_decl.h"

STable *symbol_table;

STable::STable()
{
    PrintDebug("sttrace", "STable constructor.\n");
    scopes = new std::vector<Scope *>;
    scopes->clear();
    scopes->push_back(new Scope());
    act_scopes = new std::vector<int>;
    act_scopes->clear();
    act_scopes->push_back(0);
    cur_scopes = 0;
    scope_count = 0;
    id_count = 0;
}

void STable::ResetSymbolTable()
{
    PrintDebug("sttrace", "======== Reset STable ========\n");
    act_scopes->clear();
    act_scopes->push_back(0);
    cur_scopes = 0;
    scope_count = 0;
    id_count = 0;
}

void STable::GenerateScope()
{
    PrintDebug("sttrace", "Build new scope %d.\n", scope_count + 1);
    scope_count++;
    scopes->push_back(new Scope());
    act_scopes->push_back(scope_count);
    cur_scopes = scope_count;
}

void STable::GenerateScope(const char *key)
{
    PrintDebug("sttrace", "Build new scope %d.\n", scope_count + 1);
    scope_count++;
    scopes->push_back(new Scope());
    scopes->at(scope_count)->SetOwner(key);
    act_scopes->push_back(scope_count);
    cur_scopes = scope_count;
}

void STable::EnterScope()
{
    PrintDebug("sttrace", "Enter scope %d.\n", scope_count + 1);
    scope_count++;
    act_scopes->push_back(scope_count);
    cur_scopes = scope_count;
}

int STable::SearchScopeOwner(const char *key)
{
    int scope = -1;

    for (int i = 0; i < scopes->size(); i++)
    {
        if (scopes->at(i)->HasOwner())
        {
            if (!strcmp(key, scopes->at(i)->GetOwner()))
            {
                scope = i;
                ;
                break;
            }
        }
    }

    PrintDebug("sttrace", "From %s find scope %d.\n", key, scope);
    return scope;
}

Decl *STable::Lookup(Identifier *id)
{
    Decl *dec = NULL;
    const char *parent = NULL;
    const char *key = id->ReturnIdenName();
    PrintDebug("sttrace", "Lookup %s from active scopes %d.\n", key, cur_scopes);

    for (int i = act_scopes->size(); i > 0; --i)
    {

        int scope = act_scopes->at(i - 1);
        Scope *s = scopes->at(scope);

        if (s->ContainsHT())
        {
            dec = s->GetHT()->Lookup(key);
        }
        if (dec != NULL)
            break;

        while (s->HasParent())
        {
            parent = s->GetParent();
            scope = SearchScopeOwner(parent);
            if (scope != -1)
            {
                if (scope == cur_scopes)
                {
                    break;
                }
                else
                {
                    s = scopes->at(scope);
                    if (s->ContainsHT())
                    {
                        dec = s->GetHT()->Lookup(key);
                    }
                    if (dec != NULL)
                        break;
                }
            }
            else
            {
                break;
            }
        }
        if (dec != NULL)
            break;
    }

    return dec;
}

Decl *STable::FindParent(Identifier *id)
{
    Decl *d = NULL;
    const char *parent = NULL;
    const char *key = id->ReturnIdenName();
    Scope *s = scopes->at(cur_scopes);
    PrintDebug("sttrace", "Lookup %s in parent of %d.\n", key, cur_scopes);

    while (s->HasParent())
    {
        parent = s->GetParent();
        int scope = SearchScopeOwner(parent);

        if (scope != -1)
        {
            if (scope == cur_scopes)
            {
                break;
            }
            else
            {
                s = scopes->at(scope);
                if (s->ContainsHT())
                {
                    d = s->GetHT()->Lookup(key);
                }
                if (d != NULL)
                    break;
            }
        }
    }

    return d;
}

Decl *STable::FindInterface(Identifier *id)
{
    Decl *d = NULL;
    const char *key = id->ReturnIdenName();
    int scope;
    Scope *s = scopes->at(cur_scopes);
    PrintDebug("sttrace", "Lookup %s in interface of %d.\n", key, cur_scopes);

    if (s->HasInterface())
    {

        std::list<const char *> *itfc = s->GetInterface();

        for (std::list<const char *>::iterator it = itfc->begin();
             it != itfc->end(); it++)
        {
            scope = SearchScopeOwner(*it);

            if (scope != -1)
            {
                Scope *sc = scopes->at(scope);
                if (sc->ContainsHT())
                {
                    d = sc->GetHT()->Lookup(key);
                }
                if (d != NULL)
                    break;
            }
        }
    }
    return d;
}

Decl *STable::LookForField(Identifier *base, Identifier *field)
{
    Decl *d = NULL;
    const char *b = base->ReturnIdenName();
    const char *f = field->ReturnIdenName();
    PrintDebug("sttrace", "Lookup %s from field %s\n", f, b);

    int scope = SearchScopeOwner(b);
    if (scope == -1)
        return NULL;

    Scope *s = scopes->at(scope);
    if (s->ContainsHT())
    {
        d = s->GetHT()->Lookup(f);
    }
    if (d != NULL)
        return d;

    while (s->HasParent())
    {
        b = s->GetParent();
        scope = SearchScopeOwner(b);
        if (scope != -1)
        {
            if (scope == cur_scopes)
            {

                break;
            }
            else
            {
                s = scopes->at(scope);
                if (s->ContainsHT())
                {
                    d = s->GetHT()->Lookup(f);
                }
                if (d != NULL)
                    break;
            }
        }
        else
        {
            break;
        }
    }
    return d;
}

Decl *STable::FindThis()
{
    PrintDebug("sttrace", "Lookup This\n");
    Decl *d = NULL;
    for (int i = act_scopes->size(); i > 0; --i)
    {

        int scope = act_scopes->at(i - 1);
        Scope *s = scopes->at(scope);

        if (s->HasOwner())
        {
            PrintDebug("sttrace", "Lookup This as %s\n", s->GetOwner());
            Scope *s0 = scopes->at(0);
            if (s0->ContainsHT())
            {
                d = s0->GetHT()->Lookup(s->GetOwner());
            }
        }
        if (d)
            break;
    }
    return d;
}

int STable::InsertSymbol(Decl *decl)
{
    const char *key = decl->GetId()->ReturnIdenName();
    Scope *s = scopes->at(cur_scopes);
    PrintDebug("sttrace", "Insert %s to scope %d\n", key, cur_scopes);

    if (!s->ContainsHT())
    {
        s->BuildHT();
    }

    s->GetHT()->Enter(key, decl);
    return id_count++;
}

bool STable::LocalLookup(Identifier *id)
{
    Decl *d = NULL;
    const char *key = id->ReturnIdenName();
    Scope *s = scopes->at(cur_scopes);
    PrintDebug("sttrace", "LocalLookup %s from scope %d\n", key, cur_scopes);

    if (s->ContainsHT())
    {
        d = s->GetHT()->Lookup(key);
    }

    return (d == NULL) ? false : true;
}

void STable::ExitScope()
{
    PrintDebug("sttrace", "Exit scope %d\n", cur_scopes);
    act_scopes->pop_back();
    cur_scopes = act_scopes->back();
}

void STable::SetScopeParent(const char *key)
{
    scopes->at(cur_scopes)->SetParent(key);
}

void STable::SetInterface(const char *key)
{
    scopes->at(cur_scopes)->AddInterface(key);
}

void STable::Print()
{
    std::cout << std::endl
              << "======== Symbol Table ========" << std::endl;

    for (int i = 0; i < scopes->size(); i++)
    {
        Scope *s = scopes->at(i);

        if (!s->ContainsHT() && !s->HasOwner() && !s->HasParent() && !s->HasInterface())
            continue;

        std::cout << "|- Scope " << i << ":";
        if (s->HasOwner())
            std::cout << " (owner: " << s->GetOwner() << ")";
        if (s->HasParent())
            std::cout << " (parent: " << s->GetParent() << ")";
        if (s->HasInterface())
        {
            std::cout << " (interface: ";
            std::list<const char *> *interface = s->GetInterface();
            for (std::list<const char *>::iterator it = interface->begin();
                 it != interface->end(); it++)
            {
                std::cout << *it << " ";
            }
            std::cout << ")";
        }

        std::cout << std::endl;

        if (s->ContainsHT())
        {
            Iterator<Decl *> iter = s->GetHT()->GetIterator();
            Decl *decl;
            while ((decl = iter.GetNextValue()) != NULL)
            {
                std::cout << "|  + " << decl << std::endl;
            }
        }
    }

    std::cout << "======== Symbol Table ========" << std::endl;
}
