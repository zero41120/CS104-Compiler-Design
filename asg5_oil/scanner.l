/* $Id: lexer.l,v 1.6 2016-10-20 13:48:57-07 - - $ */

%{

#include "lyutils.h"

#define YY_USER_ACTION  { lexer::advance(); }

int yylval_token (int symbol) {
   yylval = new astree (symbol, lexer::lloc, yytext);
   if (tok_file != nullptr) {
      fprintf(tok_file, "%5zu %2zu.%03zu %4u  %-15s (%s)\n",
         yylval->lloc.filenr,
         yylval->lloc.linenr,
         yylval->lloc.offset,
         yylval->symbol,
         parser::get_tname(yylval->symbol),
         yylval->lexinfo->c_str()
      );
   }
   return symbol;
}

%}

%option 8bit
%option debug
%option nodefault
%option noinput
%option nounput
%option noyywrap
%option warn
/*%option verbose*/

LETTER         [A-Za-z_]
DIGIT          [0-9]
IDENT          ({LETTER}({LETTER}|{DIGIT})*)
NUMBER         ({DIGIT}+)
CHARCON        ('([^\\'\n]|\\[\\'"0nt])')
STRINGCON      (\"([^\\"\n]|\\[\\'"0nt])*\")
TOK_COMP       ({TOK_EQ}|{TOK_NE}|<|{TOK_LE}|{TOK_GE}|>)
TOK_OPER       (\+|\-|\/|\*|\%|\=)
TOK_BINOP      ({TOK_COMP}|{TOK_OPER})

%%

"#".*           { lexer::include(); }
[ \t]+          { }
\n              { lexer::newline(); }


void             { return yylval_token (TOK_VOID); }
char             { return yylval_token (TOK_CHAR); }
int              { return yylval_token (TOK_INT); }
string           { return yylval_token (TOK_STRING); }
if               { return yylval_token (TOK_IF); }
else             { return yylval_token (TOK_ELSE); }
while            { return yylval_token (TOK_WHILE); }
return           { return yylval_token (TOK_RETURN); }
struct           { return yylval_token (TOK_STRUCT); }
null             { return yylval_token (TOK_NULL); }
new              { return yylval_token (TOK_NEW); }
bool             { return yylval_token (TOK_BOOL); }
true             { return yylval_token (TOK_TRUE); }
false            { return yylval_token (TOK_FALSE); }
"[]"             { return yylval_token (TOK_ARRAY); }
"=="             { return yylval_token (TOK_EQ); }
"!="             { return yylval_token (TOK_NE); }
"<"              { return yylval_token (TOK_LT); }
"<="             { return yylval_token (TOK_LE); }
">"              { return yylval_token (TOK_GT); }
">="             { return yylval_token (TOK_GE); }
"!"              { return yylval_token ('!'); }
"+"              { return yylval_token ('+'); }
"-"              { return yylval_token ('-'); }
"*"              { return yylval_token ('*'); }
"/"              { return yylval_token ('/'); }
"^"              { return yylval_token ('^'); }
"("              { return yylval_token ('('); }
")"              { return yylval_token (')'); }
"["              { return yylval_token ('['); }
"]"              { return yylval_token (']'); }
"."              { return yylval_token ('.'); }
","              { return yylval_token (','); }
"{"              { return yylval_token ('{'); }
"}"              { return yylval_token ('}'); }
"="              { return yylval_token ('='); }
";"              { return yylval_token (';'); }

{NUMBER}        { return yylval_token (TOK_INTCON); }
{IDENT}         { return yylval_token (TOK_IDENT); }
{STRINGCON}    { return yylval_token (TOK_STRINGCON); }
{CHARCON}      { return yylval_token (TOK_CHARCON); }

.               { lexer::badchar (*yytext); }

%%

