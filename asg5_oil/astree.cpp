// $Id: astree.cpp,v 1.14 2016-08-18 15:05:42-07 - - $

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>


#include "astree.h"
#include "string_set.h"
#include "lyutils.h"

astree::astree (int symbol_, const string* info){
   symbol         = symbol_;
   lexinfo        = string_set::intern (info->c_str());
   attributes     = 0;
   block_number   = 0;
   node           = NULL;
}

astree::astree (int symbol_, const location& lloc_, const char* info) {
   symbol         = symbol_;
   lloc           = lloc_;
   lexinfo        = string_set::intern (info);
   attributes     = 0;
   block_number   = 0;
   node           = NULL;
}

astree::~astree() {
   while (not children.empty()) {
      astree* child = children.back();
      children.pop_back();
      delete child;
   }
   if (yydebug) {
      fprintf (stderr, "Deleting astree (");
      astree::dump (stderr, this);
      fprintf (stderr, ")\n");
   }
}

astree* astree::adopt (astree* child1, astree* child2, astree* child3) {
   if (child1 != nullptr) children.push_back (child1);
   if (child2 != nullptr) children.push_back (child2);
   if (child3 != nullptr) children.push_back (child3);
   //astree::print(stderr, this, 0);
   return this;
}

astree* astree::adopt_sym (int symbol_, astree* child1, 
      astree* child2, astree* child3) {
   symbol = symbol_;
   return adopt (child1, child2, child3);
}
astree* astree::set_sym(int symbol_){
   this->symbol = symbol_;
   return this;
}

void astree::dump_node (FILE* outfile) {
   fprintf (outfile, "%s \"%s\" %zd.%zd.%zd \":",
      parser::get_tname (symbol), lexinfo->c_str(),
      lloc.filenr, lloc.linenr, lloc.offset
   );
   /*for (size_t child = 0; child < children.size(); ++child) {
      fprintf (outfile, " %p", children.at(child));
   }*/
}

void astree::dump_tree (FILE* outfile, int depth) {
   fprintf (outfile, "%*s", depth * 3, "");
   dump_node (outfile);
   fprintf (outfile, "\n");
   for (astree* child: children) child->dump_tree (outfile, depth + 1);
      fflush (NULL);
}

void astree::dump (FILE* outfile, astree* tree) {
   if (tree == nullptr) fprintf (outfile, "nullptr");
   else tree->dump_node (outfile);
}

void astree::print (FILE* outfile, astree* tree, int depth) {
   DEBUGF('t', "\tastree print\n");
   for (int i = 0; i < depth; i++) fprintf(outfile, "|   ");

   fprintf (outfile, "%s \"%s\" (%zd.%zd.%zd)\n",
      parser::get_tname (tree->symbol), tree->lexinfo->c_str(),
      tree->lloc.filenr, tree->lloc.linenr, tree->lloc.offset);
   for (astree* child: tree->children) {
      astree::print (outfile, child, depth + 1);
      DEBUGF('t', "\t\tastree print_r\n");

   }
}

void destroy (astree* tree1, astree* tree2, astree* tree3) {
   if (tree1 != nullptr) delete tree1;
   if (tree2 != nullptr) delete tree2;
   if (tree3 != nullptr) delete tree3;
}

void errllocprintf (const location& lloc, const char* format,
 const char* arg) {
   static char buffer[0x1000];
   assert (sizeof buffer > strlen (format) + strlen (arg));
   snprintf (buffer, sizeof buffer, format, arg);
   errprintf ("%s:%zd.%zd: %s", 
    lexer::filename (lloc.filenr), lloc.linenr, lloc.offset,
    buffer);
}
