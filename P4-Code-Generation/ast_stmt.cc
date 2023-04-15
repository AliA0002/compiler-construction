/* File: ast_stmt.cc
 * -----------------
 * Implementation of statement node classes.
 */
#include "ast_stmt.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "ast_expr.h"

Program::Program(List<Decl *> *d)
{
    Assert(d != NULL);
    (decls = d)->SetParentAll(this);
}

void Program::ShowChildNodes(int indentLevel)
{
    decls->PrintAll(indentLevel + 1);
    printf("\n");
}

void Program::GenerateST()
{
    if (IsDebugOn("ast"))
    {
        this->Print(0);
    }
    symbol_table = new STable();
    decls->DeclareAll();
    if (IsDebugOn("st"))
    {
        symbol_table->Print();
    }
    PrintDebug("ast+", "GenerateST finished.");
    if (IsDebugOn("ast+"))
    {
        this->Print(0);
    }
}

void Program::Check()
{
    symbol_table->ResetSymbolTable();
    decls->CheckAll(enum_DeclCheck);
    PrintDebug("ast+", "CheckDecl finished.");
    if (IsDebugOn("ast+"))
    {
        this->Print(0);
    }
    symbol_table->ResetSymbolTable();
    decls->CheckAll(enum_InheritCheck);
    PrintDebug("ast+", "CheckInherit finished.");
    if (IsDebugOn("ast+"))
    {
        this->Print(0);
    }
    symbol_table->ResetSymbolTable();
    decls->CheckAll(enum_TypeCheck);
    PrintDebug("ast+", "ConfirmType finished.");
    if (IsDebugOn("ast+"))
    {
        this->Print(0);
    }
}

void Program::Emit()
{
    bool main_exists = false;
    for (int i = 0; i < decls->NumElements(); i++)
    {
        Decl *d = decls->Nth(i);
        if (d->FnIsDecl())
        {
            if (!strcmp(d->GetId()->ReturnIdenName(), "main"))
            {
                main_exists = true;
                break;
            }
        }
    }
    if (!main_exists)
    {
        ReportError::NoMainFound();
        return;
    }

    PrintDebug("tac+", "Assign offset for class/interface members & global.");
    for (int i = 0; i < decls->NumElements(); i++)
    {
        decls->Nth(i)->OffsetAssign();
    }
    for (int i = 0; i < decls->NumElements(); i++)
    {
        decls->Nth(i)->PrefixForMember();
    }
    if (IsDebugOn("tac+"))
    {
        this->Print(0);
    }

    PrintDebug("tac+", "Begin Emitting TAC for Program.");
    decls->EmitAll();
    if (IsDebugOn("tac+"))
    {
        this->Print(0);
    }

    CodeGen->DoFinalCodeGen();
}

StmtBlock::StmtBlock(List<VarDecl *> *d, List<Stmt *> *s)
{
    Assert(d != NULL && s != NULL);
    (decls = d)->SetParentAll(this);
    (stmts = s)->SetParentAll(this);
}

void StmtBlock::ShowChildNodes(int indentLevel)
{
    decls->PrintAll(indentLevel + 1);
    stmts->PrintAll(indentLevel + 1);
}

void StmtBlock::GenerateST()
{
    symbol_table->GenerateScope();
    decls->DeclareAll();
    stmts->DeclareAll();
    symbol_table->ExitScope();
}

void StmtBlock::Check(checkT c)
{
    symbol_table->EnterScope();
    decls->CheckAll(c);
    stmts->CheckAll(c);
    symbol_table->ExitScope();
}

void StmtBlock::Emit()
{
    decls->EmitAll();
    stmts->EmitAll();
}

ConditionalStmt::ConditionalStmt(Expr *t, Stmt *b)
{
    Assert(t != NULL && b != NULL);
    (test = t)->SetParent(this);
    (body = b)->SetParent(this);
}

ForStmt::ForStmt(Expr *i, Expr *t, Expr *s, Stmt *b) : LoopStmt(t, b)
{
    Assert(i != NULL && t != NULL && s != NULL && b != NULL);
    (init = i)->SetParent(this);
    (step = s)->SetParent(this);
}

void ForStmt::ShowChildNodes(int indentLevel)
{
    init->Print(indentLevel + 1, "(init) ");
    test->Print(indentLevel + 1, "(test) ");
    step->Print(indentLevel + 1, "(step) ");
    body->Print(indentLevel + 1, "(body) ");
}

void ForStmt::GenerateST()
{
    symbol_table->GenerateScope();
    body->GenerateST();
    symbol_table->ExitScope();
}

void ForStmt::ConfirmType()
{
    init->Check(enum_TypeCheck);
    test->Check(enum_TypeCheck);
    if (test->ReturnType() && test->ReturnType() != Type::boolType)
    {
        ReportError::TestNotBoolean(test);
    }
    step->Check(enum_TypeCheck);
    symbol_table->EnterScope();
    body->Check(enum_TypeCheck);
    symbol_table->ExitScope();
}

void ForStmt::Check(checkT c)
{
    switch (c)
    {
    case enum_TypeCheck:
        this->ConfirmType();
        break;
    default:
        init->Check(c);
        test->Check(c);
        step->Check(c);
        symbol_table->EnterScope();
        body->Check(c);
        symbol_table->ExitScope();
    }
}

void ForStmt::Emit()
{
    init->Emit();
    const char *label0 = CodeGen->NewLabel();
    CodeGen->GenLabel(label0);
    test->Emit();
    Location *loc = test->ReturnEmitLocD();
    const char *label1 = CodeGen->NewLabel();
    LoopEndLabel = label1;
    CodeGen->GenIfZ(loc, label1);
    body->Emit();
    step->Emit();
    CodeGen->GenGoto(label0);
    CodeGen->GenLabel(label1);
}

void WhileStmt::ShowChildNodes(int indentLevel)
{
    test->Print(indentLevel + 1, "(test) ");
    body->Print(indentLevel + 1, "(body) ");
}

void WhileStmt::GenerateST()
{
    symbol_table->GenerateScope();
    body->GenerateST();
    symbol_table->ExitScope();
}

void WhileStmt::ConfirmType()
{
    test->Check(enum_TypeCheck);
    if (test->ReturnType() && test->ReturnType() != Type::boolType)
    {
        ReportError::TestNotBoolean(test);
    }
    symbol_table->EnterScope();
    body->Check(enum_TypeCheck);
    symbol_table->ExitScope();
}

void WhileStmt::Check(checkT c)
{
    switch (c)
    {
    case enum_TypeCheck:
        this->ConfirmType();
        break;
    default:
        test->Check(c);
        symbol_table->EnterScope();
        body->Check(c);
        symbol_table->ExitScope();
    }
}

void WhileStmt::Emit()
{
    const char *label0 = CodeGen->NewLabel();
    CodeGen->GenLabel(label0);

    test->Emit();
    Location *loc = test->ReturnEmitLocD();
    const char *label1 = CodeGen->NewLabel();
    LoopEndLabel = label1;
    CodeGen->GenIfZ(loc, label1);

    body->Emit();
    CodeGen->GenGoto(label0);

    CodeGen->GenLabel(label1);
}

IfStmt::IfStmt(Expr *t, Stmt *tb, Stmt *eb) : ConditionalStmt(t, tb)
{
    Assert(t != NULL && tb != NULL); // else can be NULL
    elseBody = eb;
    if (elseBody)
        elseBody->SetParent(this);
}

void IfStmt::ShowChildNodes(int indentLevel)
{
    test->Print(indentLevel + 1, "(test) ");
    body->Print(indentLevel + 1, "(then) ");
    if (elseBody)
        elseBody->Print(indentLevel + 1, "(else) ");
}

void IfStmt::GenerateST()
{
    symbol_table->GenerateScope();
    body->GenerateST();
    symbol_table->ExitScope();
    if (elseBody)
    {
        symbol_table->GenerateScope();
        elseBody->GenerateST();
        symbol_table->ExitScope();
    }
}

void IfStmt::ConfirmType()
{
    test->Check(enum_TypeCheck);
    if (test->ReturnType() && test->ReturnType() != Type::boolType)
    {
        ReportError::TestNotBoolean(test);
    }
    symbol_table->EnterScope();
    body->Check(enum_TypeCheck);
    symbol_table->ExitScope();
    if (elseBody)
    {
        symbol_table->EnterScope();
        elseBody->Check(enum_TypeCheck);
        symbol_table->ExitScope();
    }
}

void IfStmt::Check(checkT c)
{
    switch (c)
    {
    case enum_TypeCheck:
        this->ConfirmType();
        break;
    default:
        test->Check(c);
        symbol_table->EnterScope();
        body->Check(c);
        symbol_table->ExitScope();
        if (elseBody)
        {
            symbol_table->EnterScope();
            elseBody->Check(c);
            symbol_table->ExitScope();
        }
    }
}

void IfStmt::Emit()
{
    test->Emit();
    Location *loc = test->ReturnEmitLocD();
    const char *label0 = CodeGen->NewLabel();
    CodeGen->GenIfZ(loc, label0);

    body->Emit();
    const char *label1 = CodeGen->NewLabel();
    CodeGen->GenGoto(label1);

    CodeGen->GenLabel(label0);
    if (elseBody)
        elseBody->Emit();
    CodeGen->GenLabel(label1);
}

void BreakStmt::Check(checkT c)
{
    if (c == enum_TypeCheck)
    {
        Node *n = this;
        while (n->GetParent())
        {
            if (n->IsLoop())
                return;
            n = n->GetParent();
        }
        ReportError::BreakOutsideLoop(this);
    }
}

void BreakStmt::Emit() // RECHECK THIS
{
    Node *node = this;
    while (node->GetParent())
    {
        if (node->IsLoop())
        {
            const char *label = dynamic_cast<LoopStmt *>(node)->ReturnLoopLabel();
            CodeGen->GenGoto(label);
            return;
        }
        node = node->GetParent();
    }
}

ReturnStmt::ReturnStmt(yyltype loc, Expr *e) : Stmt(loc)
{
    Assert(e != NULL);
    (expr = e)->SetParent(this);
}

void ReturnStmt::ShowChildNodes(int indentLevel)
{
    expr->Print(indentLevel + 1);
}

void ReturnStmt::Check(checkT c)
{
    expr->Check(c);
    if (c == enum_TypeCheck)
    {
        Node *n = this;
        while (n->GetParent())
        {
            if (dynamic_cast<FnDecl *>(n) != NULL)
                break;
            n = n->GetParent();
        }
        Type *t_given = expr->ReturnType();
        Type *t_expected = dynamic_cast<FnDecl *>(n)->ReturnType();
        if (t_given && t_expected)
        {
            if (!t_expected->IsCompatibleWith(t_given))
            {
                ReportError::ReturnMismatch(this, t_given, t_expected);
            }
        }
    }
}

void ReturnStmt::Emit()
{
    if (expr->ExprIsEmpty())
    {
        CodeGen->GenReturn();
    }
    else
    {
        expr->Emit();
        CodeGen->GenReturn(expr->ReturnEmitLocD());
    }
}

PrintStmt::PrintStmt(List<Expr *> *a)
{
    Assert(a != NULL);
    (args = a)->SetParentAll(this);
}

void PrintStmt::ShowChildNodes(int indentLevel)
{
    args->PrintAll(indentLevel + 1, "(args) ");
}

void PrintStmt::Check(checkT c)
{
    args->CheckAll(c);
    if (c == enum_TypeCheck)
    {
        for (int i = 0; i < args->NumElements(); i++)
        {
            Type *t = args->Nth(i)->ReturnType();
            if (t != NULL && t != Type::stringType && t != Type::intType && t != Type::boolType)
            {
                ReportError::PrintArgMismatch(args->Nth(i), i + 1, t);
            }
        }
    }
}

void PrintStmt::Emit()
{
    for (int i = 0; i < args->NumElements(); ++i)
    {
        args->Nth(i)->Emit();
        Type *type = args->Nth(i)->ReturnType();
        BuiltIn func;
        if (type == Type::intType)
            func = PrintInt;
        else if (type == Type::boolType)
            func = PrintBool;
        else if (type == Type::stringType)
            func = PrintString;
        else
            func == PrintBool; // CHECK THIS
        Location *loc = args->Nth(i)->ReturnEmitLocD();
        Assert(loc);
        CodeGen->GenBuiltInCall(func, loc);
    }
}
