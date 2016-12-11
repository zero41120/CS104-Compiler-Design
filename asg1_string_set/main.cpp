
#include <string>
#include <unistd.h>
#include <iostream>
#include <fstream>
using namespace std;

#include "auxlib.h"
#include "stringset.h"
#include "cppstrtok.h"


// Code provided by Mackey
int scan_options(int argc, char** argv) {
   int option;
   opterr = 0;
   for(;;) {
      option = getopt(argc, argv, "ly@:");
      if (option == EOF) break;
      switch (option) {
         case '@': set_debugflags(optarg);             break;
         case 'l':          break;
         case 'y':          break;
         default:  errprintf("%:bad option (%c)\n", optopt); break;
      }
   }
   if (optind > argc) {
      errprintf("Usage: %s [-ly] [filename]\n", get_execname());
      exit(get_exitstatus());
   }
   return optind;
}

int scan_oc_extension(string name){
   size_t dotpos = name.find_last_of (".");
   if (dotpos == string::npos) {
      cout << "Invalid file format" << endl;
   }
   if(name.substr(name.find_last_of(".") + 1) != "oc") {
      set_exitstatus(EXIT_FAILURE);
      return 0;
   }
   return 1;
}

int main (int argc, char** argv) {
   // Handle the command line args and debug flags
   set_execname (argv[0]);
   set_exitstatus(EXIT_SUCCESS);
   int argc_ = scan_options (argc, argv);
   int show = 0;
   string_set set;


   // Filter the input through C preprocessor (cppstrok)
   // Read use fgets(), read only from C preprocessor
   // Tokenize use strtok_r() " \t\n" 
   for (int argi = argc_; argi < argc; ++argi) {
      char* filename = argv[argi];
      string filename_s(filename);
      size_t dotpos = filename_s.find_last_of (".");
      string outputname = filename_s.substr (0, dotpos) + ".str";
      cout << outputname << endl;
      if (scan_oc_extension(filename) == 0){
         return get_exitstatus();
      }
      string command = CPP + " " + filename;
      FILE* pipe = popen (command.c_str(), "r");
      if (pipe == NULL) {
         set_exitstatus(EXIT_FAILURE);
         cerr << get_execname() << command.c_str() << endl;
         return get_exitstatus();
      }else {
         cpplines (pipe, filename, show, set);
         int pclose_rc = pclose (pipe);
         eprint_status (command.c_str(), pclose_rc);
         if (pclose_rc != 0) set_exitstatus(EXIT_FAILURE);
      }
      FILE *fp;
      fp = fopen(outputname.c_str(), "w");
      set.dump_stringset(fp);
      fclose(fp);
   }

   // Insert into string_set ADT (stringset.cpp)
   /*for (int i = 1; i < argc; ++i) {
      const string* str = string_set::intern_stringset (argv[i]);
      DEBUGF ('m', str->c_str());
   }*/

   // Dump the output as .str (stringset.cpp)
      
   // set.dump_stringset (stdout);

      return get_exitstatus();
   }
