/* File:  scanner.l
 * ----------------
 * Lex inupt file to generate the scanner for the compiler.
 */

%{

/* The text within this first region delimited by %{ and %} is assumed to
 * be C/C++ code and will be copied verbatim to the lex.yy.c file ahead
 * of the definitions of the yylex() function. Add other header file inclusions
 * or C++ variable declarations/prototypes that are needed by your code here.
 */

#include <string.h>
#include <sstream>
#include <iostream>
#include "scanner.h"
#include "utility.h" // for PrintDebug()
#include "errors.h"
using namespace std;

/* Global variable: yylval
 * -----------------------
 * This global variable is how we get attribute information about the token
 * just scanned to the client. The scanner sets the global variable
 * appropriately and since it's global the client can just read it.  In the
 * future, this variable will be declared for us in the y.tab.c file
 * produced by Yacc, but for now, we declare it manually.
 */
YYSTYPE yylval;  // manually declared for pp1, later Yacc provides
TokenType token;

/* Global variable: yylloc
 * -----------------------
 * This global variable is how we get position information about the token
 * just scanned to the client. (Operates similarly to yylval above)
 */
struct yyltype yylloc; // manually dclared for pp1, later Yacc provides

void intConstant(char *);
void doubleConstant(char *);
void booleanexp(char *);
void validString(char *);
void invalidString(char *);
void newtab();
TokenType keyword(char *);
TokenType Operators(char *);
char punc(char *);
void newSpace();

/* Macro: YY_USER_ACTION 
 * ---------------------
 * This flex built-in macro can be defined to provide an action which is
 * always executed prior to any matched rule's action. Basically, it is
 * a way of having a piece of code common to all actions factored out to
 * this routine.  We already defined it for you and left the empty
 * function DoBeforeEachAction ready for your use as needed. It will
 * be called once for each pattern scanned from the file, before
 * executing its action.
 */
static void DoBeforeEachAction(); 
#define YY_USER_ACTION DoBeforeEachAction();

%}

 /* The section before the first %% is the Definitions section of the lex
  * input file. Here is where you set options for the scanner, define lex
  * states, and can set up definitions to give names to regular expressions
  * as a simple substitution mechanism that allows for more readable
  * entries in the Rules section later. 
  */

%%             /* BEGIN RULES SECTION */
 /* All patterns and actions should be placed between the start and stop
  * %% markers which delimit the Rules section. 
  */ 

\"[^\"\n]*\" {
  
  validString(yytext);
  return T_StringConstant;
}

([0-9]+)|(0[xX][0-9A-Fa-f]+) {

  intConstant(yytext);
  return T_IntConstant;
}

true|false {

 booleanexp(yytext);
 return T_BoolConstant;
}

[a-zA-Z][a-zA-Z0-9_]* {
  
  if (strlen(yytext) > 31) {
    ReportError::LongIdentifier(&yylloc, yytext);
  }
  return keyword(yytext);
}

[0-9]+\.[0-9]*((e|E)(\+|\-)?[0-9]+)?  {

 doubleConstant(yytext);
 return T_DoubleConstant;
}

(==|>=|<=|!=|[||]{2}|[&&]{2}) {

  return Operators(yytext);  
}

[-]|[+]|[*]|[;]|[[]|[]]|[(]|[)]|[!]|[,]|[.]|[%]|[<]|[>]|[=]|[{]|[}]|[\/] {

  return punc(yytext);
}

\"[^\"]*\n {
  ReportError::UntermString(&yylloc, yytext);
  for(int i = 0;i <strlen(yytext);++i)
  {
    if(yytext[i] == '\n')
    yylloc.first_line++;
  }
  yylloc.first_column = 0;
  yylloc.last_column = 0;
}

\n {
  yylloc.first_line++;
  yylloc.first_column = 0;
  yylloc.last_column = 0;
}

\t newtab();

[ ] newSpace();

. {
  char var = yytext[0];
  ReportError::UnrecogChar(&yylloc, var);
}

%%
/* The closing %% above marks the end of the Rules section and the beginning
 * of the User Subroutines section. All text from here to the end of the
 * file is copied verbatim to the end of the generated lex.yy.c file.
 * This section is where you put definitions of helper functions.
 */


/* Function: InitScanner
 * ---------------------
 * This function will be called before any calls to yylex().  It is designed
 * to give you an opportunity to do anything that must be done to initialize
 * the scanner (set global variables, configure starting state, etc.). One
 * thing it already does for you is assign the value of the global variable
 * yy_flex_debug that controls whether flex prints debugging information
 * about each token and what rule was matched. If set to false, no information
 * is printed. Setting it to true will give you a running trail that might
 * be helpful when debugging your scanner. Please be sure the variable is
 * set to false when submitting your final version.
 */

void InitScanner()
{
  //"[^(\")]*\"
  //\"[^(\")]*\n
    PrintDebug("lex", "Initializing scanner");
    yy_flex_debug = false;
    yytext = "";
    yylloc.first_line = 1;
    yylloc.first_column = 0;
    yylloc.last_column = 0;
}

char punc(char *c)
{
  yylloc.first_column = yylloc.last_column + 1;
  yylloc.last_column = yylloc.first_column + yyleng - 1;
  char var;
  var = c[0];
  return var;
}

TokenType keyword(char *c)
{
  yylloc.first_column = yylloc.last_column + 1;
  yylloc.last_column = yylloc.first_column + yyleng - 1;
  if(strcmp(c, "void") == 0){
    return T_Void;
  }
  else if (strcmp(c, "int") == 0){
    return T_Int;
  }
  else if (strcmp(c, "double") == 0){
    return T_Double;
  }
  else if (strcmp(c, "bool") == 0){
    return T_Bool;
  }
  else if (strcmp(c, "string") == 0){
    return T_String;
  }
  else if (strcmp(c, "class") == 0){
    return T_Class;
  }
  else if (strcmp(c, "interface") == 0){
    return T_Interface;
  }
  else if (strcmp(c, "null") == 0){
    return T_Null;
  }
  else if (strcmp(c, "this") == 0){
    return T_This;
  }
  else if (strcmp(c, "extends") == 0){
    return T_Extends;
  }
  else if (strcmp(c, "implements") == 0){
    return T_Implements;
  }
  else if (strcmp(c, "for") == 0){
    return T_For;
  }
  else if (strcmp(c, "while") == 0){
    return T_While;
  }
  else if (strcmp(c, "if") == 0){
    return T_If;
  }
  else if (strcmp(c, "else") == 0){
    return T_Else;
  }
  else if (strcmp(c, "return") == 0){
    return T_Return;
  }
  else if (strcmp(c, "break") == 0){
    return T_Break;
  }
  else if (strcmp(c, "New") == 0){
    return T_New;
  }
  else if (strcmp(c, "NewArray") == 0){
    return T_NewArray;
  }
  strncpy(yylval.identifier, c,31);
  return T_Identifier;
}

TokenType Operators(char *c)
{
  yylloc.first_column = yylloc.last_column + 1;
  yylloc.last_column = yylloc.first_column + yyleng - 1;  
  if(strcmp(c, "==") == 0) return T_Equal;
  else if (strcmp(c,">=") == 0) return T_GreaterEqual;
  else if (strcmp(c,"<=") == 0) return T_LessEqual;
  else if (strcmp(c,"!=") == 0) return T_NotEqual;
  else if (strcmp(c, "&&") == 0) return T_And;
  else return T_Or;
}

void validString(char *c)
{
  yylval.stringConstant = c;
  yylloc.first_column = yylloc.last_column + 1;
  yylloc.last_column = yylloc.first_column + yyleng - 1;
  return;
}

void intConstant(char *c)
{
  if(c[1] == 'x' || c[1] == 'X')
  {
    long num = strtol(c,NULL,16);
    yylval.integerConstant = (int) num;
  }
  else
  {
     yylval.integerConstant = atoi(c);
  }
  yylloc.first_column = yylloc.last_column + 1;
  yylloc.last_column = yylloc.first_column + yyleng -1;
  return;

}

void doubleConstant(char *c)
{
  string str(c);
  istringstream os(str);
  os >> yylval.doubleConstant;
  yylloc.first_column = yylloc.last_column + 1;
  yylloc.last_column = yylloc.first_column + yyleng - 1;
}

void booleanexp(char *c)
{
  if(strcmp(c, "true") == 0)
  {
    yylval.boolConstant = true;
  }
  else
  {
    yylval.boolConstant = false;
  } 
  yylloc.first_column = yylloc.last_column + 1;
  yylloc.last_column = yylloc.first_column + yyleng - 1;
  return;
}

void newtab()
{
  yylloc.first_column = yylloc.last_column + 8;
  yylloc.last_column = yylloc.first_column;
}

void newSpace()
{
  yylloc.first_column = yylloc.last_column + 1;
  yylloc.last_column = yylloc.first_column;
}



/* Function: DoBeforeEachAction()
 * ------------------------------
 * This function is installed as the YY_USER_ACTION. This is a place
 * to group code common to all actions.
 */
static void DoBeforeEachAction()
{
}
