// $Id: parser.y,v 1.14 2016-10-06 16:26:41-07 - - $

%{

// File from e08, this is not the scanner project parser.y

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "astree.h"
#include "lyutils.h"

%}

%debug
%defines
%error-verbose
%token-table
%verbose

%start  start

%token ROOT NUMBER 
%token TOK_VOID TOK_CHAR TOK_INT TOK_STRING
%token TOK_IF TOK_ELSE TOK_WHILE TOK_RETURN TOK_STRUCT
%token TOK_NULL TOK_NEW TOK_ARRAY TOK_BOOL TOK_TRUE TOK_FALSE
%token TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%token TOK_IDENT TOK_INTCON TOK_CHARCON TOK_STRINGCON

%token TOK_BLOCK TOK_CALL TOK_IFELSE TOK_INITDECL
%token TOK_POS TOK_NEG TOK_NEWARRAY TOK_TYPEID TOK_FIELD
%token TOK_ORD TOK_CHR TOK_COMP

%token TOK_DECLID TOK_INDEX TOK_NEWSTRING TOK_RETURNVOID 
%token TOK_VARDECL TOK_FUNCTION TOK_PARAMLIST TOK_PROTOTYPE


%right  '='
%left   TOK_COMP
%left   '+' '-'
%left   '*' '/'
%right  '^'
%right  POS NEG


%%

start       : program           { $$ = $1; }
            ;

program     : program structdef { $$ = $1->adopt ($2); }
            | program function  { $$ = $1->adopt ($2); }
            | program statement { $$ = $1->adopt ($2); }
            | program error '}' { $$ = $1; }
            | program error ';' { $$ = $1; }
            |                   { $$ = parser::root = 
                                  new astree (ROOT, {0, 0, 0}, ""); }
            ;

structhead  : '{' fielddecl         { destroy ($1); $$ = $2; }
            | structhead fielddecl   { $$ = $1->adopt($2);}
            ;

structdef   : TOK_STRUCT TOK_IDENT structhead '}' {
               destroy($4);
               $2 = $2->set_sym(TOK_TYPEID);
               $$ = $1->adopt($2, $3);
            }
            | TOK_STRUCT TOK_IDENT '{' '}' { 
               destroy($3, $4);
               $2 = $2->set_sym(TOK_TYPEID);
               $$ = $1->adopt($2);
            }
            ;

fielddecl   : basetype TOK_IDENT ';' { 
               destroy($3);
               $2 = $2->set_sym(TOK_FIELD);
               $$ = $1->adopt($2);
            }
            | basetype TOK_ARRAY TOK_IDENT ';' {
               destroy($4);
               $3 = $3->set_sym(TOK_FIELD);
               $$ = $2->adopt($1, $3);
            }
            ;

basetype    : TOK_VOID        { $$ = $1; }
            | TOK_BOOL        { $$ = $1; }
            | TOK_CHAR        { $$ = $1; }
            | TOK_INT         { $$ = $1; }
            | TOK_STRING      { $$ = $1; }
            | TOK_IDENT       { $$ = $1->set_sym(TOK_TYPEID); }
            ;

funchead    : '(' identdecl         {
                  $$ = $1->adopt_sym(TOK_PARAMLIST, $2);
            }
            | funchead ',' identdecl    { $$ = $1->adopt($3);}
            ;

function    : identdecl funchead ')' block {
               destroy($3);
               $$ = (new astree(TOK_FUNCTION, $1->lloc, ""))
               ->adopt($1, $2, $4);
            }
            | identdecl funchead ')' ';' {
               destroy($4, $3);
               $$ = (new astree(TOK_PROTOTYPE, $1->lloc, ""))
               ->adopt($1, $2);   
            }
            | identdecl '(' ')' block { 
               destroy($3);
               $$ = (new astree(TOK_FUNCTION, $1->lloc, ""))
               ->adopt($1, $2, $4);   
            }
            | identdecl '(' ')' ';' {
               destroy($3, $4);
               $$ = (new astree(TOK_PROTOTYPE, $1->lloc, ""))
               ->adopt($1, $2);   
                  
            }
            ;

identdecl   : basetype TOK_IDENT {
               $2 = $2->set_sym(TOK_DECLID);
               $$ = $1->adopt($2);
            }
            | basetype TOK_ARRAY TOK_IDENT {
               $3 = $3->set_sym(TOK_DECLID);
               $$ = $2->adopt($1, $3);
            }
            ;

blockhead   : blockhead statement   { $$ = $1->adopt($2); }
            | '{' statement         { $$ = $1->adopt($2); }
            ;

block       : blockhead '}' {
               destroy($2);
               $$ = $1->set_sym(TOK_BLOCK);
            }
            | '{' '}' { 
               destroy($2);
               $$ = $1->set_sym(TOK_BLOCK);
            } 
            ;

statement   : block     { $$ = $1; }
            | vardecl   { $$ = $1; }
            | while     { $$ = $1; }
            | ifelse    { $$ = $1; }
            | return    { $$ = $1; }
            | expr ';'  {
               destroy($2);
               $$ = $1;
            }
            | ';'       { $$ = $1; }
            ;

vardecl     : identdecl '=' expr ';' {
               destroy($4);
               $$ = $2->adopt_sym(TOK_VARDECL, $1, $3);
            }
            ;

while       : TOK_WHILE '(' expr ')' statement {
               destroy($2, $4);
               $$ = $1->adopt($3, $5);
            }
            ;

ifelse      : TOK_IF '(' expr ')' statement TOK_ELSE statement {
               destroy($2, $4, $6);
               $1 = $1->set_sym(TOK_IFELSE);
               $$ = $1->adopt($3, $5, $7);
            }
            | TOK_IF '(' expr ')' statement %prec TOK_IF {
               destroy($2, $4);
               $$ = $1->adopt($3, $5);
            }
            ;

return      : TOK_RETURN ';' {
               destroy($2);
               $$ = $1->set_sym( TOK_RETURNVOID);
            }
            | TOK_RETURN expr ';'{
               destroy($3);
               $$ = $1->adopt($2);
            }
            ;

expr        : binop        { $$ = $1; }
            | unop         { $$ = $1; }
            | allocator    { $$ = $1; }
            | call         { $$ = $1; }
            | '(' expr ')' {
               destroy($1, $3);
               $$ = $2;
            }
            | variable {$$ = $1; }
            | constant {$$ = $1; }
            ;

binop       : expr '+' expr      { $$ = $2->adopt($1, $3); }
            | expr '-' expr      { $$ = $2->adopt($1, $3); }
            | expr '*' expr      { $$ = $2->adopt($1, $3); }
            | expr '/' expr      { $$ = $2->adopt($1, $3); }
            | expr TOK_LT expr   { $$ = $2->adopt($1, $3); }
            | expr TOK_GT expr   { $$ = $2->adopt($1, $3); }
            | expr TOK_LE expr   { $$ = $2->adopt($1, $3); }
            | expr TOK_GE expr   { $$ = $2->adopt($1, $3); }
            | expr '=' expr      { $$ = $2->adopt($1, $3); }
            | expr TOK_EQ expr   { $$ = $2->adopt($1, $3); }
            | expr TOK_NE expr   { $$ = $2->adopt($1, $3); }
            | expr '%' expr      { $$ = $2->adopt($1, $3); }
            ;

unop        : '+' expr        { $$ = $1->adopt_sym(TOK_POS, $2); }
            | '-' expr        { $$ = $1->adopt_sym(TOK_NEG, $2); }
            | '!' expr        { $$ = $1->adopt($2); }
            | TOK_CHR expr    { $$ = $1->adopt($2); }
            | TOK_ORD expr    { $$ = $1->adopt($2); }
            ;

allocator   : TOK_NEW TOK_IDENT '(' ')' { 
               destroy($3, $4);
               $2 = $2->set_sym(TOK_TYPEID);
               $$ = $1->adopt($2);
            }
            | TOK_NEW TOK_STRING '(' expr ')' {
               destroy($3, $5);
               $$ = $1->adopt_sym(TOK_NEWSTRING, $4);
            }
            | TOK_NEW basetype '[' expr ']' {
               destroy($3); destroy($5);
               $$ = $1->adopt_sym(TOK_NEWARRAY, $2, $4);
            }
            ;

callexprs   : TOK_IDENT '(' expr    {
               $$ = $2->adopt_sym(TOK_CALL, $1, $3);
            }
            | callexprs ',' expr    {
               destroy($2);
               $$ = $1->adopt($3); 
            }
            ;

call        : callexprs ')' {
               destroy($2);
               $$ = $1;
            }
            | TOK_IDENT '(' ')' {
               destroy($3);
               $2 = $2->set_sym(TOK_CALL);
               $$ = $2->adopt( $1);
            }
            ;

variable    : TOK_IDENT         { $$ = $1; }
            | expr '[' expr ']' { 
               destroy($4);
               $2 = $2->set_sym(TOK_INDEX);
               $$ = $2->adopt($1, $3);
            }
            | expr '.' TOK_IDENT {
               $3 = $3->set_sym(TOK_FIELD); 
               $$ = $2->adopt($1, $3);
            }
            ;

constant    : TOK_INTCON      { $$ = $1; }
            | TOK_CHARCON     { $$ = $1; }
            | TOK_STRINGCON   { $$ = $1; }
            | TOK_FALSE       { $$ = $1; }
            | TOK_TRUE        { $$ = $1; }
            | TOK_NULL        { $$ = $1; }
            ;


%%

const char* parser::get_tname (int symbol) {
   return yytname [YYTRANSLATE (symbol)];
}

