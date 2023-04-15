/* File: parser.y
 * --------------
 * Yacc input file to generate the parser for the compiler.
 *
 * pp2: your job is to write a parser that will construct the parse tree
 *      and if no parse errors were found, print it.  The parser should 
 *      accept the language as described in specification, and as augmented 
 *      in the pp2 handout.
 */

%{

/* Just like lex, the text within this first region delimited by %{ and %}
 * is assumed to be C/C++ code and will be copied verbatim to the y.tab.c
 * file ahead of the definitions of the yyparse() function. Add other header
 * file inclusions or C++ variable declarations/prototypes that are needed
 * by your code here.
 */
#include "scanner.h" // for yylex
#include "parser.h"
#include "errors.h"

void yyerror(const char *msg); // standard error-handling routine

%}

/* The section before the first %% is the Definitions section of the yacc
 * input file. Here is where you declare tokens and types, add precedence
 * and associativity options, and so on.
 */
 
/* yylval 
 * ------
 * Here we define the type of the yylval global variable that is used by
 * the scanner to store attibute information about the token just scanned
 * and thus communicate that information to the parser. 
 *
 * pp2: You will need to add new fields to this union as you add different 
 *      attributes to your non-terminal symbols.
 */
%union {
    int integerConstant;
    bool boolConstant;
    char *stringConstant;
    double doubleConstant;
    char identifier[MaxIdentLen+1]; // +1 for terminating null
    Decl *decl;
    List<Decl*> *declList;

    List<VarDecl*> *varList;
    VarDecl *varDecl;
    List<VarDecl*> *vars;
    VarDecl *var;

    Type *type;
    NamedType *typeNamed;

    FnDecl *functDecl;
    FnDecl *funct;

    List<VarDecl*> *formals;

    ClassDecl *classDecl;
    NamedType *extends;
    List<NamedType*> *implements;

    List<NamedType*> *idents;

    Decl *field;
    List<Decl*> *fieldList; 

    InterfaceDecl *interDecl;
    
    Decl *prototype;
    List<Decl*> *protoList;

    Stmt *stmt;
    StmtBlock *stmtBlock;
    List<Stmt*> *stmtList;

    IfStmt *ifStmt;
    WhileStmt *whileStmt;
    ForStmt *forStmt;
    ReturnStmt *returnStmt;
    BreakStmt *breakStmt;
    PrintStmt *printStmt;
    SwitchStmt *switchStmt;
    CaseStmt *caseStmt;
    List<CaseStmt*> *caseStmtList;
    DefaultStmt *defaultStmt;

    Expr *expr;
    List<Expr*> *exprs;
    Expr *opExpr;

    LValue *lValue;
    Call *call;
    List<Expr*> *actuals;
    Expr *constant;

}


/* Tokens
 * ------
 * Here we tell yacc about all the token types that we are using.
 * Yacc will assign unique numbers to these and export the #define
 * in the generated y.tab.h header file.
 */
%token   T_Void T_Bool T_Int T_Double T_String T_Class 
%token   T_LessEqual T_GreaterEqual T_Equal T_NotEqual T_Dims
%token   T_And T_Or T_Null T_Extends T_This T_Interface T_Implements
%token   T_While T_For T_If T_Else T_Return T_Break
%token   T_New T_NewArray T_Print T_ReadInteger T_ReadLine

%token   <identifier> T_Identifier
%token   <stringConstant> T_StringConstant 
%token   <integerConstant> T_IntConstant
%token   <doubleConstant> T_DoubleConstant
%token   <boolConstant> T_BoolConstant

%token   T_Increm T_Decrem T_Switch T_Case T_Default


/* Non-terminal types
 * ------------------
 * In order for yacc to assign/access the correct field of $$, $1, we
 * must to declare which field is appropriate for the non-terminal.
 * As an example, this first type declaration establishes that the DeclList
 * non-terminal uses the field named "declList" in the yylval union. This
 * means that when we are setting $$ for a reduction for DeclList ore reading
 * $n which corresponds to a DeclList nonterminal we are accessing the field
 * of the union named "declList" which is of type List<Decl*>.
 * pp2: You'll need to add many of these of your own.
 */
%type <declList>  DeclList 
%type <decl>      Decl

%type <varList> VarList
%type <varDecl> VarDecl
%type <vars> Vars
%type <var> Var

%type <type> Type
%type <typeNamed> TypeNamed

%type <functDecl> FunctDecl
%type <funct> Funct

%type <formals> Formals

%type <classDecl> ClassDecl
%type <extends> Extends
%type <implements> Implements

%type <idents> Idents

%type <fieldList> FieldList
%type <field> Field

%type <interDecl> InterDecl

%type <protoList> ProtoList
%type <prototype> Prototype

%type <stmtBlock> StmtBlock
%type <stmtList> StmtList
%type <stmt> Stmt
%type <opExpr> OpExpr

%type <ifStmt> IfStmt
%type <whileStmt> WhileStmt
%type <forStmt> ForStmt
%type <returnStmt> ReturnStmt
%type <breakStmt> BreakStmt
%type <printStmt> PrintStmt
%type <switchStmt> SwitchStmt
%type <caseStmt> CaseStmt
%type <caseStmtList> CaseStmtList;
%type <defaultStmt> DefaultStmt;

%type <exprs> Exprs
%type <expr> Expr

%type <lValue> LValue
%type <call> Call
%type <actuals> Actuals
%type <constant> Constant

/* Precendence and associativity */
%right '='
%left T_Or
%left T_And
%nonassoc T_Equal T_NotEqual
%nonassoc '<' '>' T_GreaterEqual T_LessEqual
%left '+' '-'
%left '*' '/' '%'
%left T_Increm T_Decrem
%right '!' UMIN
%nonassoc '.' '[' 
%nonassoc IF
%nonassoc T_Else
%nonassoc NOCASE
%nonassoc NODEFAULT
%nonassoc CASE
%nonassoc DEFAULT

%%
/* Rules
 * -----
 * All productions and actions should be placed between the start and stop
 * %% markers which delimit the Rules section.
	 
 */
Program   :    DeclList            { 
                                      @1; 
                                      /* pp2: The @1 is needed to convince 
                                       * yacc to set up yylloc. You can remove 
                                       * it once you have other uses of @n*/
                                      Program *program = new Program($1);
                                      // if no errors, advance to next phase
                                      if (ReportError::NumErrors() == 0) 
                                          program->Print(0);
                                    }
          ;

DeclList  :    DeclList Decl        { ($$=$1)->Append($2); }
          |    Decl                 { ($$ = new List<Decl*>)->Append($1); }
          ;

Decl      :    VarDecl                 { $$ = $1; } 
          |    FunctDecl               { $$ = $1; }
          |    ClassDecl               { $$ = $1; } 
          |    InterDecl               { $$ = $1; } 
          ;

VarDecl   :    Var ';'                 { $$ = $1; } 
          ;

Var       :    Type T_Identifier               { Identifier *i = new Identifier(@2, $2);
                                                 $$ = new VarDecl(i, $1); }
          ;

Type      :    T_Int                   { $$ = Type::intType; }
          |    T_Double                { $$ = Type::doubleType; }
          |    T_Bool                  { $$ = Type::boolType; } 
          |    T_String                { $$ = Type::stringType; }
          |    T_Void                  { $$ = Type::voidType; }
          |    Type T_Dims             { $$ = new ArrayType(Join(@1, @2), $1); }
          |    TypeNamed               { $$ = $1; }
          ;

TypeNamed :    T_Identifier            { Identifier *i = new Identifier(@1, $1);
                                         $$ = new NamedType(i); }
          ;

FunctDecl :    Funct StmtBlock         { FnDecl *f = $1;
                                         f->SetFunctionBody($2); }
          ;

Funct     :    Type T_Identifier '(' Formals ')'   { Identifier *i = new Identifier(@2, $2);
                                                     $$ = new FnDecl(i, $1, $4); }
          ;

Formals   :    Vars                    { $$ = $1; }
          |    /*empty*/               { $$ = new List<VarDecl*>; }
          ;

Vars      :    Vars ',' Var            { ($$ = $1)->Append($3); }
          |    Var                     { ($$ = new List<VarDecl*>)->Append($1); }
          ;

ClassDecl :    T_Class T_Identifier Extends Implements '{' FieldList '}'   { Identifier *i = new Identifier(@2, $2);
                                                                             $$ = new ClassDecl(i, $3, $4, $6); }
          |    T_Class T_Identifier Extends Implements '{' '}'             { Identifier *i = new Identifier(@2, $2);
                                                                             $$ = new ClassDecl(i, $3, $4, new List<Decl*>); }
          ;

Extends   :    T_Extends T_Identifier          { Identifier *i = new Identifier(@2, $2);
                                                 $$ = new NamedType(i); } 
          |    /* empty */                     { $$ = NULL; }
          ;

Implements :    T_Implements Idents            { $$ = $2; }
           |    /* empty */                    { $$ = new List<NamedType*>; }
           ;

Idents    :    Idents ',' T_Identifier         { Identifier *i = new Identifier(@3, $3);
                                                 NamedType *n = new NamedType(i);
                                                 ($$ = $1)->Append(n); }
          |    T_Identifier                    { Identifier *i = new Identifier(@1, $1);
                                                 NamedType *n = new NamedType(i);
                                                 ($$ = new List<NamedType*>)->Append(n); }
          ;

FieldList :    FieldList Field                 { ($$ = $1)->Append($2); }
          |    Field                           { ($$ = new List<Decl*>)->Append($1); }
          ;

Field     :    VarDecl                         { $$ = $1; }
          |    FunctDecl                       { $$ = $1; }
          ;

InterDecl :    T_Interface T_Identifier '{' ProtoList '}'      { Identifier *i = new Identifier(@2, $2);
                                                                 $$ = new InterfaceDecl(i, $4); }
          |    T_Interface T_Identifier '{' '}'                { Identifier *i = new Identifier(@2, $2);
                                                                 $$ = new InterfaceDecl(i, new List<Decl*>); }
          ;

ProtoList :    ProtoList Prototype             { ($$ = $1)->Append($2); }
          |    Prototype                       { ($$ = new List<Decl*>)->Append($1); }
          ;
    
Prototype :    Funct ';'                       { $$ = $1; }
          ;

StmtBlock :    '{' VarList StmtList '}'        { $$ = new StmtBlock($2, $3); }
          |    '{' VarList '}'                 { $$ = new StmtBlock($2, new List<Stmt*>);}
          |    '{' StmtList '}'                { $$ = new StmtBlock(new List<VarDecl*>, $2); }
          |    '{' /* empty */ '}'             { $$ = new StmtBlock(new List<VarDecl*>, new List<Stmt*>); }
          ;

VarList   :    VarList VarDecl                 { ($$ = $1)->Append($2); } 
          |    VarDecl                         { ($$ = new List<VarDecl*>)->Append($1); }
          ;

StmtList  :    StmtList Stmt                   { ($$ = $1)->Append($2); }
          |    Stmt                            { ($$ = new List<Stmt*>)->Append($1); }
          ;

Stmt      :    OpExpr ';'                      { $$ = $1; }
          |    IfStmt                          { $$ = $1; }
          |    WhileStmt                       { $$ = $1; }
          |    ForStmt                         { $$ = $1; }
          |    BreakStmt                       { $$ = $1; }
          |    ReturnStmt                      { $$ = $1; }
          |    PrintStmt                       { $$ = $1; }
          |    StmtBlock                       { $$ = $1; }
          |    SwitchStmt                      { $$ = $1; }
          ;

OpExpr    :    Expr                            { $$ = $1; }
          |    /* empty */                     { $$ = new EmptyExpr(); }
          ;

IfStmt    :    T_If '(' Expr ')' Stmt %prec IF         { $$ = new IfStmt($3, $5, NULL); }
          |    T_If '(' Expr ')' Stmt T_Else Stmt      { $$ = new IfStmt($3, $5, $7); }
          ;

WhileStmt :    T_While '(' Expr ')' Stmt               { $$ = new WhileStmt($3, $5); }
          ;

ForStmt   :    T_For '(' OpExpr ';' Expr ';' OpExpr ')' Stmt       { $$ = new ForStmt($3, $5, $7, $9); }
          ;

ReturnStmt :    T_Return ';'                   { $$ = new ReturnStmt(@1, new EmptyExpr()); }
           |    T_Return Expr ';'              { $$ = new ReturnStmt(@2, $2); }
           ;

BreakStmt  :    T_Break ';'                    { $$ = new BreakStmt(@1); }
           ;

PrintStmt  :    T_Print '(' Exprs ')' ';'      { $$ = new PrintStmt($3); }
           ;

SwitchStmt :    T_Switch '(' Expr ')' '{' CaseStmtList DefaultStmt '}' { $$ = new SwitchStmt($3, $6, $7);}
           ;

CaseStmtList :  CaseStmtList CaseStmt   { ($$ = $1)->Append($2);}
            |   CaseStmt                { ($$ = new List<CaseStmt*>)->Append($1);}
            ;

CaseStmt   :    T_Case T_IntConstant ':' StmtList %prec CASE  { IntConstant *i =  new IntConstant(@2, $2);
                                                                  $$ = new CaseStmt(i,$4);}
           |    T_Case T_IntConstant ':' %prec NOCASE   { IntConstant *i = new IntConstant(@2, $2);
                                                          $$ = new CaseStmt(i, new List<Stmt*>);}
           ;

DefaultStmt:    T_Default ':' StmtList %prec DEFAULT    { $$ = new DefaultStmt($3);}
           |    T_Default ':' %prec NODEFAULT           { $$ = new DefaultStmt(new List<Stmt*>);}


Exprs      :    Exprs ',' Expr                 { ($$=$1)->Append($3); }
           |    Expr                           { ($$ = new List<Expr*>)->Append($1); }
           ;

Expr       :    Constant                               { $$ = $1; }
           |    LValue                                 { $$ = $1; }
           |    LValue '=' Expr                        { Operator *o = new Operator(@2, "=");
                                                         $$ = new AssignExpr($1, o, $3); }
           |    T_This                                 { $$ = new This(@1); }
           |    Call                                   { $$ = $1; }
           |    '(' Expr ')'                           { $$ = $2; }
           |    Expr '+' Expr                          { Operator *o = new Operator(@2, "+");
                                                         $$ = new ArithmeticExpr($1, o, $3); }
           |    Expr '-' Expr                          { Operator *o = new Operator(@2, "-");
                                                         $$ = new ArithmeticExpr($1, o, $3); }
           |    Expr '*' Expr                          { Operator *o = new Operator(@2, "*");
                                                         $$ = new ArithmeticExpr($1, o, $3); }
           |    Expr '/' Expr                          { Operator *o = new Operator(@2, "/");
                                                         $$ = new ArithmeticExpr($1, o, $3); }
           |    Expr '%' Expr                          { Operator *o = new Operator(@2, "%");
                                                         $$ = new ArithmeticExpr($1, o, $3); }
           |    '-' Expr %prec UMIN                    { Operator *o = new Operator(@1, "-");
                                                         $$ = new ArithmeticExpr(o, $2); }
           |    Expr '<' Expr                          { Operator *o = new Operator(@2, "<");
                                                         $$ = new RelationalExpr($1, o, $3); }
           |    Expr T_LessEqual Expr                  { Operator *o = new Operator(@2, "<=");
                                                         $$ = new RelationalExpr($1, o, $3); }
           |    Expr '>' Expr                          { Operator *o = new Operator(@2, ">");
                                                         $$ = new RelationalExpr($1, o, $3); }
           |    Expr T_GreaterEqual Expr               { Operator *o = new Operator(@2, ">=");
                                                         $$ = new RelationalExpr($1, o, $3); }
           |    Expr T_Equal Expr                      { Operator *o = new Operator(@2, "==");
                                                         $$ = new EqualityExpr($1, o, $3); }
           |    Expr T_NotEqual Expr                   { Operator *o = new Operator(@2, "!=");
                                                         $$ = new EqualityExpr($1, o, $3); }
           |    Expr T_And Expr                        { Operator *o = new Operator(@2, "&&");
                                                         $$ = new LogicalExpr($1, o, $3); }
           |    Expr T_Or Expr                         { Operator *o = new Operator(@2, "||");
                                                         $$ = new LogicalExpr($1, o, $3); }
           |    '!' Expr                               { Operator *o = new Operator(@1, "!");
                                                         $$ = new LogicalExpr(o, $2); }
           |    T_ReadInteger '(' /* empty */ ')'      { $$ = new ReadIntegerExpr(Join(@1, @3)); }
           |    T_ReadLine '(' /* empty */ ')'         { $$ = new ReadLineExpr(Join(@1, @3)); }
           |    T_New '(' TypeNamed ')'                { $$ = new NewExpr(Join(@1, @3), $3); }
           |    T_NewArray '(' Expr ',' Type ')'       { $$ = new NewArrayExpr(Join(@1, @6), $3, $5); }
           |    LValue T_Increm                               {Operator *o = new Operator(@2, "++");
                                                             $$ = new PostFixExpr($1, o);}
           |    LValue T_Decrem                               {Operator *o = new Operator(@2, "--");
                                                             $$ = new PostFixExpr($1, o);}
           ;

LValue     :    T_Identifier                       { Identifier *i = new Identifier(@1, $1);
                                                     $$ = new FieldAccess(NULL, i); }
           |    Expr '.' T_Identifier              { Identifier *i = new Identifier(@3, $3);
                                                     $$ = new FieldAccess($1, i); }
           |    Expr '[' Expr ']'                  { $$ = new ArrayAccess(Join(@1, @4), $1, $3); }
           ;

Call       :    T_Identifier '(' Actuals ')'                   { Identifier *i = new Identifier(@1, $1);
                                                                 $$ = new Call(Join(@1, @4), NULL, i, $3); }
           |    Expr '.' T_Identifier '(' Actuals ')'          { Identifier *i = new Identifier(@3, $3);
                                                                 $$ = new Call(Join(@1, @6), $1, i, $5); }
           ;

Actuals    :    Exprs                  { $$ = $1; }
           |    /*empty*/              { $$ = new List<Expr*>; }
           ;

Constant   :    T_IntConstant          { $$ = new IntConstant(@1, $1); }
           |    T_DoubleConstant       { $$ = new DoubleConstant(@1, $1); }
           |    T_BoolConstant         { $$ = new BoolConstant(@1, $1); }
           |    T_StringConstant       { $$ = new StringConstant(@1, $1); }
           |    T_Null                 { $$ = new NullConstant(@1); }
           ;

%%

/* The closing %% above marks the end of the Rules section and the beginning
 * of the User Subroutines section. All text from here to the end of the
 * file is copied verbatim to the end of the generated y.tab.c file.
 * This section is where you put definitions of helper functions.
 */

/* Function: InitParser
 * --------------------
 * This function will be called before any calls to yyparse().  It is designed
 * to give you an opportunity to do anything that must be done to initialize
 * the parser (set global variables, configure starting state, etc.). One
 * thing it already does for you is assign the value of the global variable
 * yydebug that controls whether yacc prints debugging information about
 * parser actions (shift/reduce) and contents of state stack during parser.
 * If set to false, no information is printed. Setting it to true will give
 * you a running trail that might be helpful when debugging your parser.
 * Please be sure the variable is set to false when submitting your final
 * version.
 */
void InitParser()
{
   PrintDebug("parser", "Initializing parser");
   yydebug = false;
}
