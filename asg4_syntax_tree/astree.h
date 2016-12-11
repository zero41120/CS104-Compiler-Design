// $Id: astree.h,v 1.10 2016-10-06 16:42:35-07 - - $

#ifndef __ASTREE_H__
#define __ASTREE_H__


#include <string>
#include <iostream>
#include <unordered_map>
#include <bitset>
#include <vector>

using namespace std;

#include "auxlib.h"


struct astree;
struct symbol;
struct location;

struct location {
   size_t filenr;
   size_t linenr;
   size_t offset;
};

/* TA's code 
 * The enum for all the possible flags
 * ATTR_bitset_size is not actually used
 * but its value is equal to the number of attributes */
enum { ATTR_void, ATTR_int, ATTR_null, ATTR_string, ATTR_proto,
   ATTR_struct, ATTR_array, ATTR_function, ATTR_variable,
   ATTR_field, ATTR_typeid, ATTR_param, ATTR_lval, ATTR_const,
   ATTR_vreg, ATTR_vaddr, ATTR_bool, ATTR_bitset_size 
};

using attr_bitset = bitset<ATTR_bitset_size>;
using symbol_table = unordered_map<const string*, symbol*>;
using symbol_entry = pair<const string*,symbol*>;

struct symbol {
   attr_bitset attributes;       // What attributes this symbol has
   location lloc;                // Location in the file
   size_t blocknr;               // Which block it blongs
   symbol_table *fields;         // If struct, it has field
   vector<symbol*> *parameters;  // If function, it has argument
};

struct astree {
   // Fields.
   int symbol;                // token code
   location lloc;             // source location
   const string* lexinfo;     // pointer to lexical information
   vector<astree*> children;  // children of this n-way node
   attr_bitset attributes;
   int block_number;
   symbol_table* node;
   const string* struct_name;

   // Functions.
   astree (int symbol, const location&, const char* lexinfo);
   ~astree();
   astree* set_sym(int symbol);
   void dump_node (FILE*);
   void dump_tree (FILE*, int depth = 0);
   static void dump (FILE* outfile, astree* tree);
   static void print (FILE* outfile, astree* tree, int depth = 0);
   astree* adopt (astree* child1, 
      astree* child2 = nullptr, 
      astree* child3 = nullptr);
   astree* adopt_sym (int symbol, 
      astree* child1, 
      astree* child2 = nullptr, 
      astree* child3 = nullptr);
};



void destroy (astree* tree1, 
   astree* tree2 = nullptr, astree* tree3 = nullptr);

void errllocprintf (const location&, const char* format, const char*);





#endif

