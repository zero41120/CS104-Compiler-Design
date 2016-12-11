
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <libgen.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

#include "auxlib.h"
#include "astree.h"
#include "lyutils.h"
#include "string_set.h"

/******************************************
               C PREPROCESSOR
 ******************************************/

string cpp_command;
const string CPP = "/usr/bin/cpp";

void cpp_popen (const char* filename) {
   cpp_command = CPP + " " + filename;
   yyin = popen (cpp_command.c_str(), "r");
   if (yyin == NULL) {
      syserrprintf (cpp_command.c_str());
      exit (exec::exit_status);
   }else {
      if (yy_flex_debug) {
         fprintf (stderr, "-- popen (%s), fileno(yyin) = %d\n",
            cpp_command.c_str(), fileno (yyin));
      }
      lexer::newfilename (cpp_command);
   }
}

void cpp_pclose() {
   int pclose_rc = pclose (yyin);
   eprint_status (cpp_command.c_str(), pclose_rc);
   if (pclose_rc != 0) exec::exit_status = EXIT_FAILURE;
}


/******************************************
                  DEBUG
 ******************************************/
void debug_command(int argc, char** argv){
   if (yydebug or yy_flex_debug) {
      fprintf (stderr, "Command:");
      for (char** arg = &argv[0]; arg < &argv[argc]; ++arg) {
         fprintf (stderr, " %s", *arg);
      }
      fprintf (stderr, "\n");
   }
}

void debug_dump(){
   if (yydebug or yy_flex_debug) {
      fprintf (stderr, "Dumping parser::root:\n");
      if (parser::root != nullptr) {
         astree::print(stderr, parser::root);
      }
      fprintf (stderr, "Dumping string_set:\n");
      string_set::dump (stderr);
   }
}

int scan_oc_extension(string name){
   return (name.find_last_of (".") == string::npos ||
      name.substr(name.find_last_of(".") + 1) != "oc")? 0 : 1;
}

int scan_options(int argc, char** argv) {
   // Basic setup
   debug_command(argc, argv);
   opterr = yy_flex_debug = yydebug = 0;
   lexer::interactive = isatty (fileno (stdin))
                        and isatty (fileno (stdout));
   // Scan options
   for(int option;;) {
      option = getopt(argc, argv, "ly@:");
      if (option == EOF)                        break;
      switch (option) {
         case '@': set_debugflags(optarg);      break;
         case 'l': yy_flex_debug = 1;           break;
         case 'y': yydebug = 1;                 break;
         default : errprintf ("bad option (%c)\n", optopt); break;
      }
   }

   // Print option issue
   if (optind > argc) {
      errprintf ("Usage: %s [-ly] [filename]\n",
       exec::execname.c_str());
      exit (exec::exit_status);
   }

   // Check file extension and do cpp
   const char* filename = optind == argc ? "-" : argv[optind];   
   if (scan_oc_extension(filename) == 0){
      exec::exit_status = EXIT_FAILURE;
      cerr << "Invalid file" << endl;
   } else {
      cpp_popen(filename);
   }
   return optind;
}


/******************************************
               FILE MANAGE
 ******************************************/

FILE *oil_file = nullptr; // TODO
FILE *sym_file = nullptr; // TODO
FILE *ast_file = nullptr; // use astree_dump
FILE *tok_file = nullptr; // lyutils & scanner uses extern
FILE *str_file = nullptr; // use string_dump

void open_files(char* name){
   // Make right names
   string s(name);
   string output_str = s.substr (0, s.find_last_of (".")) + ".str";
   string output_tok = s.substr (0, s.find_last_of (".")) + ".tok";
   string output_ast = s.substr (0, s.find_last_of (".")) + ".ast";
   string output_sym = s.substr (0, s.find_last_of (".")) + ".sym";
   string output_oil = s.substr (0, s.find_last_of (".")) + ".oil";

   str_file = fopen(output_str.c_str(), "w");
   tok_file = fopen(output_tok.c_str(), "w");
   ast_file = fopen(output_ast.c_str(), "w");
   // sym_file = fopen(output_sym.c_str(), "w");
   // oil_file = fopen(output_oil.c_str(), "w");
}

void close_files(){
   fclose(tok_file);
   fclose(str_file);
   fclose(ast_file);
   // fclose(sym_file);
   // fclose(oil_file);

}

void make_output_files(string_set& set){
   /* STR: yyparse calls  yylex() and sets the string in string_set */
   set.dump_stringset(str_file);
   /* TOK: yyparse calls yylex() calls yylval_token() in scanner.l */
   /* AST: yyparse sets the root for printing */
   astree::print(ast_file, parser::root, 0);
}

/******************************************
              POST PROCESSING
 ******************************************/
void clean_up(){
   debug_dump();
   close_files();
   cpp_pclose();
   yylex_destroy();
   destroy(parser::root);
}


/******************************************
                  MAIN
 ******************************************/

int main (int argc, char** argv) {
   
   //Basic setup
   exec::execname = basename (argv[0]);
   string_set set;

   // Scan option and popen file
   int argc_ = scan_options(argc, argv);

   // Check file extension is done in scan_option()
   if (exec::exit_status == EXIT_FAILURE) return exec::exit_status;

   // Open output files
   open_files(argv[argc_]);
      
   // Do program processing
   yyparse();

   // Output files
   make_output_files(set);

   // Clean up
   clean_up();
   
   return exec::exit_status;
}











