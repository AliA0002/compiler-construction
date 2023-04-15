/* File: ast_decl.cc
 * -----------------
 * Implementation of Decl node classes.
 */
#include "ast_decl.h"
#include "ast_type.h"
#include "ast_stmt.h"
#include "errors.h"
#include "list.h"

Decl::Decl(Identifier *n) : Node(*n->GetLocation())
{
    Assert(n != NULL);
    (id = n)->SetParent(this);
    idx = -1;
    type_of_expr = NULL;
}

VarDecl::VarDecl(Identifier *n, Type *t) : Decl(n)
{
    Assert(n != NULL && t != NULL);
    (type = t)->SetParent(this);
    class_member_offset = -1;
}

void VarDecl::ShowChildNodes(int indentLevel)
{
    if (type_of_expr)
        std::cout << " <" << type_of_expr << ">";
    if (emit_loc)
        emit_loc->Print();
    if (class_member_offset != -1)
        std::cout << " ~~[Ofst: " << class_member_offset << "]";
    type->Print(indentLevel + 1);
    id->Print(indentLevel + 1);
    if (id->ReturnCache())
        printf(" ........ {def}");
}

void VarDecl::GenerateST()
{
    if (symbol_table->LocalLookup(this->GetId()))
    {
        Decl *d = symbol_table->Lookup(this->GetId());
        ReportError::DeclConflict(this, d);
    }
    else
    {
        idx = symbol_table->InsertSymbol(this);
        id->SetCache(this);
    }
}

void VarDecl::CheckDecl()
{
    type->Check(enum_DeclCheck);
    id->Check(enum_DeclCheck);

    type_of_expr = type->ReturnType();
}

void VarDecl::Check(checkT c)
{
    switch (c)
    {
    case enum_DeclCheck:
        this->CheckDecl();
        break;
    default:
        type->Check(c);
        id->Check(c);
    }
}

void VarDecl::OffsetAssign()
{
    if (this->IsGlobalVar())
    {
        emit_loc = new Location(gpRelative, CodeGen->GetNextGlobal(), id->ReturnIdenName());
    }
}

void VarDecl::OffsetForMember(bool inClass, int offset)
{
    class_member_offset = offset;
    emit_loc = new Location(fpRelative, offset, id->ReturnIdenName(), CodeGen->ptrThis);
}

void VarDecl::Emit()
{
    if (type == Type::doubleType)
    {
        ReportError::Formatted(this->GetLocation(), "Double not supported");
        Assert(false);
    }
    if (!emit_loc)
        emit_loc = new Location(fpRelative, CodeGen->GetNextLocal(), id->ReturnIdenName());
}

ClassDecl::ClassDecl(Identifier *n, NamedType *ex, List<NamedType *> *imp, List<Decl *> *m) : Decl(n)
{
    Assert(n != NULL && imp != NULL && m != NULL);
    extends = ex;
    if (extends)
        extends->SetParent(this);
    (implements = imp)->SetParentAll(this);
    (members = m)->SetParentAll(this);
    inst_size = 4;
    vtable_size = 0;
}

void ClassDecl::ShowChildNodes(int indentLevel)
{
    if (type_of_expr)
        std::cout << " <" << type_of_expr << ">";
    if (emit_loc)
        emit_loc->Print();
    id->Print(indentLevel + 1);
    if (id->ReturnCache())
        printf(" ........ {def}");
    if (extends)
        extends->Print(indentLevel + 1, "(extends) ");
    implements->PrintAll(indentLevel + 1, "(implements) ");
    members->PrintAll(indentLevel + 1);
}

void ClassDecl::GenerateST()
{
    if (symbol_table->LocalLookup(this->GetId()))
    {
        Decl *d = symbol_table->Lookup(this->GetId());
        ReportError::DeclConflict(this, d);
    }
    else
    {
        idx = symbol_table->InsertSymbol(this);
        id->SetCache(this);
    }
    symbol_table->GenerateScope(this->GetId()->ReturnIdenName());
    if (extends)
    {
        symbol_table->SetScopeParent(extends->GetId()->ReturnIdenName());
    }
    for (int i = 0; i < implements->NumElements(); i++)
    {
        symbol_table->SetInterface(implements->Nth(i)->GetId()->ReturnIdenName());
    }
    members->DeclareAll();
    symbol_table->ExitScope();
}

void ClassDecl::CheckDecl()
{
    id->Check(enum_DeclCheck);
    if (extends)
    {
        extends->Check(enum_DeclCheck, LookingForClass);
    }
    for (int i = 0; i < implements->NumElements(); i++)
    {
        implements->Nth(i)->Check(enum_DeclCheck, LookingForInterface);
    }
    symbol_table->EnterScope();
    members->CheckAll(enum_DeclCheck);
    symbol_table->ExitScope();

    type_of_expr = new NamedType(id);
    type_of_expr->SetSelfType();
}

void ClassDecl::CheckInherit()
{
    symbol_table->EnterScope();

    for (int i = 0; i < members->NumElements(); i++)
    {
        Decl *d = members->Nth(i);
        Assert(d != NULL);

        if (d->IsVarDecl())
        {
            Decl *t = symbol_table->FindParent(d->GetId());
            if (t != NULL)
            {
                ReportError::DeclConflict(d, t);
            }

            t = symbol_table->FindInterface(d->GetId());
            if (t != NULL)
            {
                ReportError::DeclConflict(d, t);
            }
        }
        else if (d->FnIsDecl())
        {
            // check class inheritance of functions.
            Decl *t = symbol_table->FindParent(d->GetId());
            if (t != NULL)
            {
                if (!t->FnIsDecl())
                {
                    ReportError::DeclConflict(d, t);
                }
                else
                {
                    FnDecl *fn1 = dynamic_cast<FnDecl *>(d);
                    FnDecl *fn2 = dynamic_cast<FnDecl *>(t);
                    if (fn1->ReturnType() && fn2->ReturnType() && !fn1->Equivalent(fn2))
                    {
                        ReportError::OverrideMismatch(d);
                    }
                }
            }
            t = symbol_table->FindInterface(d->GetId());
            if (t != NULL)
            {
                FnDecl *fn1 = dynamic_cast<FnDecl *>(d);
                FnDecl *fn2 = dynamic_cast<FnDecl *>(t);
                if (fn1->ReturnType() && fn2->ReturnType() && !fn1->Equivalent(fn2))
                {
                    ReportError::OverrideMismatch(d);
                }
            }
            d->Check(enum_InheritCheck);
        }
    }

    for (int i = 0; i < implements->NumElements(); i++)
    {
        Decl *d = implements->Nth(i)->GetId()->ReturnCache();
        if (d != NULL)
        {
            List<Decl *> *m = dynamic_cast<InterfaceDecl *>(d)->GetMembers();
            for (int j = 0; j < m->NumElements(); j++)
            {
                Identifier *mid = m->Nth(j)->GetId();
                Decl *t = symbol_table->LookForField(this->id, mid);
                if (t == NULL)
                {
                    ReportError::InterfaceNotImplemented(this,
                                                         implements->Nth(i));
                    break;
                }
                else
                {
                    FnDecl *fn1 = dynamic_cast<FnDecl *>(m->Nth(j));
                    FnDecl *fn2 = dynamic_cast<FnDecl *>(t);
                    if (!fn1 || !fn2 || !fn1->ReturnType() || !fn2->ReturnType() || !fn1->Equivalent(fn2))
                    {
                        ReportError::InterfaceNotImplemented(this,
                                                             implements->Nth(i));
                        break;
                    }
                }
            }
        }
    }
    symbol_table->ExitScope();
}

void ClassDecl::Check(checkT c)
{
    switch (c)
    {
    case enum_DeclCheck:
        this->CheckDecl();
        break;
    case enum_InheritCheck:
        this->CheckInherit();
        break;
    default:
        id->Check(c);
        if (extends)
            extends->Check(c);
        implements->CheckAll(c);
        symbol_table->EnterScope();
        members->CheckAll(c);
        symbol_table->ExitScope();
    }
}

bool ClassDecl::IsChildOf(Decl *other)
{
    if (other->IsClassDecl())
    {
        if (id->Equivalent(other->GetId()))
        {
            // self.
            return true;
        }
        else if (!extends)
        {
            return false;
        }
        else
        {
            Decl *d = extends->GetId()->ReturnCache();
            return dynamic_cast<ClassDecl *>(d)->IsChildOf(other);
        }
    }
    else if (other->IsInterfaceDecl())
    {
        for (int i = 0; i < implements->NumElements(); i++)
        {
            Identifier *iid = implements->Nth(i)->GetId();
            if (iid->Equivalent(other->GetId()))
            {
                return true;
            }
        }
        if (!extends)
        {
            return false;
        }
        else
        {
            Decl *d = extends->GetId()->ReturnCache();
            return dynamic_cast<ClassDecl *>(d)->IsChildOf(other);
        }
    }
    else
    {
        return false;
    }
}

void ClassDecl::MembersForList(List<VarDecl *> *vars, List<FnDecl *> *fns)
{
    for (int i = members->NumElements() - 1; i >= 0; --i)
    {
        Decl *dec = members->Nth(i);
        if (dec->IsVarDecl())
            vars->InsertAt(dynamic_cast<VarDecl *>(dec), 0);
        else if (dec->FnIsDecl())
            fns->InsertAt(dynamic_cast<FnDecl *>(dec), 0);
    }
}

void ClassDecl::OffsetAssign()
{
    var_members = new List<VarDecl *>;
    fn_members = new List<FnDecl *>;
    ClassDecl *class_dec = this;
    while (class_dec)
    {
        class_dec->MembersForList(var_members, fn_members);
        NamedType *type = class_dec->GetExtends();
        if (!type)
            break;
        class_dec = dynamic_cast<ClassDecl *>(type->GetId()->ReturnCache());
    }

    for (int i = 0; i < fn_members->NumElements(); i++)
    {
        FnDecl *func1 = fn_members->Nth(i);
        for (int j = i + 1; j < fn_members->NumElements(); j++)
        {
            FnDecl *func2 = fn_members->Nth(j);
            if (!strcmp(func1->GetId()->ReturnIdenName(), func2->GetId()->ReturnIdenName()))
            {
                fn_members->RemoveAt(i);
                fn_members->InsertAt(func2, i);
                fn_members->RemoveAt(j);
                j--;
            }
        }
    }

    inst_size = var_members->NumElements() * 4 + 4;
    vtable_size = fn_members->NumElements() * 4;

    int temp = inst_size;
    for (int i = members->NumElements() - 1; i >= 0; i--)
    {
        Decl *dec = members->Nth(i);
        if (dec->IsVarDecl())
        {
            temp -= 4;
            dec->OffsetForMember(true, temp);
        }
        else if (dec->FnIsDecl())
        {
            for (int i = 0; i < fn_members->NumElements(); i++)
            {
                FnDecl *func1 = fn_members->Nth(i);
                if (!strcmp(func1->GetId()->ReturnIdenName(), dec->GetId()->ReturnIdenName()))
                    dec->OffsetForMember(true, i * 4);
            }
        }
    }
}

void ClassDecl::PrefixForMember()
{
    for (int i = 0; i < members->NumElements(); ++i)
        members->Nth(i)->PrefixForMember();
}

void ClassDecl::Emit()
{
    members->EmitAll();

    List<const char *> *method_l = new List<const char *>;
    for (int i = 0; i < fn_members->NumElements(); ++i)
    {
        FnDecl *func = fn_members->Nth(i);
        method_l->Append(func->GetId()->ReturnIdenName());
    }
    CodeGen->GenVTable(id->ReturnIdenName(), method_l);
}

InterfaceDecl::InterfaceDecl(Identifier *n, List<Decl *> *m) : Decl(n)
{
    Assert(n != NULL && m != NULL);
    (members = m)->SetParentAll(this);
}

void InterfaceDecl::ShowChildNodes(int indentLevel)
{
    if (type_of_expr)
        std::cout << " <" << type_of_expr << ">";
    if (emit_loc)
        emit_loc->Print();
    id->Print(indentLevel + 1);
    if (id->ReturnCache())
        printf(" ........ {def}");
    members->PrintAll(indentLevel + 1);
}

void InterfaceDecl::GenerateST()
{
    if (symbol_table->LocalLookup(this->GetId()))
    {
        Decl *d = symbol_table->Lookup(this->GetId());
        ReportError::DeclConflict(this, d);
    }
    else
    {
        idx = symbol_table->InsertSymbol(this);
        id->SetCache(this);
    }
    symbol_table->GenerateScope(this->GetId()->ReturnIdenName());
    members->DeclareAll();
    symbol_table->ExitScope();
}

void InterfaceDecl::Check(checkT c)
{
    switch (c)
    {
    case enum_DeclCheck:
        type_of_expr = new NamedType(id);
        type_of_expr->SetSelfType();
    default:
        id->Check(c);
        symbol_table->EnterScope();
        members->CheckAll(c);
        symbol_table->ExitScope();
    }
}

void InterfaceDecl::Emit()
{
    ReportError::Formatted(this->GetLocation(),
                           "Interface is not supported");
    Assert(0);
}

FnDecl::FnDecl(Identifier *n, Type *r, List<VarDecl *> *d) : Decl(n)
{
    Assert(n != NULL && r != NULL && d != NULL);
    (returnType = r)->SetParent(this);
    (formals = d)->SetParentAll(this);
    body = NULL;
    vtable_ofst = -1;
}

void FnDecl::SetBodyOfFunction(Stmt *b)
{
    (body = b)->SetParent(this);
}

void FnDecl::ShowChildNodes(int indentLevel)
{
    if (type_of_expr)
        std::cout << " <" << type_of_expr << ">";
    if (emit_loc)
        emit_loc->Print();
    if (vtable_ofst != -1)
        std::cout << " ~~[VTable: " << vtable_ofst << "]";
    returnType->Print(indentLevel + 1, "(return type) ");
    id->Print(indentLevel + 1);
    if (id->ReturnCache())
        printf(" ........ {def}");
    formals->PrintAll(indentLevel + 1, "(formals) ");
    if (body)
        body->Print(indentLevel + 1, "(body) ");
}

void FnDecl::GenerateST()
{
    if (symbol_table->LocalLookup(this->GetId()))
    {
        Decl *d = symbol_table->Lookup(this->GetId());
        ReportError::DeclConflict(this, d);
    }
    else
    {
        idx = symbol_table->InsertSymbol(this);
        id->SetCache(this);
    }
    symbol_table->GenerateScope();
    formals->DeclareAll();
    if (body)
        body->GenerateST();
    symbol_table->ExitScope();
}

void FnDecl::CheckDecl()
{
    returnType->Check(enum_DeclCheck);
    id->Check(enum_DeclCheck);
    symbol_table->EnterScope();
    formals->CheckAll(enum_DeclCheck);
    if (body)
        body->Check(enum_DeclCheck);
    symbol_table->ExitScope();

    if (!strcmp(id->ReturnIdenName(), "main"))
    {
        if (returnType != Type::voidType)
        {
            ReportError::Formatted(this->GetLocation(),
                                   "Return value of 'main' function is expected to be void.");
        }
        if (formals->NumElements() != 0)
        {
            ReportError::NumArgsMismatch(id, 0, formals->NumElements());
        }
    }

    type_of_expr = returnType->ReturnType();
}

void FnDecl::Check(checkT c)
{
    switch (c)
    {
    case enum_DeclCheck:
        this->CheckDecl();
        break;
    default:
        returnType->Check(c);
        id->Check(c);
        symbol_table->EnterScope();
        formals->CheckAll(c);
        if (body)
            body->Check(c);
        symbol_table->ExitScope();
    }
}

bool FnDecl::Equivalent(Decl *other)
{
    Assert(this->ReturnType() && other->ReturnType());

    if (!other->FnIsDecl())
    {
        return false;
    }
    FnDecl *fn = dynamic_cast<FnDecl *>(other);
    if (!returnType->Equivalent(fn->ReturnType()))
    {
        return false;
    }
    if (formals->NumElements() != fn->GetFormals()->NumElements())
    {
        return false;
    }
    for (int i = 0; i < formals->NumElements(); i++)
    {
        Type *var_type1 =
            (dynamic_cast<VarDecl *>(formals->Nth(i)))->ReturnType();
        Type *var_type2 =
            (dynamic_cast<VarDecl *>(fn->GetFormals()->Nth(i)))->ReturnType();
        if (!var_type1->Equivalent(var_type2))
        {
            return false;
        }
    }
    return true;
}

void FnDecl::PrefixForMember()
{
    Decl *dec = dynamic_cast<Decl *>(this->GetParent());
    if (dec && dec->IsClassDecl())
    {
        id->SetPrefix(".");
        id->SetPrefix(dec->GetId()->ReturnIdenName());
        id->SetPrefix("_");
    }
    else if (strcmp(id->ReturnIdenName(), "main"))
    {
        id->SetPrefix("_");
    }
}

void FnDecl::OffsetForMember(bool inClass, int offset)
{
    vtable_ofst = offset;
}

void FnDecl::Emit()
{
    Decl *dec = dynamic_cast<Decl *>(this->GetParent());
    CodeGen->GenLabel(id->ReturnIdenName());

    BeginFunc *func = CodeGen->GenBeginFunc();

    if (dec && dec->IsClassDecl())
    {
        CodeGen->GetNextParam();
    }

    for (int i = 0; i < formals->NumElements(); i++)
    {
        VarDecl *v = formals->Nth(i);
        Location *l = new Location(fpRelative, CodeGen->GetNextParam(),
                                   v->GetId()->ReturnIdenName());
        v->SetEmitLoc(l);
    }

    if (body)
        body->Emit();

    func->SetFrameSize(CodeGen->GetFrameSize());

    CodeGen->GenEndFunc();
}
