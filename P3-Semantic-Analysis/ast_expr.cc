/* File: ast_expr.cc
 * -----------------
 * Implementation of expression node classes.
 */
#include "ast_expr.h"
#include "ast_type.h"
#include "ast_decl.h"
#include <string.h>

// Expr stuff
ClassDecl *Expr::GetClass(Scope *s)
{
    ClassDecl *cl;
    while (s)
    { // not NULL
        if ((cl = s->GetClass()))
        { // not NULL
            return cl;
        }
        s = s->GetParent();
    }
    return NULL;
}

Decl *Expr::GetField(Identifier *ident, Type *t)
{
    NamedType *nt = dynamic_cast<NamedType *>(t);
    while (nt)
    { // not NULL
        Decl *dec = Program::globalScp->hashTable->Lookup(nt->ReturnName());
        ClassDecl *cl = dynamic_cast<ClassDecl *>(dec);
        InterfaceDecl *intf = dynamic_cast<InterfaceDecl *>(dec);
        if (cl)
        { // not NULL
            Decl *look_up = cl->GetScp()->hashTable->Lookup(ident->ReturnName());
            if (look_up)
            { // not NULL
                return look_up;
            }
            nt = cl->GetExtends();
        }
        else if (intf)
        { // not NULL
            Decl *look_up = intf->GetScp()->hashTable->Lookup(ident->ReturnName());
            if (look_up)
            { // not NULL
                return look_up;
            }
            nt = NULL;
        }
        else
        {
            nt = NULL;
        }
    }
    return GetField(ident, scp);
}

Decl *Expr::GetField(Identifier *ident, Scope *s)
{
    // int i = 0;
    while (s)
    { // not NULL
        Decl *look_up = s->hashTable->Lookup(ident->ReturnName());
        if (look_up)
        { // not NULL
            // printf("%s\n", ident->ReturnName());
            return look_up;
        }
        s = s->GetParent();
        // printf("%d ", i++);
    }
    return NULL;
}

// Constants stuff
IntConstant::IntConstant(yyltype loc, int val) : Expr(loc)
{
    value = val;
}

DoubleConstant::DoubleConstant(yyltype loc, double val) : Expr(loc)
{
    value = val;
}

BoolConstant::BoolConstant(yyltype loc, bool val) : Expr(loc)
{
    value = val;
}

StringConstant::StringConstant(yyltype loc, const char *val) : Expr(loc)
{
    Assert(val != NULL);
    value = strdup(val);
}

// Operator Stuff
Operator::Operator(yyltype loc, const char *tok) : Node(loc)
{
    Assert(tok != NULL);
    strncpy(tokenString, tok, sizeof(tokenString));
}

// CompoundExpr stuff
CompoundExpr::CompoundExpr(Expr *l, Operator *o, Expr *r)
    : Expr(Join(l->GetLocation(), r->GetLocation()))
{
    Assert(l != NULL && o != NULL && r != NULL);
    (op = o)->SetParent(this);
    (left = l)->SetParent(this);
    (right = r)->SetParent(this);
}

CompoundExpr::CompoundExpr(Operator *o, Expr *r)
    : Expr(Join(o->GetLocation(), r->GetLocation()))
{
    Assert(o != NULL && r != NULL);
    left = NULL;
    (op = o)->SetParent(this);
    (right = r)->SetParent(this);
}

void CompoundExpr::ConstructScp(Scope *par)
{
    scp->SetParent(par);
    if (left)
    { // not NULL
        left->ConstructScp(scp);
    }
    if (right)
    { // not NULL
        right->ConstructScp(scp);
    }
}

Type *CompoundExpr::ReturnType()
{
    if (left)
    { // not NULL
        return left->ReturnType();
    }
    else
    {
        return right->ReturnType();
    }
}

void CompoundExpr::Check()
{
    if (left)
    { // not NULL
        left->Check();
    }
    else
    {
        right->Check();
    }
}

// ArrayAccess stuff
ArrayAccess::ArrayAccess(yyltype loc, Expr *b, Expr *s) : LValue(loc)
{
    (base = b)->SetParent(this);
    (subscript = s)->SetParent(this);
}

void ArrayAccess::ConstructScp(Scope *par)
{
    scp->SetParent(par);
    base->ConstructScp(scp);
    subscript->ConstructScp(scp);
}

Type *ArrayAccess::ReturnType()
{
    ArrayType *at = dynamic_cast<ArrayType *>(base->ReturnType());
    if (at)
    { // not NULL
        return at->GetElem();
    }
    return Type::errorType;
}

void ArrayAccess::Check()
{
    base->Check();
    subscript->Check();
    ArrayType *at = dynamic_cast<ArrayType *>(base->ReturnType());
    Type *st = subscript->ReturnType();
    if (at == NULL)
    {
        ReportError::BracketsOnNonArray(base);
    }
    if (!st->IsEquivalentTo(Type::intType))
    {
        ReportError::SubscriptNotInteger(subscript);
    }
}

// FieldAccess stuff
FieldAccess::FieldAccess(Expr *b, Identifier *f)
    : LValue(b ? Join(b->GetLocation(), f->GetLocation()) : *f->GetLocation())
{
    Assert(f != NULL); // b can be be NULL (just means no explicit base)
    base = b;
    if (base)
        base->SetParent(this);
    (field = f)->SetParent(this);
}

void FieldAccess::ConstructScp(Scope *par)
{
    scp->SetParent(par);
    if (base)
    { // not NULL
        base->ConstructScp(scp);
    }
}

Type *FieldAccess::ReturnType()
{
    Decl *dec;
    ClassDecl *cl = GetClass(scp);
    Type *t;
    if (base == NULL)
    {
        if (cl == NULL)
        {
            dec = GetField(field, scp);
        }
        else
        {
            t = cl->ReturnType();
            dec = GetField(field, t);
        }
    }
    else
    {
        t = base->ReturnType();
        dec = GetField(field, t);
    }
    if (dec == NULL || dynamic_cast<VarDecl *>(dec) == NULL)
    {
        return Type::errorType;
    }
    return dynamic_cast<VarDecl *>(dec)->ReturnType();
}

void FieldAccess::Check()
{
    if (base)
    { // not NULL
        base->Check();
    }
    Decl *dec;
    ClassDecl *cl = GetClass(scp);
    Type *t;
    if (base == NULL)
    {
        if (cl == NULL)
        {
            if ((dec = GetField(field, scp)) == NULL)
            {
                ReportError::IdentifierNotDeclared(field, LookingForVariable);
                return;
            }
        }
        else
        {
            t = cl->ReturnType();
            if ((dec = GetField(field, t)) == NULL)
            {
                ReportError::FieldNotFoundInBase(field, t);
                return;
            }
        }
    }
    else
    {
        t = base->ReturnType();
        if ((dec = GetField(field, t)) == NULL)
        {
            ReportError::FieldNotFoundInBase(field, t);
            return;
        }
        else if (cl == NULL)
        {
            ReportError::InaccessibleField(field, t);
            return;
        }
    }
    if (dynamic_cast<VarDecl *>(dec) == NULL)
    {
        ReportError::IdentifierNotDeclared(field, LookingForVariable);
    }
}

// Call stuff
Call::Call(yyltype loc, Expr *b, Identifier *f, List<Expr *> *a) : Expr(loc)
{
    Assert(f != NULL && a != NULL); // b can be be NULL (just means no explicit base)
    base = b;
    if (base)
        base->SetParent(this);
    (field = f)->SetParent(this);
    (actuals = a)->SetParentAll(this);
}

void Call::ConstructScp(Scope *par)
{
    scp->SetParent(par);
    if (base)
    { // not NULL
        base->ConstructScp(scp);
    }
    for (int i = 0; i < actuals->NumElements(); ++i)
    {
        actuals->Nth(i)->ConstructScp(scp);
    }
}

Type *Call::ReturnType()
{
    Decl *dec;
    ClassDecl *cl = GetClass(scp);
    Type *t;
    if (base == NULL)
    {
        if (cl == NULL)
        {
            dec = GetField(field, scp);
        }
        else
        {
            t = cl->ReturnType();
            dec = GetField(field, t);
        }
    }
    else
    {
        t = base->ReturnType();
        dec = GetField(field, t);
        if (dec == NULL && dynamic_cast<ArrayType *>(t) && strcmp(field->ReturnName(), "length") == 0)
        {
            return Type::intType;
        }
    }
    if (dec == NULL || dynamic_cast<FnDecl *>(dec) == NULL)
    {
        return Type::errorType;
    }
    return dynamic_cast<FnDecl *>(dec)->GetReturn();
}

void Call::Check()
{
    if (base)
    { // not NULL
        base->Check();
    }
    for (int i = 0; i < actuals->NumElements(); ++i)
    {
        actuals->Nth(i)->Check();
    }
    Decl *dec;
    ClassDecl *cl = GetClass(scp);
    Type *t;
    if (base == NULL)
    {
        if (cl == NULL)
        {
            if ((dec = GetField(field, scp)) == NULL)
            {
                ReportError::IdentifierNotDeclared(field, LookingForFunction);
                return;
            }
        }
        else
        {
            t = cl->ReturnType();
            if ((dec = GetField(field, t)) == NULL)
            {
                ReportError::IdentifierNotDeclared(field, LookingForFunction);
                return;
            }
        }
    }
    else
    {
        t = base->ReturnType();
        if ((dec = GetField(field, t)) == NULL)
        {
            if (dynamic_cast<ArrayType *>(t) && strcmp(field->ReturnName(), "length") == 0)
            {
                return;
            }
            NamedType *nt = dynamic_cast<NamedType *>(t);
            if (nt)
            { // not NULL
                Decl *look_up = Program::globalScp->hashTable->Lookup(nt->ReturnName());
                if (look_up == NULL)
                {
                    return;
                }
            }
            ReportError::FieldNotFoundInBase(field, t);
            return;
        }
    }
    FnDecl *fn = dynamic_cast<FnDecl *>(dec);
    if (fn == NULL)
    {
        return;
    }
    CheckActuals(fn);
}

void Call::CheckActuals(FnDecl *fun)
{
    List<VarDecl *> *forms = fun->GetFormals();
    int numExpect = forms->NumElements();
    int numGiven = actuals->NumElements();
    if (numGiven != numExpect)
    {
        ReportError::NumArgsMismatch(field, numExpect, numGiven);
        return;
    }
    for (int i = 0; i < numExpect; ++i)
    {
        Type *g = actuals->Nth(i)->ReturnType();
        Type *e = forms->Nth(i)->ReturnType();
        if (!g->IsEquivalentTo(e))
        {
            ReportError::ArgMismatch(actuals->Nth(i), i + 1, g, e);
        }
    }
}

// NewExpr stuff
NewExpr::NewExpr(yyltype loc, NamedType *c) : Expr(loc)
{
    Assert(c != NULL);
    (cType = c)->SetParent(this);
}

Type *NewExpr::ReturnType()
{
    Decl *dec = Program::globalScp->hashTable->Lookup(cType->ReturnName());
    ClassDecl *cl = dynamic_cast<ClassDecl *>(dec);
    if (cl == NULL)
    {
        return Type::errorType;
    }
    return cl->ReturnType();
}

void NewExpr::Check()
{
    Decl *dec = Program::globalScp->hashTable->Lookup(cType->ReturnName());
    ClassDecl *cl = dynamic_cast<ClassDecl *>(dec);
    if (cl == NULL)
    {
        cType->ReportNotDeclIdent(LookingForClass);
    }
}

// NewArrayExpr stuff
NewArrayExpr::NewArrayExpr(yyltype loc, Expr *sz, Type *et) : Expr(loc)
{
    Assert(sz != NULL && et != NULL);
    (size = sz)->SetParent(this);
    (elemType = et)->SetParent(this);
}

void NewArrayExpr::ConstructScp(Scope *par)
{
    scp->SetParent(par);
    size->ConstructScp(scp);
}

Type *NewArrayExpr::ReturnType()
{
    return new ArrayType(elemType);
}

void NewArrayExpr::Check()
{
    size->Check();
    if (!size->ReturnType()->IsEquivalentTo(Type::intType))
    {
        ReportError::NewArraySizeNotInteger(size);
    }
    if (elemType->IsPrim() && !elemType->IsEquivalentTo(Type::voidType))
    {
        return;
    }
    Decl *dec = Program::globalScp->hashTable->Lookup(elemType->ReturnName());
    if (dynamic_cast<ClassDecl *>(dec) == NULL)
    {
        elemType->ReportNotDeclIdent(LookingForType);
    }
}

// ArithmeticExpr stuff
Type *ArithmeticExpr::ReturnType()
{
    if (left && right)
    { // not NULL
        Type *lt = left->ReturnType();
        Type *rt = right->ReturnType();
        if (lt->IsEquivalentTo(Type::intType) && rt->IsEquivalentTo(Type::intType))
        {
            return lt;
        }
        if (lt->IsEquivalentTo(Type::doubleType) && rt->IsEquivalentTo(Type::doubleType))
        {
            return lt;
        }
        return Type::errorType;
    }
    Type *ut;
    if (left)
    { // not NULL
        ut = left->ReturnType();
    }
    else
    {
        ut = right->ReturnType();
    }
    if (ut->IsEquivalentTo(Type::intType) || ut->IsEquivalentTo(Type::doubleType))
    {
        return ut;
    }
    return Type::errorType;
}

void ArithmeticExpr::Check()
{
    if (left)
    { // not NULL
        left->Check();
    }
    if (right)
    { // not NULL
        right->Check();
    }
    if (left && right)
    { // not NULL
        Type *lt = left->ReturnType();
        Type *rt = right->ReturnType();
        if (lt->IsEquivalentTo(Type::intType) && rt->IsEquivalentTo(Type::intType))
        {
            return;
        }
        if (lt->IsEquivalentTo(Type::doubleType) && rt->IsEquivalentTo(Type::doubleType))
        {
            return;
        }
        ReportError::IncompatibleOperands(op, lt, rt);
        return;
    }
    Type *ut;
    if (left)
    { // not NULL
        ut = left->ReturnType();
    }
    else
    {
        ut = right->ReturnType();
    }
    if (ut->IsEquivalentTo(Type::intType) || ut->IsEquivalentTo(Type::doubleType))
    {
        return;
    }
    ReportError::IncompatibleOperand(op, ut);
}

// RelationalExpr stuff
Type *RelationalExpr::ReturnType()
{
    Type *lt = left->ReturnType();
    Type *rt = right->ReturnType();
    if (lt->IsEquivalentTo(Type::intType) && rt->IsEquivalentTo(Type::intType))
    {
        return Type::boolType;
    }
    if (lt->IsEquivalentTo(Type::doubleType) && rt->IsEquivalentTo(Type::doubleType))
    {
        return Type::boolType;
    }
    return Type::errorType;
}

void RelationalExpr::Check()
{
    left->Check();
    right->Check();
    Type *lt = left->ReturnType();
    Type *rt = right->ReturnType();
    if (lt->IsEquivalentTo(Type::intType) && rt->IsEquivalentTo(Type::intType))
    {
        return;
    }
    if (lt->IsEquivalentTo(Type::doubleType) && rt->IsEquivalentTo(Type::doubleType))
    {
        return;
    }
    ReportError::IncompatibleOperands(op, lt, rt);
}

// EqualityExpr stuff
Type *EqualityExpr::ReturnType()
{
    Type *lt = left->ReturnType();
    Type *rt = right->ReturnType();
    if (lt->IsEquivalentTo(rt) || rt->IsEquivalentTo(lt))
    {
        return Type::boolType;
    }
    return Type::errorType;
}

void EqualityExpr::Check()
{
    left->Check();
    right->Check();
    Type *lt = left->ReturnType();
    Type *rt = right->ReturnType();
    if (!(lt->IsEquivalentTo(rt) || rt->IsEquivalentTo(lt)))
    {
        ReportError::IncompatibleOperands(op, lt, rt);
    }
    if (lt->IsEquivalentTo(Type::voidType) || rt->IsEquivalentTo(Type::voidType))
    {
        ReportError::IncompatibleOperands(op, lt, rt);
    }
}

// LogicalExpr stuff
Type *LogicalExpr::ReturnType()
{
    Type *rt = right->ReturnType();
    if (left == NULL)
    {
        if (rt->IsEquivalentTo(Type::boolType))
        {
            return Type::boolType;
        }
        return Type::errorType;
    }
    Type *lt = left->ReturnType();
    if (lt->IsEquivalentTo(Type::boolType) && rt->IsEquivalentTo(Type::boolType))
    {
        return Type::boolType;
    }
    return Type::errorType;
}

void LogicalExpr::Check()
{
    if (left)
    { // not NULL
        left->Check();
    }
    right->Check();
    Type *rt = right->ReturnType();
    if (left == NULL)
    {
        if (rt->IsEquivalentTo(Type::boolType))
        {
            return;
        }
        ReportError::IncompatibleOperand(op, rt);
        return;
    }
    Type *lt = left->ReturnType();
    if (lt->IsEquivalentTo(Type::boolType) && rt->IsEquivalentTo(Type::boolType))
    {
        return;
    }
    ReportError::IncompatibleOperands(op, lt, rt);
}

// AssignExpr stuff
Type *AssignExpr::ReturnType()
{
    Type *lt = left->ReturnType();
    Type *rt = right->ReturnType();
    if (lt->IsEquivalentTo(rt))
    {
        return lt;
    }
    return Type::errorType;
}

void AssignExpr::Check()
{
    // printf("bruh");
    left->Check();
    right->Check();
    Type *lt = left->ReturnType();
    Type *rt = right->ReturnType();
    if (rt->IsEquivalentTo(lt))
    {
        return;
    }
    ReportError::IncompatibleOperands(op, lt, rt);
}

// This stuff
Type *This::ReturnType()
{
    ClassDecl *cl = GetClass(scp);
    if (cl)
    { // not NULL
        return cl->ReturnType();
    }
    return Type::errorType;
}

void This::Check()
{
    ClassDecl *cl = GetClass(scp);
    if (cl)
    { // not NULL
        return;
    }
    ReportError::ThisOutsideClassScope(this);
}

// ReadIntegerExpr stuff
Type *ReadIntegerExpr::ReturnType()
{
    return Type::intType;
}

// ReadLineExpr stuff
Type *ReadLineExpr::ReturnType()
{
    return Type::stringType;
}