
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

FILE *tok_file; // lyutils.h uses extern to find this FILE*
FILE *str_file;
string cpp_command;
const string CPP = "/usr/bin/cpp";

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
         fprintf(stderr, "\tparser has something\n");
         parser::root->dump_tree (stderr);
      } else {
         fprintf(stderr, "\tparser is nullptr\n");
      }
      fprintf (stderr, "Dumping string_set:\n");
      string_set::dump (stderr);
   }
}


int scan_oc_extension(string name){
   return (name.find_last_of (".") == string::npos ||
      name.substr(name.find_last_of(".") + 1) != "oc")? 0 : 1;
}

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

int scan_options(int argc, char** argv) {
   int option;
   opterr = 0;
   yy_flex_debug = 0;
   yydebug = 0;
   lexer::interactive = isatty (fileno (stdin))
   and isatty (fileno (stdout));
   for(;;) {
      option = getopt(argc, argv, "ly@:");
      if (option == EOF)                        break;
      switch (option) {
         case '@': set_debugflags(optarg);      break;
         case 'l': yy_flex_debug = 1;           break;
         case 'y': yydebug = 1;                 break;
         default : errprintf ("bad option (%c)\n", optopt); break;
      }
   }
   if (optind > argc) {
      errprintf ("Usage: %s [-ly] [filename]\n",
        exec::execname.c_str());
      exit (exec::exit_status);
   }
   const char* filename = optind == argc ? "-" : argv[optind];
   DEBUGF('t', "\tfilename: %s\n", filename);
   
   // Open file, check oc extension
   if (scan_oc_extension(filename) == 0){
      exec::exit_status = EXIT_FAILURE;
      cerr << "Invalid file" << endl;
   } else {
      cpp_popen(filename);
   }
   return optind;
}

void make_str_file(string_set& set, char* name){
   /* The function yylex() and sets the string in string_set, 
    * this function only dumps the output*/

   // Make right names
   string s(name);
   string output_str = s.substr (0, s.find_last_of (".")) + ".str";

   // Output .str
   str_file = fopen(output_str.c_str(), "w");
   set.dump_stringset(str_file);
   fclose(str_file);
}

void make_tok_file(int& token, char* name){
   /* The function yylex() calls yylval_token() in scanner.l, 
    * all tok outputs are done there. */

   // Make tok names
   string s(name);
   string output_tok = s.substr (0, s.find_last_of (".")) + ".tok";

   // Get tokens
   tok_file = fopen(output_tok.c_str(), "w");
   while((token = yylex()) != YYEOF) {}
   fclose(tok_file);
}


int main (int argc, char** argv) {
   //Basic setup
   exec::execname = basename (argv[0]);
   debug_command(argc, argv);
   string_set set;
   int token;

   // Scan option and popen file
   int argc_ = scan_options(argc, argv);
   if (exec::exit_status != EXIT_FAILURE){
      char* filename_m = argv[argc_];

      // Make output files
      make_tok_file(token, filename_m);
      make_str_file(set,   filename_m);

      // Clean up
      cpp_pclose();
      yylex_destroy();
   }

   return exec::exit_status;
}











