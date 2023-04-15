/* File: ast_decl.cc
 * -----------------
 * Implementation of Decl node classes.
 */
#include "ast_decl.h"
#include "ast_type.h"
#include "ast_stmt.h"

// Decl stuff
Decl::Decl(Identifier *n) : Node(*n->GetLocation()), scp(new Scope)
{
    Assert(n != NULL);
    (id = n)->SetParent(this);
}

void Decl::ConstructScp(Scope *par)
{
    scp->SetParent(par);
}

// VarDecl stuff
VarDecl::VarDecl(Identifier *n, Type *t) : Decl(n)
{
    Assert(n != NULL && t != NULL);
    (type = t)->SetParent(this);
}

void VarDecl::Check()
{
    if (type->IsPrim())
    {
        return;
    }
    Scope *s = scp;
    while (s)
    { // not NULL
        Decl *dec = s->hashTable->Lookup(type->ReturnName());
        if (dec)
        { // not NULL
            if (dynamic_cast<ClassDecl *>(dec) == NULL &&
                dynamic_cast<InterfaceDecl *>(dec) == NULL)
            {
                type->ReportNotDeclIdent(LookingForType);
            }
            return;
        }
        s = s->GetParent();
    }
    type->ReportNotDeclIdent(LookingForType);
}

bool VarDecl::IsEquivalentTo(Decl *d)
{
    VarDecl *var = dynamic_cast<VarDecl *>(d);
    if (var == NULL)
    {
        return false;
    }
    return type->IsEquivalentTo(var->ReturnType());
}

// ClassDecl stuff
ClassDecl::ClassDecl(Identifier *n, NamedType *ex, List<NamedType *> *imp, List<Decl *> *m) : Decl(n)
{
    // extends can be NULL, impl & mem may be empty lists but cannot be NULL
    Assert(n != NULL && imp != NULL && m != NULL);
    extends = ex;
    if (extends)
        extends->SetParent(this);
    (implements = imp)->SetParentAll(this);
    (members = m)->SetParentAll(this);
}

void ClassDecl::ConstructScp(Scope *par)
{
    scp->SetParent(par);
    scp->SetClass(this);
    for (int i = 0; i < members->NumElements(); ++i)
    {
        scp->NewDecl(members->Nth(i));
    }
    for (int i = 0; i < members->NumElements(); ++i)
    {
        members->Nth(i)->ConstructScp(scp);
    }
}

void ClassDecl::Check()
{
    for (int i = 0; i < members->NumElements(); ++i)
    {
        members->Nth(i)->Check();
    }
    if (extends)
    { // not NULL
        CheckExtends();
    }
    CheckImplements();
    for (int i = 0; i < implements->NumElements(); ++i)
    {
        CheckImplementsMems(implements->Nth(i));
    }
    if (extends)
    { // not NULL
        CheckExtendsMems(extends);
    }
    for (int i = 0; i < implements->NumElements(); ++i)
    {
        CheckImplementsIntfs(implements->Nth(i));
    }
}

void ClassDecl::CheckExtends()
{
    Decl *dec = scp->GetParent()->hashTable->Lookup(extends->ReturnName());
    if (dynamic_cast<ClassDecl *>(dec) == NULL)
    {
        extends->ReportNotDeclIdent(LookingForClass);
    }
}

void ClassDecl::CheckImplements()
{
    for (int i = 0; i < implements->NumElements(); ++i)
    {
        Decl *dec = scp->GetParent()->hashTable->Lookup(implements->Nth(i)->ReturnName());
        if (dynamic_cast<InterfaceDecl *>(dec) == NULL)
        {
            implements->Nth(i)->ReportNotDeclIdent(LookingForInterface);
        }
    }
}

void ClassDecl::CheckExtendsMems(NamedType *ex)
{
    if (ex == NULL)
    {
        return;
    }
    Decl *dec = scp->GetParent()->hashTable->Lookup(ex->ReturnName());
    ClassDecl *exDecl = dynamic_cast<ClassDecl *>(dec);
    if (exDecl == NULL)
    {
        return;
    }
    CheckExtendsMems(exDecl->GetExtends());
    CheckScp(exDecl->GetScp());
}

void ClassDecl::CheckImplementsMems(NamedType *imp)
{
    Decl *dec = scp->GetParent()->hashTable->Lookup(imp->ReturnName());
    InterfaceDecl *intfDecl = dynamic_cast<InterfaceDecl *>(dec);
    if (intfDecl == NULL)
    {
        return;
    }
    CheckScp(intfDecl->GetScp());
}

void ClassDecl::CheckImplementsIntfs(NamedType *intf)
{
    Decl *dec = scp->GetParent()->hashTable->Lookup(intf->ReturnName());
    InterfaceDecl *intfDecl = dynamic_cast<InterfaceDecl *>(dec);
    if (intfDecl == NULL)
    {
        return;
    }
    List<Decl *> *mems = intfDecl->GetMembers();
    for (int i = 0; i < mems->NumElements(); ++i)
    {
        Decl *proDecl = mems->Nth(i);
        ClassDecl *classDecl = this;
        Decl *look_up;
        while (classDecl)
        { // not NULL
            look_up = classDecl->GetScp()->hashTable->Lookup(proDecl->ReturnName());
            if (look_up)
            { // not NULL
                break;
            }
            if (classDecl->GetExtends() == NULL)
            {
                classDecl = NULL;
            }
            else
            {
                NamedType *ext = classDecl->GetExtends();
                Decl *dec = Program::globalScp->hashTable->Lookup(ext->ReturnName());
                classDecl = dynamic_cast<ClassDecl *>(dec);
            }
        }
        if (look_up == NULL)
        {
            ReportError::InterfaceNotImplemented(this, intf);
            return;
        }
    }
}

void ClassDecl::CheckScp(Scope *s)
{
    Iterator<Decl *> it = scp->hashTable->GetIterator();
    Decl *dec;
    while ((dec = it.GetNextValue()))
    { // not NULL
        Assert(dynamic_cast<VarDecl *>(dec) || dynamic_cast<FnDecl *>(dec));
        Decl *look_up = s->hashTable->Lookup(dec->ReturnName());
        if (look_up)
        { // not NULL
            if (dynamic_cast<VarDecl *>(look_up))
            { // not NULL
                ReportError::DeclConflict(dec, look_up);
            }
            if (dynamic_cast<FnDecl *>(look_up) && !dec->IsEquivalentTo(look_up))
            {
                ReportError::OverrideMismatch(dec);
            }
        }
    }
}

// InterfaceDecl stuff
InterfaceDecl::InterfaceDecl(Identifier *n, List<Decl *> *m) : Decl(n)
{
    Assert(n != NULL && m != NULL);
    (members = m)->SetParentAll(this);
}

void InterfaceDecl::ConstructScp(Scope *par)
{
    scp->SetParent(par);

    for (int i = 0; i < members->NumElements(); ++i)
    {
        scp->NewDecl(members->Nth(i));
    }
    for (int i = 0; i < members->NumElements(); ++i)
    {
        members->Nth(i)->ConstructScp(scp);
    }
}

void InterfaceDecl::Check()
{
    for (int i = 0; i < members->NumElements(); ++i)
    {
        members->Nth(i)->Check();
    }
}

// FnDecl stuff
FnDecl::FnDecl(Identifier *n, Type *r, List<VarDecl *> *d) : Decl(n)
{
    Assert(n != NULL && r != NULL && d != NULL);
    (returnType = r)->SetParent(this);
    (formals = d)->SetParentAll(this);
    body = NULL;
}

void FnDecl::SetFunctionBody(Stmt *b)
{
    (body = b)->SetParent(this);
}

void FnDecl::ConstructScp(Scope *par)
{
    scp->SetParent(par);
    scp->SetFunction(this);
    for (int i = 0; i < formals->NumElements(); ++i)
    {
        scp->NewDecl(formals->Nth(i));
    }
    for (int i = 0; i < formals->NumElements(); ++i)
    {
        formals->Nth(i)->ConstructScp(scp);
    }
    if (body)
    { // not NULL
        body->ConstructScp(scp);
    }
}

void FnDecl::Check()
{
    for (int i = 0; i < formals->NumElements(); ++i)
    {
        formals->Nth(i)->Check();
    }
    if (body)
    { // not NULL
        body->Check();
    }
}

bool FnDecl::IsEquivalentTo(Decl *d)
{
    FnDecl *fn = dynamic_cast<FnDecl *>(d);
    if (fn == NULL)
    {
        return false;
    }
    if (!returnType->IsEquivalentTo(fn->GetReturn()))
    {
        return false;
    }
    if (formals->NumElements() != fn->GetFormals()->NumElements())
    {
        return false;
    }
    for (int i = 0; i < formals->NumElements(); ++i)
    {
        if (!formals->Nth(i)->IsEquivalentTo(fn->GetFormals()->Nth(i)))
        {
            return false;
        }
    }
    return true;
}