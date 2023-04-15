/* File: ast.cc
 * ------------
 */

#include "ast.h"
#include "ast_type.h"
#include "ast_decl.h"
#include <string.h> // strdup
#include <stdio.h>  // printf
#include "errors.h"

CodeGenerator *CodeGen = new CodeGenerator();

Node::Node(yyltype loc)
{
    location = new yyltype(loc);
    parent = NULL;
}

Node::Node()
{
    location = NULL;
    parent = NULL;
}

void Node::Print(int indentLevel, const char *label)
{
    const int numSpaces = 3;
    printf("\n");
    if (GetLocation())
        printf("%*d", numSpaces, GetLocation()->first_line);
    else
        printf("%*s", numSpaces, "");
    printf("%*s%s%s: ", indentLevel * numSpaces, "",
           label ? label : "", ReturnNodeName());
    ShowChildNodes(indentLevel);
}

Identifier::Identifier(yyltype loc, const char *n) : Node(loc)
{
    name = strdup(n);
}

void Identifier::ShowChildNodes(int indentLevel)
{
    printf("%s", name);
    if (cache)
        printf(" ---------------- {%d}", cache->GetIndex());
}

void Identifier::CheckDecl()
{
    Decl *d = symbol_table->Lookup(this);
    if (d == NULL)
    {
        ReportError::IdentifierNotDeclared(this, LookingForVariable);
    }
    else
    {
        this->SetCache(d);
    }
}

void Identifier::Check(checkT c)
{
    if (c == enum_DeclCheck)
    {
        this->CheckDecl();
    }
}

bool Identifier::Equivalent(Identifier *other)
{
    bool eq = false;
    if (!strcmp(name, other->ReturnIdenName()))
        eq = true;
    return eq;
}

void Identifier::Emit()
{
    if (cache)
        emit_loc = cache->GetEmitLoc();
}

void Identifier::SetPrefix(const char *prefix)
{
    char *s = (char *)malloc(strlen(name) + strlen(prefix) + 1);
    sprintf(s, "%s%s", prefix, name);
    name = s;
}
