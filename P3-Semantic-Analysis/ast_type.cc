/* File: ast_type.cc
 * -----------------
 * Implementation of type node classes.
 */
#include "ast_type.h"
#include "ast_decl.h"
#include <string.h>
#include "ast_stmt.h"

/* Class constants
 * ---------------
 * These are public constants for the built-in base types (int, double, etc.)
 * They can be accessed with the syntax Type::intType. This allows you to
 * directly access them and share the built-in types where needed rather that
 * creates lots of copies.
 */

// Type stuff
Type *Type::intType = new Type("int");
Type *Type::doubleType = new Type("double");
Type *Type::voidType = new Type("void");
Type *Type::boolType = new Type("bool");
Type *Type::nullType = new Type("null");
Type *Type::stringType = new Type("string");
Type *Type::errorType = new Type("error");

Type::Type(const char *n)
{
    Assert(n);
    typeName = strdup(n);
}

bool Type::IsEquivalentTo(Type *other)
{
    if (IsEqual(Type::errorType) || other->IsEqual(Type::errorType))
    {
        return true;
    }
    if (IsEqual(Type::nullType) && dynamic_cast<NamedType *>(other))
    {
        return true;
    }
    return IsEqual(other);
}

// NamedType stuff
NamedType::NamedType(Identifier *i) : Type(*i->GetLocation())
{
    Assert(i != NULL);
    (id = i)->SetParent(this);
}

bool NamedType::IsEqual(Type *t)
{
    NamedType *nt = dynamic_cast<NamedType *>(t);
    if (nt == NULL)
    {
        return false;
    }
    return *(nt->id) == *id;
}

bool NamedType::IsEquivalentTo(Type *t)
{
    if (IsEqual(t))
    {
        return true;
    }
    Decl *dec;
    NamedType *nt = this;
    while ((dec = Program::globalScp->hashTable->Lookup(nt->ReturnName())))
    {
        ClassDecl *cl = dynamic_cast<ClassDecl *>(dec);
        if (cl == NULL)
        {
            return false;
        }
        List<NamedType *> *imp_list = cl->GetImplements();
        for (int i = 0; i < imp_list->NumElements(); ++i)
        {
            if (imp_list->Nth(i)->IsEqual(t))
            {
                return true;
            }
        }
        nt = cl->GetExtends();
        if (nt == NULL)
        {
            return false;
        }
        if (nt->IsEqual(t))
        {
            return true;
        }
    }
    return false;
}

void NamedType::ReportNotDeclIdent(reasonT r)
{
    ReportError::IdentifierNotDeclared(id, r);
}

// ArrayType stuff
ArrayType::ArrayType(yyltype loc, Type *et) : Type(loc)
{
    Assert(et != NULL);
    (elemType = et)->SetParent(this);
}

ArrayType::ArrayType(Type *et) : Type()
{
    Assert(et != NULL);
    (elemType = et)->SetParent(this);
}

bool ArrayType::IsEqual(Type *t)
{
    ArrayType *at = dynamic_cast<ArrayType *>(t);
    if (at == NULL)
    {
        return false;
    }
    return elemType->IsEqual(at->elemType);
}

bool ArrayType::IsEquivalentTo(Type *t)
{
    ArrayType *at = dynamic_cast<ArrayType *>(t);
    if (at == NULL)
    {
        return false;
    }
    return elemType->IsEquivalentTo(at->elemType);
}

void ArrayType::ReportNotDeclIdent(reasonT r)
{
    elemType->ReportNotDeclIdent(r);
}