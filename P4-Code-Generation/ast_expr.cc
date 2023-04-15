/* File: ast_expr.cc
 * -----------------
 * Implementation of expression node classes.
 */
#include <iostream>
#include "ast_expr.h"
#include "ast_type.h"
#include "ast_decl.h"
#include <string.h>
#include "errors.h"

void EmptyExpr::ShowChildNodes(int indentLevel)
{
    if (type_of_expr)
        std::cout << " <" << type_of_expr << ">";
    if (emit_loc)
        emit_loc->Print();
}

void EmptyExpr::Check(checkT c)
{
    if (c == enum_TypeCheck)
    {
        type_of_expr = Type::voidType;
    }
}

IntConstant::IntConstant(yyltype loc, int val) : Expr(loc)
{
    value = val;
}
void IntConstant::ShowChildNodes(int indentLevel)
{
    printf("%d", value);
    if (type_of_expr)
        std::cout << " <" << type_of_expr << ">";
    if (emit_loc)
        emit_loc->Print();
}

void IntConstant::Check(checkT c)
{
    if (c == enum_DeclCheck)
    {
        type_of_expr = Type::intType;
    }
}

void IntConstant::Emit()
{
    emit_loc = CodeGen->GenLoadConstant(value);
}

DoubleConstant::DoubleConstant(yyltype loc, double val) : Expr(loc)
{
    value = val;
}

void DoubleConstant::ShowChildNodes(int indentLevel)
{
    printf("%g", value);
    if (type_of_expr)
        std::cout << " <" << type_of_expr << ">";
    if (emit_loc)
        emit_loc->Print();
}

void DoubleConstant::Check(checkT c)
{
    if (c == enum_DeclCheck)
    {
        type_of_expr = Type::doubleType;
    }
}

void DoubleConstant::Emit()
{
    ReportError::Formatted(this->GetLocation(),
                           "Double is not supported");
    Assert(0);
}

BoolConstant::BoolConstant(yyltype loc, bool val) : Expr(loc)
{
    value = val;
}

void BoolConstant::ShowChildNodes(int indentLevel)
{
    printf("%s", value ? "true" : "false");
    if (type_of_expr)
        std::cout << " <" << type_of_expr << ">";
    if (emit_loc)
        emit_loc->Print();
}

void BoolConstant::Check(checkT c)
{
    if (c == enum_DeclCheck)
    {
        type_of_expr = Type::boolType;
    }
}

void BoolConstant::Emit()
{
    int temp = 0;
    if (value == true)
        temp = 1;
    emit_loc = CodeGen->GenLoadConstant(temp);
}

StringConstant::StringConstant(yyltype loc, const char *val) : Expr(loc)
{
    Assert(val != NULL);
    value = strdup(val);
}
void StringConstant::ShowChildNodes(int indentLevel)
{
    printf("%s", value);
    if (type_of_expr)
        std::cout << " <" << type_of_expr << ">";
    if (emit_loc)
        emit_loc->Print();
}

void StringConstant::Check(checkT c)
{
    if (c == enum_DeclCheck)
    {
        type_of_expr = Type::stringType;
    }
}

void StringConstant::Emit()
{
    emit_loc = CodeGen->GenLoadConstant(value);
}

void NullConstant::ShowChildNodes(int indentLevel)
{
    if (type_of_expr)
        std::cout << " <" << type_of_expr << ">";
    if (emit_loc)
        emit_loc->Print();
}

void NullConstant::Check(checkT c)
{
    if (c == enum_DeclCheck)
    {
        type_of_expr = Type::nullType;
    }
}

void NullConstant::Emit()
{
    emit_loc = CodeGen->GenLoadConstant(0);
}

Operator::Operator(yyltype loc, const char *tok) : Node(loc)
{
    Assert(tok != NULL);
    strncpy(tokenString, tok, sizeof(tokenString));
}

void Operator::ShowChildNodes(int indentLevel)
{
    printf("%s", tokenString);
}

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

void CompoundExpr::ShowChildNodes(int indentLevel)
{
    if (type_of_expr)
        std::cout << " <" << type_of_expr << ">";
    if (emit_loc)
        emit_loc->Print();
    if (left)
        left->Print(indentLevel + 1);
    op->Print(indentLevel + 1);
    right->Print(indentLevel + 1);
}

void ArithmeticExpr::ConfirmType()
{
    if (left)
        left->Check(enum_TypeCheck);
    op->Check(enum_TypeCheck);
    right->Check(enum_TypeCheck);

    if (!strcmp(op->GetOpStr(), "-") && !left)
    {
        Type *tr = right->ReturnType();
        if (tr == NULL)
        {
            // some error accur in left or right, so skip it.
            return;
        }
        if (tr == Type::intType)
        {
            type_of_expr = Type::intType;
        }
        else if (tr == Type::doubleType)
        {
            type_of_expr = Type::doubleType;
        }
        else
        {
            ReportError::IncompatibleOperand(op, tr);
        }
    }
    else
    { // for + - * / %
        Type *tl = left->ReturnType();
        Type *tr = right->ReturnType();
        if (tl == NULL || tr == NULL)
        {
            // some error accur in left or right, so skip it.
            return;
        }
        if (tl == Type::intType && tr == Type::intType)
        {
            type_of_expr = Type::intType;
        }
        else if ((tl == Type::doubleType && tr == Type::doubleType)
                 // && strcmp(op->GetOpStr(), "%") // % can apply to float.
        )
        {
            type_of_expr = Type::doubleType;
        }
        else
        {
            ReportError::IncompatibleOperands(op, tl, tr);
        }
    }
}

void ArithmeticExpr::Check(checkT c)
{
    if (c == enum_TypeCheck)
    {
        this->ConfirmType();
    }
    else
    {
        if (left)
            left->Check(c);
        op->Check(c);
        right->Check(c);
    }
}

void ArithmeticExpr::Emit()
{
    bool left_exists = false;
    Location *loc;
    if (left != NULL)
    {
        left->Emit();
        left_exists = true;
    }
    right->Emit();
    if (left_exists)
        loc = left->ReturnEmitLocD();
    else
        loc = CodeGen->GenLoadConstant(0);

    emit_loc = CodeGen->GenBinaryOp(op->GetOpStr(), loc, right->ReturnEmitLocD());
}

void RelationalExpr::ConfirmType()
{
    left->Check(enum_TypeCheck);
    op->Check(enum_TypeCheck);
    right->Check(enum_TypeCheck);

    // the type of RelationalExpr is always boolType.
    type_of_expr = Type::boolType;

    Type *tl = left->ReturnType();
    Type *tr = right->ReturnType();

    if (tl == NULL || tr == NULL)
    {
        // some error accur in left or right, so skip it.
        return;
    }

    if (!(tl == Type::intType && tr == Type::intType) &&
        !(tl == Type::doubleType && tr == Type::doubleType))
    {
        ReportError::IncompatibleOperands(op, tl, tr);
    }
}

void RelationalExpr::Check(checkT c)
{
    if (c == enum_TypeCheck)
    {
        this->ConfirmType();
    }
    else
    {
        if (left)
            left->Check(c);
        op->Check(c);
        right->Check(c);
    }
}

void RelationalExpr::Emit()
{
    left->Emit();
    right->Emit();
    emit_loc = CodeGen->GenBinaryOp(op->GetOpStr(), left->ReturnEmitLocD(), right->ReturnEmitLocD());
}

void EqualityExpr::ConfirmType()
{
    left->Check(enum_TypeCheck);
    op->Check(enum_TypeCheck);
    right->Check(enum_TypeCheck);

    Type *tl = left->ReturnType();
    Type *tr = right->ReturnType();

    // the type of EqualityExpr is always boolType.
    type_of_expr = Type::boolType;

    if (tl == NULL || tr == NULL)
    {
        // some error accur in left or right, so skip it.
        return;
    }

    if (!tr->IsCompatibleWith(tl) && !tl->IsCompatibleWith(tr))
    {
        ReportError::IncompatibleOperands(op, tl, tr);
    }
}

void EqualityExpr::Check(checkT c)
{
    if (c == enum_TypeCheck)
    {
        this->ConfirmType();
    }
    else
    {
        if (left)
            left->Check(c);
        op->Check(c);
        right->Check(c);
    }
}

void EqualityExpr::Emit()
{
    const char *operator_type = op->GetOpStr(); // CHECK THIS LATER WHEN DEBUGGING
    left->Emit();
    right->Emit();

    Type *leftType = left->ReturnType();
    Type *rightType = right->ReturnType();

    if (leftType == rightType && leftType == Type::stringType)
    {
        emit_loc = CodeGen->GenBuiltInCall(StringEqual, left->ReturnEmitLocD(), right->ReturnEmitLocD());
        if (!strcmp(operator_type, "!="))
        {
            emit_loc = CodeGen->GenBinaryOp("==", CodeGen->GenLoadConstant(0), emit_loc);
        }
    }
    /*else if (leftType == rightType && (leftType == Type::boolType || leftType == Type::intType))
    {
        emit_loc = CodeGen->GenBinaryOp(operator_type, left->ReturnEmitLocD(), right->ReturnEmitLocD());
    }*/
    else
        emit_loc = CodeGen->GenBinaryOp(operator_type, left->ReturnEmitLocD(), right->ReturnEmitLocD());
}

void LogicalExpr::ConfirmType()
{
    if (left)
        left->Check(enum_TypeCheck);
    op->Check(enum_TypeCheck);
    right->Check(enum_TypeCheck);

    // the type of LogicalExpr is always boolType.
    type_of_expr = Type::boolType;

    if (!strcmp(op->GetOpStr(), "!"))
    {
        Type *tr = right->ReturnType();
        if (tr == NULL)
        {
            // some error accur in left or right, so skip it.
            return;
        }
        if (tr != Type::boolType)
        {
            ReportError::IncompatibleOperand(op, tr);
        }
    }
    else
    { // for && and ||
        Type *tl = left->ReturnType();
        Type *tr = right->ReturnType();
        if (tl == NULL || tr == NULL)
        {
            // some error accur in left or right, so skip it.
            return;
        }
        if (tl != Type::boolType || tr != Type::boolType)
        {
            ReportError::IncompatibleOperands(op, tl, tr);
        }
    }
}

void LogicalExpr::Check(checkT c)
{
    if (c == enum_TypeCheck)
    {
        this->ConfirmType();
    }
    else
    {
        if (left)
            left->Check(c);
        op->Check(c);
        right->Check(c);
    }
}

void LogicalExpr::Emit()
{
    if (left != NULL)
        left->Emit();
    right->Emit();

    if (left == NULL)
        emit_loc = CodeGen->GenBinaryOp("==", CodeGen->GenLoadConstant(0), right->ReturnEmitLocD());
    else
        emit_loc = CodeGen->GenBinaryOp(op->GetOpStr(), left->ReturnEmitLocD(), right->ReturnEmitLocD());
}

void AssignExpr::ConfirmType()
{
    left->Check(enum_TypeCheck);
    op->Check(enum_TypeCheck);
    right->Check(enum_TypeCheck);

    Type *tl = left->ReturnType();
    Type *tr = right->ReturnType();

    if (tl == NULL || tr == NULL)
    {
        // some error accur in left or right, so skip it.
        return;
    }

    if (!tl->IsCompatibleWith(tr))
    {
        ReportError::IncompatibleOperands(op, tl, tr);
    }
}

void AssignExpr::Check(checkT c)
{
    if (c == enum_TypeCheck)
    {
        this->ConfirmType();
    }
    else
    {
        left->Check(c);
        op->Check(c);
        right->Check(c);
    }
}

void AssignExpr::Emit()
{
    right->Emit();
    left->Emit();
    Location *rightLoc = right->ReturnEmitLocD();
    Location *leftLoc = left->GetEmitLoc();
    if (rightLoc && leftLoc)
    {
        if (leftLoc->GetBase() != NULL)
        {
            CodeGen->GenStore(leftLoc->GetBase(), rightLoc, leftLoc->GetOffset());
        }
        else if (left->AccessibleArray())
        {
            CodeGen->GenStore(leftLoc, rightLoc);
        }
        else
        {
            CodeGen->GenAssign(leftLoc, rightLoc);
        }
        emit_loc = left->ReturnEmitLocD();
    }
}

void This::ShowChildNodes(int indentLevel)
{
    if (type_of_expr)
        std::cout << " <" << type_of_expr << ">";
}

void This::ConfirmType()
{
    Decl *d = symbol_table->FindThis();
    if (!d || !d->IsClassDecl())
    {
        ReportError::ThisOutsideClassScope(this);
    }
    else
    {
        // Note: here create a new NamedType for 'this'.
        type_of_expr = new NamedType(d->GetId());
        type_of_expr->SetSelfType();
    }
}

void This::Check(checkT c)
{
    if (c == enum_TypeCheck)
    {
        this->ConfirmType();
    }
}

void This::Emit()
{
    emit_loc = CodeGen->ptrThis;
}

ArrayAccess::ArrayAccess(yyltype loc, Expr *b, Expr *s) : LValue(loc)
{
    (base = b)->SetParent(this);
    (subscript = s)->SetParent(this);
}

void ArrayAccess::ShowChildNodes(int indentLevel)
{
    if (type_of_expr)
        std::cout << " <" << type_of_expr << ">";
    if (emit_loc)
        emit_loc->Print();
    base->Print(indentLevel + 1);
    subscript->Print(indentLevel + 1, "(subscript) ");
}

void ArrayAccess::ConfirmType()
{
    Type *t;
    int err = 0;

    subscript->Check(enum_TypeCheck);
    t = subscript->ReturnType();
    if (t == NULL)
    {
        // some error accur in subscript, so skip it.
    }
    else if (t != Type::intType)
    {
        ReportError::SubscriptNotInteger(subscript);
    }

    base->Check(enum_TypeCheck);
    t = base->ReturnType();
    if (t == NULL)
    {
        // some error accur in base, so skip it.
        err++;
    }
    else if (!t->Type_ArrayType())
    {
        ReportError::BracketsOnNonArray(base);
        err++;
    }

    if (!err)
    {
        // the error of subscript will not affect the type of array access.
        type_of_expr = (dynamic_cast<ArrayType *>(t))->GetElemType();
    }
}

void ArrayAccess::Check(checkT c)
{
    if (c == enum_TypeCheck)
    {
        this->ConfirmType();
    }
    else
    {
        base->Check(c);
        subscript->Check(c);
    }
}

void ArrayAccess::Emit()
{
    base->Emit();
    subscript->Emit();
    Location *t0, *t1, *t2, *t3, *t4, *t5, *t6, *t7, *t8, *t9, *t10, *t11;
    t0 = subscript->ReturnEmitLocD();
    t1 = CodeGen->GenLoadConstant(0);
    t2 = CodeGen->GenBinaryOp("<", t0, t1);
    t3 = base->ReturnEmitLocD();
    t4 = CodeGen->GenLoad(t3, -4);
    t5 = CodeGen->GenBinaryOp("<", t0, t4);
    t6 = CodeGen->GenBinaryOp("==", t5, t1);
    t7 = CodeGen->GenBinaryOp("||", t2, t6);
    const char *l = CodeGen->NewLabel();
    CodeGen->GenIfZ(t7, l);
    t8 = CodeGen->GenLoadConstant(err_arr_out_of_bounds);
    CodeGen->GenBuiltInCall(PrintString, t8);
    CodeGen->GenBuiltInCall(Halt);
    CodeGen->GenLabel(l);
    t9 = CodeGen->GenLoadConstant(type_of_expr->ReturnTypeSize());
    t10 = CodeGen->GenBinaryOp("*", t9, t0);
    t11 = CodeGen->GenBinaryOp("+", t3, t10);
    emit_loc = t11;
}

Location *ArrayAccess::ReturnEmitLocD()
{
    Location *t = CodeGen->GenLoad(emit_loc, 0);
    return t;
}

FieldAccess::FieldAccess(Expr *b, Identifier *f)
    : LValue(b ? Join(b->GetLocation(), f->GetLocation()) : *f->GetLocation())
{
    Assert(f != NULL); // b can be be NULL (just means no explicit base)
    base = b;
    if (base)
        base->SetParent(this);
    (field = f)->SetParent(this);
}

void FieldAccess::ShowChildNodes(int indentLevel)
{
    if (type_of_expr)
        std::cout << " <" << type_of_expr << ">";
    if (emit_loc)
        emit_loc->Print();
    if (base)
        base->Print(indentLevel + 1);
    field->Print(indentLevel + 1);
}

void FieldAccess::CheckDecl()
{
    if (!base)
    {
        Decl *d = symbol_table->Lookup(field);
        if (d == NULL)
        {
            ReportError::IdentifierNotDeclared(field, LookingForVariable);
            return;
        }
        else
        {
            field->SetCache(d);
        }
    }
    else
    {
        base->Check(enum_DeclCheck);
    }
}

void FieldAccess::ConfirmType()
{
    if (!base)
    {
        if (field->ReturnCache())
        {
            if (field->ReturnCache()->IsVarDecl())
            {
                type_of_expr = field->ReturnCache()->ReturnType();
            }
            else
            {
                ReportError::IdentifierNotDeclared(field, LookingForVariable);
            }
        }
        return;
    }

    base->Check(enum_TypeCheck);
    Type *base_t = base->ReturnType();
    if (base_t != NULL)
    {
        if (!base_t->Type_NamedType())
        {
            ReportError::FieldNotFoundInBase(field, base_t);
            return;
        }

        Decl *d = symbol_table->LookForField(
            dynamic_cast<NamedType *>(base_t)->GetId(), field);

        if (d == NULL || !d->IsVarDecl())
        {
            ReportError::FieldNotFoundInBase(field, base_t);
        }
        else
        {
            Decl *cur_class = symbol_table->FindThis();
            if (!cur_class || !cur_class->IsClassDecl())
            {

                ReportError::InaccessibleField(field, base_t);
                return;
            }

            Type *cur_t = cur_class->ReturnType();
            d = symbol_table->LookForField(
                dynamic_cast<NamedType *>(cur_t)->GetId(), field);

            if (d == NULL || !d->IsVarDecl())
            {
                ReportError::FieldNotFoundInBase(field, cur_t);
                return;
            }
            if (cur_t->IsCompatibleWith(base_t) ||
                base_t->IsCompatibleWith(cur_t))
            {
                field->SetCache(d);
                type_of_expr = d->ReturnType();
            }
            else
            {
                ReportError::InaccessibleField(field, base_t);
            }
        }
    }
}

void FieldAccess::Check(checkT c)
{
    switch (c)
    {
    case enum_DeclCheck:
        this->CheckDecl();
        break;
    case enum_TypeCheck:
        this->ConfirmType();
        break;
    default:;
    }
}

void FieldAccess::Emit()
{
    if (base)
        base->Emit();
    field->Emit();
    emit_loc = field->ReturnEmitLocD();

    if (base)
        emit_loc = new Location(fpRelative, emit_loc->GetOffset(), emit_loc->GetName(), base->ReturnEmitLocD());
}

Location *FieldAccess::ReturnEmitLocD()
{
    Location *t = emit_loc;
    if (t->GetBase() != NULL)
    {
        // this or some class instances.
        t = CodeGen->GenLoad(t->GetBase(), t->GetOffset());
    }
    return t;
}

Call::Call(yyltype loc, Expr *b, Identifier *f, List<Expr *> *a) : Expr(loc)
{
    Assert(f != NULL && a != NULL);
    base = b;
    if (base)
        base->SetParent(this);
    (field = f)->SetParent(this);
    (actuals = a)->SetParentAll(this);
}

void Call::ShowChildNodes(int indentLevel)
{
    if (type_of_expr)
        std::cout << " <" << type_of_expr << ">";
    if (emit_loc)
        emit_loc->Print();
    if (base)
        base->Print(indentLevel + 1);
    field->Print(indentLevel + 1);
    actuals->PrintAll(indentLevel + 1, "(actuals) ");
}

void Call::CheckDecl()
{
    if (!base)
    {
        Decl *d = symbol_table->Lookup(field);
        if (d == NULL || !d->FnIsDecl())
        {
            ReportError::IdentifierNotDeclared(field, LookingForFunction);
            return;
        }
        else
        {
            field->SetCache(d);
            type_of_expr = d->ReturnType();
        }
    }
    else
    {
        base->Check(enum_DeclCheck);
    }
    actuals->CheckAll(enum_DeclCheck);
}

void Call::ConfirmType()
{
    if (!base)
    {
        if (field->ReturnCache() && !type_of_expr)
        {
            type_of_expr = field->ReturnCache()->ReturnType();
        }
    }
    else
    {
        base->Check(enum_TypeCheck);
        Type *type = base->ReturnType();
        if (type != NULL)
        {
            if (type->Type_ArrayType() && !strcmp(field->ReturnIdenName(), "length"))
            {
                int n = actuals->NumElements();
                if (n)
                {
                    ReportError::NumArgsMismatch(field, 0, n);
                }
                type_of_expr = Type::intType;
            }
            else if (!type->Type_NamedType())
            {
                ReportError::FieldNotFoundInBase(field, type);
            }
            else
            {
                Decl *d = symbol_table->LookForField(
                    dynamic_cast<NamedType *>(type)->GetId(), field);
                if (d == NULL || !d->FnIsDecl())
                {
                    ReportError::FieldNotFoundInBase(field, type);
                }
                else
                {
                    field->SetCache(d);
                    type_of_expr = d->ReturnType();
                }
            }
        }
    }
    actuals->CheckAll(enum_TypeCheck);
    this->CheckFuncArgs();
}

void Call::CheckFuncArgs()
{
    Decl *f = field->ReturnCache();
    if (!f || !f->FnIsDecl())
        return;
    FnDecl *fun = dynamic_cast<FnDecl *>(f);
    List<VarDecl *> *formals = fun->GetFormals();

    int n_expected = formals->NumElements();
    int n_given = actuals->NumElements();
    if (n_given != n_expected)
    {
        ReportError::NumArgsMismatch(field, n_expected, n_given);
    }
    else
    {
        for (int i = 0; i < actuals->NumElements(); i++)
        {
            Type *t_a = actuals->Nth(i)->ReturnType();
            Type *t_f = formals->Nth(i)->ReturnType();

            if (t_a && t_f && !t_f->IsCompatibleWith(t_a))
            {
                ReportError::ArgMismatch(actuals->Nth(i), i + 1, t_a, t_f);
            }
        }
    }
}

void Call::Check(checkT c)
{
    switch (c)
    {
    case enum_DeclCheck:
        this->CheckDecl();
        break;
    case enum_TypeCheck:
        this->ConfirmType();
        break;
    default:;
    }
}

void Call::Emit()
{
    if (base)
        base->Emit();
    field->Emit();
    actuals->EmitAll();

    Location *current_loc, *t;

    if (base && base->ReturnType()->Type_ArrayType() && !strcmp(field->ReturnIdenName(), "length"))
    {
        Location *t0 = base->ReturnEmitLocD();
        Location *t1 = CodeGen->GenLoad(t0, -4);
        emit_loc = t1;
        return;
    }

    FnDecl *func = dynamic_cast<FnDecl *>(field->ReturnCache());
    Assert(func != NULL);
    bool isCall = (base) || (func->MemberOfClass());

    if (base)
        current_loc = base->ReturnEmitLocD();
    else if (func->MemberOfClass())
        current_loc = CodeGen->ptrThis;

    if (isCall)
    {
        t = CodeGen->GenLoad(current_loc, 0);
        t = CodeGen->GenLoad(t, func->ReturnVTableOfst());
    }

    for (int i = actuals->NumElements() - 1; i >= 0; --i)
    {
        Location *loc = actuals->Nth(i)->ReturnEmitLocD();
        CodeGen->GenPushParam(loc);
    }

    if (isCall)
    {
        CodeGen->GenPushParam(current_loc);
        emit_loc = CodeGen->GenACall(t, func->HasReturnValue());
        CodeGen->GenPopParams(actuals->NumElements() * 4 + 4);
    }
    else
    {
        field->SetPrefix("_");
        emit_loc = CodeGen->GenLCall(field->ReturnIdenName(), type_of_expr != Type::voidType);
        CodeGen->GenPopParams(actuals->NumElements() * 4);
    }
}

NewExpr::NewExpr(yyltype loc, NamedType *c) : Expr(loc)
{
    Assert(c != NULL);
    (cType = c)->SetParent(this);
}

void NewExpr::ShowChildNodes(int indentLevel)
{
    if (type_of_expr)
        std::cout << " <" << type_of_expr << ">";
    if (emit_loc)
        emit_loc->Print();
    cType->Print(indentLevel + 1);
}

void NewExpr::CheckDecl()
{
    cType->Check(enum_DeclCheck, LookingForClass);
}

void NewExpr::ConfirmType()
{
    cType->Check(enum_TypeCheck);
    if (cType->ReturnType())
    {
        type_of_expr = cType;
    }
}

void NewExpr::Check(checkT c)
{
    switch (c)
    {
    case enum_DeclCheck:
        this->CheckDecl();
        break;
    case enum_TypeCheck:
        this->ConfirmType();
        break;
    default:
        cType->Check(c);
    }
}

void NewExpr::Emit()
{
    Location *t, *loc;
    ClassDecl *classdec = dynamic_cast<ClassDecl *>(cType->GetId()->ReturnCache());
    Assert(classdec != NULL);
    int size = classdec->GetInstanceSize();
    t = CodeGen->GenLoadConstant(size);
    emit_loc = CodeGen->GenBuiltInCall(Alloc, t);
    loc = CodeGen->GenLoadLabel(classdec->GetId()->ReturnIdenName());
    CodeGen->GenStore(emit_loc, loc, 0);
}

NewArrayExpr::NewArrayExpr(yyltype loc, Expr *sz, Type *et) : Expr(loc)
{
    Assert(sz != NULL && et != NULL);
    (size = sz)->SetParent(this);
    (elemType = et)->SetParent(this);
}

void NewArrayExpr::ShowChildNodes(int indentLevel)
{
    if (type_of_expr)
        std::cout << " <" << type_of_expr << ">";
    if (emit_loc)
        emit_loc->Print();
    size->Print(indentLevel + 1);
    elemType->Print(indentLevel + 1);
}

void NewArrayExpr::ConfirmType()
{
    Type *t;

    size->Check(enum_TypeCheck);
    t = size->ReturnType();
    if (t == NULL)
    {
    }

    else if (t != Type::intType)
    {
        ReportError::NewArraySizeNotInteger(size);
    }

    elemType->Check(enum_TypeCheck);
    if (!elemType->ReturnType())
    {
        return;
    }
    else
    {
        type_of_expr = new ArrayType(*location, elemType);
        type_of_expr->Check(enum_DeclCheck);
    }
}

void NewArrayExpr::Check(checkT c)
{
    if (c == enum_TypeCheck)
    {
        this->ConfirmType();
    }
    else
    {
        size->Check(c);
        elemType->Check(c);
    }
}

void NewArrayExpr::Emit()
{
    size->Emit();
    Location *t0, *t1, *t2, *t3, *t4, *t5, *t6, *t7, *t8, *t9;
    t0 = size->ReturnEmitLocD();
    t1 = CodeGen->GenLoadConstant(0);
    t2 = CodeGen->GenBinaryOp("<=", t0, t1);
    const char *label = CodeGen->NewLabel();
    CodeGen->GenIfZ(t2, label);
    t3 = CodeGen->GenLoadConstant(err_arr_bad_size);
    CodeGen->GenBuiltInCall(PrintString, t3);
    CodeGen->GenBuiltInCall(Halt);
    CodeGen->GenLabel(label);
    t4 = CodeGen->GenLoadConstant(1);
    t5 = CodeGen->GenBinaryOp("+", t4, t0);
    t6 = CodeGen->GenLoadConstant(elemType->ReturnTypeSize()); // May need to change?
    t7 = CodeGen->GenBinaryOp("*", t5, t6);
    t8 = CodeGen->GenBuiltInCall(Alloc, t7);
    CodeGen->GenStore(t8, t0);
    t9 = CodeGen->GenBinaryOp("+", t8, t6);
    emit_loc = t9;
}

void ReadIntegerExpr::Check(checkT c)
{
    if (c == enum_TypeCheck)
    {
        type_of_expr = Type::intType;
    }
}

void ReadIntegerExpr::Emit()
{
    emit_loc = CodeGen->GenBuiltInCall(ReadInteger);
}

void ReadLineExpr::Check(checkT c)
{
    if (c == enum_TypeCheck)
    {
        type_of_expr = Type::stringType;
    }
}

void ReadLineExpr::Emit()
{
    emit_loc = CodeGen->GenBuiltInCall(ReadLine);
}
