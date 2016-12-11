// $Id: string_set.h,v 1.2 2016-08-18 15:13:48-07 - - $

#ifndef __STRING_SET__
#define __STRING_SET__

#include <string>
#include <unordered_set>
using namespace std;

#include <stdio.h>

class string_set {
   public:
   string_set();
   static unordered_set<string> set;
   /**
    * Insert a new string into the hash set.
    * @return a pointer to the string just inserted. 
    * If it is already there, nothing is inserted, 
    * and the previously-inserted string is returned.
    */
   const string* intern_stringset (const char*);
   static const string* intern(const char* str){
      auto handle = set.insert (str);
      return &*handle.first;
   }
   /**
   * Dumps out the string set in debug format
   * hash header number, spaces, hash number, address, string
   */
   void dump_stringset (FILE*);
   static void dump(FILE*);
};

#endif

