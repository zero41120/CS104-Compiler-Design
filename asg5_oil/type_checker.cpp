
#include "type_checker.h"
#include <assert.h>

#define NDEBUG 


using namespace std;

type_checker::type_checker(){
   symbol_stack = vector<symbol_table*>(16, nullptr);
   struct_stack = new symbol_table();
}

type_checker::~type_checker() {
   int size = symbol_stack.size();
   for (int i = 0; i < size; i++) {
      free (symbol_stack[i]);
   }
}


string type_checker::get_attributes
(attr_bitset attr, const string* field_name, const string* struct_name){
   string out;
   if (attr[ATTR_struct] == 1 && struct_name != nullptr) {
      out += "struct \"";
      out += *struct_name;
      out += "\" ";
   } else if (attr[ATTR_struct] == 1){
      out += "struct ";
   }
   if (attr[ATTR_field] == 1 && field_name != nullptr) {
      out += "field {";
      out += *field_name;
      out += "} ";
   } else if (attr[ATTR_field] == 1){
      out += "field ";
   }
   if (attr[ATTR_void]     == 1) out += "void ";
   if (attr[ATTR_bool]     == 1) out += "bool ";
   if (attr[ATTR_int]      == 1) out += "int ";
   if (attr[ATTR_null]     == 1) out += "null ";
   if (attr[ATTR_string]   == 1) out += "string ";
   if (attr[ATTR_array]    == 1) out += "array ";
   if (attr[ATTR_function] == 1) out += "function ";
   if (attr[ATTR_variable] == 1) out += "variable ";
   if (attr[ATTR_typeid]   == 1) out += "typeid ";
   if (attr[ATTR_param]    == 1) out += "param ";
   if (attr[ATTR_lval]     == 1) out += "lval ";
   if (attr[ATTR_const]    == 1) out += "const ";
   if (attr[ATTR_vreg]     == 1) out += "vreg ";
   if (attr[ATTR_vaddr]    == 1) out += "vaddr ";
   return out;
}

void type_checker::write_attributes(ofstream& out, attr_bitset attr,
   const string* field_name, const string* struct_name) {
    output(out, get_attributes(attr, field_name, struct_name));
}



void type_checker::write_symbol
(ofstream& out, astree* node, symbol_table* sym_table, const string* s)
{
    const auto& auto_sym = sym_table->find(s);
    assert(auto_sym != sym_table->end());
    symbol* sym = auto_sym->second;
    const string* struct_name = nullptr;
    if (sym->attributes[ATTR_struct]) {
        if (sym->attributes[ATTR_function] || 
            sym->attributes[ATTR_array]) {
            if (node->symbol == TOK_ARRAY) {
                struct_name = node->children[0]->lexinfo;
            }
        } else {
            const auto& auto_struct = struct_stack->find(node->lexinfo);
            if(auto_struct != struct_stack->end() || 
               node->lexinfo != nullptr){
               struct_name = node->lexinfo;
            } else {
               return;
            }
            //assert(auto_struct != struct_stack->end());
            //assert(node->lexinfo != nullptr);
        }
    }
    write_attributes(out, sym->attributes, nullptr, struct_name);
}

void type_checker::write_node(ofstream& out, astree* node, int depth){
   if (node == nullptr) return;
   if(node->attributes != 0) {
      switch (node->symbol) {
      case '}': case '{': case '(': case ')': case ':':
      case '=': case '+': case '-': case '*': case '/':
      return;
   }
   out << std::string(depth * 3, ' ')  << node->lexinfo->c_str()
       << " ("   << node->lloc.filenr  << ":"
                 << node->lloc.linenr  << "."
                 << node->lloc.offset  << ")"
       << " {"   << node->block_number << "} ";
   write_symbol(out, node, node->node, node->lexinfo);
   out << endl;
  }
}



void type_checker::write_struct_field
(ofstream& out, astree* node, int depth, const string* name) {
    out << std::string(depth * 3, ' ')
        << node->children[0]->lexinfo->c_str()
        << " ("
        << node->lloc.filenr << ":"
        << node->lloc.linenr << "."
        << node->lloc.offset
        << ") ";
    write_attributes(out, node->attributes, name, nullptr);
    out << endl;
}



void type_checker::write_struct
(ofstream& out, astree* node, int depth) {
   out << std::string(depth * 3, ' ') << *node->children[0]->lexinfo
       << " (" << node->children[0]->lloc.filenr << ":"
               << node->children[0]->lloc.linenr << "."
               << node->children[0]->lloc.offset << ")"
       << " {" << node->children[0]->block_number << "} ";
   write_attributes
   (out, node->attributes, nullptr, node->children[0]->lexinfo);
   out << endl;
   for(size_t child = 1; child < node->children.size(); ++child) {
       write_struct_field(out, node->children[child], depth+1,
                          node->children[0]->lexinfo);
   }
}


void type_checker::write_tree(ofstream& out, astree* node, int depth) {
   switch (node->symbol) {
      case TOK_STRUCT:
         if (!node->attributes[ATTR_array]) 
            write_struct(out, node, depth);
         break;
      default:
         write_node(out, node, depth);
         if (node->children.empty()) break;
         for (astree* child: node->children){
            assert(child != nullptr);
            write_tree(out, child, depth+1);
         }
   }
}

void type_checker::auto_attributes(astree* n) {
   if (n == NULL) return;
   DEBUGF('z', "%s \"%s\" (%zd.%zd.%zd)\n",
      parser::get_tname (n->symbol), n->lexinfo->c_str(),
      n->lloc.filenr, n->lloc.linenr, n->lloc.offset);
   switch (n->symbol) {
      case TOK_INTCON: n->attributes[ATTR_const]      = true; 
      case TOK_INT: n->attributes[ATTR_int]           = true;  break;
      case TOK_CHARCON: n->attributes[ATTR_const]     = true;
      case TOK_CHAR: n->attributes[ATTR_int]          = true;  break;
      case TOK_STRINGCON: n->attributes[ATTR_const]   = true;
      case TOK_STRING: n->attributes[ATTR_string]     = true;  break;
      case TOK_VOID: n->attributes[ATTR_void]         = true;  break;
      case TOK_BOOL: n->attributes[ATTR_bool]         = true;  break;
      case TOK_STRUCT: n->attributes[ATTR_struct]     = true;  break;
      case TOK_NULL: n->attributes[ATTR_const]        = true;  break;
      case TOK_FIELD: n->attributes[ATTR_field]       = true;  break;
      case TOK_FUNCTION: n->attributes[ATTR_function] = true;  break;
      case TOK_ARRAY: n->attributes[ATTR_array]       = true;  break;
      case TOK_VARDECL: n->attributes[ATTR_variable]  = true;  break;
      case TOK_PROTOTYPE: n->attributes[ATTR_proto]   = true;  break;
      default: break;
   }
}

void type_checker::auto_attributes_rec (astree *root) {
   if (root == NULL) return;
   if (root->children.empty()) return;
   for (auto &child : root->children) {
      auto_attributes(child);
      auto_attributes_rec(child);
   }  
}


attr_bitset type_checker::get_node_attr(astree* n) {
   attr_bitset attr = n->attributes;
   const auto& auto_node = struct_stack->find(n->lexinfo);
   if (auto_node != struct_stack->end()) {
       attr.set(ATTR_struct);
   }
   switch (n->symbol) {
      case TOK_INTCON:  n->attributes[ATTR_const]      = true; 
      case TOK_INT:     n->attributes[ATTR_int]        = true;  break;
      case TOK_CHARCON: n->attributes[ATTR_const]      = true;
      case TOK_CHAR:    n->attributes[ATTR_int]        = true;  break;
      case TOK_STRINGCON: n->attributes[ATTR_const]    = true;
      case TOK_STRING:  n->attributes[ATTR_string]     = true;  break;
      case TOK_VOID:    n->attributes[ATTR_void]       = true;  break;
      case TOK_BOOL:    n->attributes[ATTR_bool]       = true;  break;
      case TOK_STRUCT:  n->attributes[ATTR_struct]     = true;  break;
      case TOK_NULL:    n->attributes[ATTR_const]      = true;  break;
      case TOK_FIELD:   n->attributes[ATTR_field]      = true;  break;
      case TOK_FUNCTION: n->attributes[ATTR_function]  = true;  break;
      case TOK_ARRAY:   n->attributes[ATTR_array]      = true;  break;
      case TOK_VARDECL: n->attributes[ATTR_variable]   = true;  break;
      case TOK_PROTOTYPE: n->attributes[ATTR_proto]    = true;  break;
      default: break;
   }
   return attr;
}


symbol* type_checker::new_symbol(astree* node, int blocknr) {
   symbol* s        = new symbol();
   s->lloc.filenr   = node->lloc.filenr;
   s->lloc.offset   = node->lloc.offset;
   s->lloc.linenr   = node->lloc.linenr;
   s->blocknr       = blocknr;
   s->attributes    = get_node_attr(node);
   s->parameters    = nullptr;
   s->fields        = nullptr;
   return s;
}


void type_checker::print_symbol_table(ostream& out, symbol_table foo) {
    out << "{";
    for (const auto& i: foo) {
        out << *i.first << ": " << i.second << ", ";
    }
    out << "}" << endl;
}

void type_checker::parse_array(astree* node) {
   node->lexinfo = node->children[1]->lexinfo;
   node->attributes |= node->attributes;
   node->attributes |= node->children[0]->attributes;
   const auto& auto_sym = node->node->find(node->lexinfo);
   assert(auto_sym != node->node->end());
   symbol* sym = auto_sym->second;
   sym->attributes |= node->attributes;
}

void type_checker::parse_var(astree* node){
  node->lexinfo = node->children[0]->lexinfo;
  node->attributes |= node->children[0]->attributes;
}

void type_checker::parse_node(astree* node) {
   if (node == nullptr) return;
   if (symbol_stack[blocknr] == nullptr) {
      vector<symbol_table*>::iterator it;
      it = symbol_stack.begin() + blocknr;
      symbol_stack.insert(it, new symbol_table());
   }
   symbol* s = new_symbol(node, blocknr);
   symbol_stack[blocknr]->insert(symbol_entry(node->lexinfo, s));
   node->node = symbol_stack[blocknr];
   node->block_number = s->blocknr;
   node->attributes = s->attributes;
   switch (node->symbol) {
      case TOK_ARRAY:   parse_array(node);  break;
      case TOK_VARDECL: parse_var(node);    break;
   }
}



void type_checker::parse_struct_child
(astree* node, const string* name, symbol_table* fields) {
   if (node == nullptr) return;
   symbol* s = new_symbol(node, 0);
   s->attributes.set(ATTR_field);
   fields->insert(symbol_entry(node->lexinfo, s));
   node->node = fields;
   node->attributes = s->attributes;
   astree* first_child;
   try {
       first_child = node->children.at(0);
   } catch (out_of_range& e) {
       return;
   }
   parse_struct_child(first_child, name, fields);
}


void type_checker::parse_struct(astree* node) {
   const string* struct_name = node->children[0]->lexinfo;
   const auto& auto_node = struct_stack->find(struct_name);
   symbol* sym;
   if (auto_node != struct_stack->end()) {
       sym = auto_node->second;
   } else {
       sym = new_symbol(node, 0);
       sym->fields = new symbol_table();
   }
   node->block_number = 0;
   node->attributes.set(ATTR_struct);
   node->node = struct_stack;
   for(size_t child = 1; child < node->children.size(); ++child) {
       parse_struct_child(node->children[child], struct_name,
                          sym->fields);
   }
   struct_stack->insert(symbol_entry(struct_name, sym));
}


void type_checker::parse_function(astree* node) {
    assert(node != nullptr);

   //set node's attributes to that of its first child
   assert(node->children[0] != nullptr);
   node->attributes |= node->children[0]->attributes;

   //set function's paramlist to all be ATTR_param
   for (size_t i = 0; i < node->children[1]->children.size(); i++) {
      astree* child = node->children[1]->children[i];
      child->children[0]->attributes |= child->attributes;
      child->children[0]->attributes.set(ATTR_param);
      assert (child->node != nullptr);
      const auto& auto_node = 
         child->node->find(child->children[0]->lexinfo);
      assert (auto_node != node->node->end());
      auto_node->second->attributes = child->children[0]->attributes;
   }
}


void type_checker::parse_tree(astree* node) {
   auto_attributes_rec(node);
   vector<symbol_table*>::iterator it;
   switch (node->symbol) {
       case TOK_STRUCT:    parse_struct(node); return;
       case TOK_BLOCK:     blocknr++;          break;
   }
   for (size_t child = 0; child < node->children.size(); ++child) {
       parse_tree(node->children[child]);
   }
   switch (node->symbol) {
      case TOK_FUNCTION:
         parse_function(node);
         parse_node(node);
         break;
      case TOK_BLOCK:
         it = symbol_stack.begin() + blocknr;
         symbol_stack.insert(it, nullptr);
         blocknr--;
      default:
         parse_node(node);
         break;
   }
}
