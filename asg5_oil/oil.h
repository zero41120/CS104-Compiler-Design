#ifndef __CODE_GENERATOR_H__
#define __CODE_GENERATOR_H__

#include <iostream>
#include <bitset>
#include <string>
#include <unordered_map>
#include <vector>

#include "lyutils.h"

using namespace std;


class oil_language{
public:
   void write_oil(ofstream&, astree*, int);

private:
   string get_type(astree*, const string*);
   string get_name(astree*, string);
   string get_reg(string);

   void format(ofstream&, astree*, int, astree*);
   void format_new(ofstream&, astree*, int);
   void format_call(ofstream&, astree*, int);
   void format_unary(ofstream&, astree*, int);
   void format_while(ofstream&, astree*, int);
   void format_equal(ofstream&, astree*, int);
   void format_return(ofstream&, astree*, int);
   void format_binary(ofstream&, astree*, int);
   void format_struct(ofstream&, astree*, int);
   void format_function(ofstream&, astree*, int);
   void format_stringcon(ofstream&, astree*, int);
   void format_expression(ofstream&, astree*, int);
   void format_condition(ofstream&, astree*, int, astree*);
};

#endif
