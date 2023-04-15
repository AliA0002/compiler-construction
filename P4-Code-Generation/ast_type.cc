/* File: ast_type.cc
 * -----------------
 * Implementation of type node classes.
 */
#include "ast_type.h"
#include "ast_decl.h"
#include <string.h>
#include "errors.h"

/* Class constants
 * ---------------
 * These are public constants for the built-in base types (int, double, etc.)
 * They can be accessed with the syntax Type::intType. This allows you to
 * directly access them and share the built-in types where needed rather that
 * creates lots of copies.
 */

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
    type_of_expr = NULL;
}

void Type::ShowChildNodes(int indentLevel)
{
    printf("%s", typeName);
    if (type_of_expr)
        std::cout << " <" << type_of_expr << ">";
    if (emit_loc)
        emit_loc->Print();
}

void Type::Check(checkT c)
{
    if (c == enum_DeclCheck)
    {
        Type::intType->SetSelfType();
        Type::doubleType->SetSelfType();
        Type::voidType->SetSelfType();
        Type::boolType->SetSelfType();
        Type::nullType->SetSelfType();
        Type::stringType->SetSelfType();
        Type::errorType->SetSelfType();
        type_of_expr = this;
    }
}

NamedType::NamedType(Identifier *i) : Type(*i->GetLocation())
{
    Assert(i != NULL);
    (id = i)->SetParent(this);
}

void NamedType::ShowChildNodes(int indentLevel)
{
    if (type_of_expr)
        std::cout << " <" << type_of_expr << ">";
    if (emit_loc)
        emit_loc->Print();
    id->Print(indentLevel + 1);
}

void NamedType::CheckDecl(reasonT r)
{
    Decl *d = symbol_table->Lookup(this->id);
    if (d == NULL || (!d->IsClassDecl() && !d->IsInterfaceDecl()))
    {
        ReportError::IdentifierNotDeclared(this->id, r);
    }
    else if (r == LookingForClass && !d->IsClassDecl())
    {
        ReportError::IdentifierNotDeclared(this->id, r);
    }
    else if (r == LookingForInterface && !d->IsInterfaceDecl())
    {
        ReportError::IdentifierNotDeclared(this->id, r);
    }
    else
    {
        this->id->SetCache(d);
        type_of_expr = this;
    }
}

void NamedType::Check(checkT c, reasonT r)
{
    if (c == enum_DeclCheck)
    {
        this->CheckDecl(r);
    }
    else
    {
        id->Check(c);
    }
}

bool NamedType::Equivalent(Type *other)
{
    Assert(this->ReturnType() && other->ReturnType());

    if (!other->Type_NamedType())
    {
        return false;
    }
    NamedType *nt = dynamic_cast<NamedType *>(other);
    return (id->Equivalent(nt->GetId()));
}

bool NamedType::IsCompatibleWith(Type *other)
{
    Assert(this->ReturnType() && other->ReturnType());

    if (other == nullType)
    {
        return true;
    }
    else if (!other->Type_NamedType())
    {
        return false;
    }
    else if (this->Equivalent(other))
    {
        return true;
    }
    else
    {
        NamedType *nt = dynamic_cast<NamedType *>(other);
        Decl *decl1 = id->ReturnCache();
        Decl *decl2 = nt->GetId()->ReturnCache();
        Assert(decl1 && decl2);
        if (!decl2->IsClassDecl())
        {
            return false;
        }
        ClassDecl *cdecl2 = dynamic_cast<ClassDecl *>(decl2);
        // subclass can compatible with its parent class and interface.
        return cdecl2->IsChildOf(decl1);
    }
}

ArrayType::ArrayType(yyltype loc, Type *et) : Type(loc)
{
    Assert(et != NULL);
    (elemType = et)->SetParent(this);
}
void ArrayType::ShowChildNodes(int indentLevel)
{
    if (type_of_expr)
        std::cout << " <" << type_of_expr << ">";
    if (emit_loc)
        emit_loc->Print();
    elemType->Print(indentLevel + 1);
}

void ArrayType::CheckDecl()
{
    elemType->Check(enum_DeclCheck);
    if (elemType->ReturnType())
    {
        type_of_expr = this;
    }
}

void ArrayType::Check(checkT c)
{
    if (c == enum_DeclCheck)
    {
        this->CheckDecl();
    }
    else
    {
        elemType->Check(c);
    }
}

bool ArrayType::Equivalent(Type *other)
{
    Assert(this->ReturnType() && other->ReturnType());

    if (!other->Type_ArrayType())
    {
        return false;
    }
    ArrayType *nt = dynamic_cast<ArrayType *>(other);
    return (elemType->Equivalent(nt->GetElemType()));
}

bool ArrayType::IsCompatibleWith(Type *other)
{
    Assert(this->ReturnType() && other->ReturnType());

    if (other == nullType)
    {
        return elemType->IsCompatibleWith(other);
    }
    else
    {
        return this->Equivalent(other);
    }
}
