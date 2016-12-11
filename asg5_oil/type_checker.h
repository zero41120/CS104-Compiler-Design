#ifndef __TYPE_CHECKER_H__
#define __TYPE_CHECKER_H__

#include <fstream>
#include <iostream>
#include <bitset>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

#include "lyutils.h"


template <typename T>
void output(ofstream& out, T arg) {
    out << arg;
    out.flush();
}

class type_checker{
private:
   vector<symbol_table*> symbol_stack;
   symbol_table* struct_stack;
   symbol_table types_table;
   vector<size_t> block_stack;
   size_t blocknr = 0;
      
public:
   type_checker();
   ~type_checker();
   void parse_tree(astree*);
   void write_tree(ofstream&, astree*, int);
   void print_symbol_table(ostream&, symbol_table);   
   string get_attributes(attr_bitset, const string*, const string*);
   void write_attributes
      (ofstream&, attr_bitset, const string*, const string*);
   void write_symbol(ofstream&, astree*, symbol_table*, const string*); 
   void write_node(ofstream&, astree*, int);
   void write_struct_field(ofstream&, astree*, int, const string*);
   void write_struct(ofstream&, astree*, int);
   attr_bitset get_node_attr(astree*);
   symbol* new_symbol(astree*, int);
   void parse_array(astree*);
   void parse_node(astree*);
   void parse_var(astree *);
   void parse_struct_child(astree*, const string*, symbol_table*);
   void parse_struct(astree*);
   void parse_function(astree*);
   void auto_attributes_rec(astree *root);
   void auto_attributes(astree* n);
};

#endif
