/* File: ast_stmt.cc
 * -----------------
 * Implementation of statement node classes.
 */
#include "ast_stmt.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "ast_expr.h"
#include "errors.h"

// Scope stuff
int Scope::NewDecl(Decl *dec)
{
    Decl *look_up = hashTable->Lookup(dec->ReturnName());
    if (look_up)
    { // not NULL
        ReportError::DeclConflict(dec, look_up);
        return 1;
    }
    hashTable->Enter(dec->ReturnName(), dec);
    // printf("%s\n", dec->ReturnName());
    return 0;
}

Scope *Program::globalScp = new Scope();

// Program stuff
Program::Program(List<Decl *> *d)
{
    Assert(d != NULL);
    (decls = d)->SetParentAll(this);
}

void Program::Check()
{
    /* pp3: here is where the semantic analyzer is kicked off.
     *      The general idea is perform a tree traversal of the
     *      entire program, examining all constructs for compliance
     *      with the semantic rules.  Each node can have its own way of
     *      checking itself, which makes for a great use of inheritance
     *      and polymorphism in the node classes.
     */

    ConstructScp();

    for (int i = 0; i < decls->NumElements(); ++i)
    {
        decls->Nth(i)->Check();
    }
}

void Program::ConstructScp()
{
    for (int i = 0; i < decls->NumElements(); ++i)
    {
        globalScp->NewDecl(decls->Nth(i));
    }
    for (int i = 0; i < decls->NumElements(); ++i)
    {
        decls->Nth(i)->ConstructScp(globalScp);
    }
}

// Stmt stuff
void Stmt::ConstructScp(Scope *par)
{
    scp->SetParent(par);
}

// StmtBlock stuff
StmtBlock::StmtBlock(List<VarDecl *> *d, List<Stmt *> *s)
{
    Assert(d != NULL && s != NULL);
    (decls = d)->SetParentAll(this);
    (stmts = s)->SetParentAll(this);
}

void StmtBlock::ConstructScp(Scope *par)
{
    scp->SetParent(par);
    for (int i = 0; i < decls->NumElements(); ++i)
    {
        scp->NewDecl(decls->Nth(i));
    }
    for (int i = 0; i < decls->NumElements(); ++i)
    {
        decls->Nth(i)->ConstructScp(scp);
    }
    for (int i = 0; i < stmts->NumElements(); ++i)
    {
        stmts->Nth(i)->ConstructScp(scp);
    }
}

void StmtBlock::Check()
{
    for (int i = 0; i < decls->NumElements(); ++i)
    {
        decls->Nth(i)->Check();
    }
    for (int i = 0; i < stmts->NumElements(); ++i)
    {
        stmts->Nth(i)->Check();
    }
}

// ConditionalStmt stuff
ConditionalStmt::ConditionalStmt(Expr *t, Stmt *b)
{
    Assert(t != NULL && b != NULL);
    (test = t)->SetParent(this);
    (body = b)->SetParent(this);
}

void ConditionalStmt::ConstructScp(Scope *par)
{
    scp->SetParent(par);
    test->ConstructScp(scp);
    body->ConstructScp(scp);
}

void ConditionalStmt::Check()
{
    if (!test->ReturnType()->IsEquivalentTo(Type::boolType))
    {
        ReportError::TestNotBoolean(test);
        return;
    }
    test->Check();
    body->Check();
}

// LoopStmt stuff
void LoopStmt::ConstructScp(Scope *par)
{
    scp->SetParent(par);
    scp->SetLoop(this);
    test->ConstructScp(scp);
    body->ConstructScp(scp);
}

// ForStmt stuff
ForStmt::ForStmt(Expr *i, Expr *t, Expr *s, Stmt *b) : LoopStmt(t, b)
{
    Assert(i != NULL && t != NULL && s != NULL && b != NULL);
    (init = i)->SetParent(this);
    (step = s)->SetParent(this);
}

// IfStmt stuff
IfStmt::IfStmt(Expr *t, Stmt *tb, Stmt *eb) : ConditionalStmt(t, tb)
{
    Assert(t != NULL && tb != NULL); // else can be NULL
    elseBody = eb;
    if (elseBody)
        elseBody->SetParent(this);
}

void IfStmt::ConstructScp(Scope *par)
{
    scp->SetParent(par);
    test->ConstructScp(scp);
    body->ConstructScp(scp);
    if (elseBody)
    { // if there is an "else"
        elseBody->ConstructScp(scp);
    }
}

void IfStmt::Check()
{
    test->Check();
    body->Check();
    if (elseBody)
    { // if there is an "else"
        elseBody->Check();
    }
    if (!test->ReturnType()->IsEquivalentTo(Type::boolType))
    {
        ReportError::TestNotBoolean(test);
        return;
    }
}

// ReturnStmt stuff
ReturnStmt::ReturnStmt(yyltype loc, Expr *e) : Stmt(loc)
{
    Assert(e != NULL);
    (expr = e)->SetParent(this);
}

void ReturnStmt::ConstructScp(Scope *par)
{
    scp->SetParent(par);
    expr->ConstructScp(scp);
}

void ReturnStmt::Check()
{
    expr->Check();
    Scope *s = scp;
    while (s)
    { // not NULL
        FnDecl *fun;
        if ((fun = s->GetFunction()) != NULL)
        { // not NULL
            Type *given = expr->ReturnType();
            Type *expect = fun->GetReturn();
            if (given->IsEquivalentTo(expect))
            {
                return;
            }
            ReportError::ReturnMismatch(this, given, expect);
        }
        s = s->GetParent();
    }
}

// PrintStmt stuff
PrintStmt::PrintStmt(List<Expr *> *a)
{
    Assert(a != NULL);
    (args = a)->SetParentAll(this);
}

void PrintStmt::ConstructScp(Scope *par)
{
    scp->SetParent(par);
    for (int i = 0; i < args->NumElements(); ++i)
    {
        args->Nth(i)->ConstructScp(scp);
    }
}

void PrintStmt::Check()
{
    for (int i = 0; i < args->NumElements(); ++i)
    {
        args->Nth(i)->Check();
    }
    for (int i = 0; i < args->NumElements(); ++i)
    {
        Expr *a = args->Nth(i);
        if (!a->ReturnType()->IsEquivalentTo(Type::stringType) &&
            !a->ReturnType()->IsEquivalentTo(Type::intType) &&
            !a->ReturnType()->IsEquivalentTo(Type::boolType))
        {
            ReportError::PrintArgMismatch(a, i + 1, a->ReturnType());
        }
    }
}

// BreakStmt stuff
void BreakStmt::Check()
{
    Scope *s = scp;
    while (s)
    { // not NULL
        if (s->GetLoop())
        { // not NULL
            return;
        }
        s = s->GetParent();
    }
    ReportError::BreakOutsideLoop(this);
}
