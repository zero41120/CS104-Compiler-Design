// $Id: cppstrtok.cpp,v 1.6 2016-08-18 13:00:16-07 - - $

// Use cpp to scan a file and print line numbers.
// Print out each input line read in, then strtok it for
// tokens.

#include <string>
using namespace std;

#include "cppstrtok.h"


// Chomp the last character from a buffer if it is delim.
void chomp (char* string, char delim) {
   size_t len = strlen (string);
   if (len == 0) return;
   char* nlpos = string + len - 1;
   if (*nlpos == delim) *nlpos = '\0';
}

// Run cpp against the lines of the file.
void cpplines (FILE* pipe, char* filename, 
               int show, string_set &set) {
   int linenr = 1;
   char inputname[LINESIZE];
   strcpy (inputname, filename);
   for (;;) {
      char buffer[LINESIZE];
      char* fgets_rc = fgets (buffer, LINESIZE, pipe);
      if (fgets_rc == NULL) break;
      chomp (buffer, '\n');
      
      if (show == 1){
         printf ("%s:line %d: [%s]\n", filename, linenr, buffer);
      }

      // http://gcc.gnu.org/onlinedocs/cpp/Preprocessor-Output.html
      int sscanf_rc = sscanf (buffer, "# %d \"%[^\"]\"",
                              &linenr, filename);
      if (sscanf_rc == 2) {
         if (show == 1){
            printf ("DIRECTIVE: line %d file \"%s\"\n", 
               linenr, filename);
         }
         continue;
      }
      char* savepos = NULL;
      char* bufptr = buffer;

      for (int tokenct = 1;; ++tokenct) {
         char* token = strtok_r (bufptr, " \t\n", &savepos);
         bufptr = NULL;
         if (token == NULL) break;
         if (show == 1){
            printf ("token %d.%d: [%s]\n", linenr, tokenct, token);
         }
         set.intern_stringset (token);
      }
      ++linenr;
   }
}

