#include <iostream>
#include <string>
#include <vector>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "oil.h"
#include "type_checker.h"
#include "auxlib.h"
#include "lyutils.h"


using namespace std;

size_t reg_counter = 1;

string oil_language::get_name
(astree* node, string old_name) {
    string new_name;
    int symbol = node->symbol;

    if (node->block_number != 0) {
        switch (symbol) {
            case TOK_WHILE:
            case TOK_IF:
            case TOK_IFELSE:
                new_name = old_name + "_"
                    + to_string(node->lloc.filenr) + "_"
                    + to_string(node->lloc.linenr) + "_"
                    + to_string(node->lloc.offset) + ":;";
                break;
            case TOK_INTCON:    
                new_name = old_name;
                break;
            case TOK_FUNCTION:  
                new_name = "__" + old_name;
                break;
            default:            
                new_name = "_"
                + to_string(node->block_number)
                + "_" + *node->lexinfo;
        }
    } else {
        switch (symbol) {
            case TOK_STRUCT:    
                new_name = "s_" + old_name;
                break;
                               
            case TOK_TYPEID:    
                new_name = "f_" + *node->lexinfo
                + "_" + old_name;
                break;
            case TOK_WHILE:
            case TOK_IF:
            case TOK_IFELSE:
                new_name = old_name + "_"
                + to_string(node->lloc.filenr) + "_"
                + to_string(node->lloc.linenr) + "_"
                + to_string(node->lloc.offset) + ":;";
                break;          
            case TOK_DECLID:    
                new_name = "_"
                + to_string(node->block_number)
                + "_" + *node->lexinfo;
                break;
            default:
                new_name = "__" + old_name;
                break;
        }
    }
    return new_name;
}

string oil_language::get_type
(astree* node, const string* struct_name) {
    string old_type = *node->lexinfo;
    string new_type;
    if (old_type == "int") {
        new_type = "int";
    } else if (old_type == "string") {
        new_type = "string";
    } else {
        if (struct_name == nullptr)
            return "ERR" + old_type;
        astree* a_struct = new astree(TOK_STRUCT , struct_name);
        new_type = "struct " + get_name(a_struct, old_type) + "*";
    }
    return new_type;
}

string oil_language::get_reg
(string s) {
    if (s == "int")     return "i";
    if (s == "string")  return "s";
    return "i";
}

void oil_language::format
(ofstream& out, astree* node,
                   int depth, astree* extra) {
    switch (node->symbol) {
        case TOK_BLOCK:     
            for (astree* child: node->children) {
                format(out, child, depth+1, extra);
            }
            break;

        case TOK_WHILE:     
            format_while(out, node, 0);
            break;

        case TOK_IF:        
            format_condition(out, node->children[0],
                            depth, extra);
            out << "if (!b" << reg_counter-1 << ") "
                << "goto fi_"
                << to_string(node->lloc.filenr) << "_"
                << to_string(node->lloc.linenr) << "_"
                << to_string(node->lloc.offset)
                << ";" << endl;
            format(out, node->children[1],
                          depth, extra);
            out << "fi_"
                << to_string(node->lloc.filenr) << "_"
                << to_string(node->lloc.linenr) << "_"
                << to_string(node->lloc.offset)
                << ":;" << endl;
            break;

        case '=':           
            format_equal(out, node, depth);
            break;

        case TOK_RETURN:    
            format_return(out, node, depth);
            break;

        case TOK_PROTOTYPE: 
            break;

        case TOK_CALL:      
            format_call(out, node, depth);
            out << "; " << endl;
            break;

        default:
            out << string(depth * 3, ' ');
            out << parser::get_tname(node->symbol) << endl;
            break;
    }
}

void oil_language::format_new
(ofstream& out, astree* node, int depth) {
    astree* new_type = node->children[0];
    if (new_type->symbol == TOK_TYPEID) {
        out << "struct " << *new_type->lexinfo
            << "* p" << reg_counter++ << " = "
            << "xcalloc (1, sizeof (struct "
            << get_name(new_type, *new_type->lexinfo)
            << "));" << endl;
    } else if (node->symbol == TOK_NEWARRAY) {
        out << get_type(node->children[0],
                            node->children[0]->lexinfo)
            << "* p" << reg_counter++
            << " = xcalloc (" << *node->children[1]->lexinfo
            << ", sizeof ("
            << get_name(node->children[0],
                          *node->children[0]->lexinfo )
            << "));" << endl;
    } else if (node->symbol == TOK_NEWSTRING) {
        out << "char* p" << reg_counter++
            << " = xcalloc (" << node->children[0]->lexinfo->length()-2
            << ", sizeof (char));" << endl;
    } else {
        out << "ERROR: " << *node->lexinfo << ";" << endl;
    }
}

void oil_language::format_call
(ofstream& out, astree* node, int depth) {
    node->children[0]->symbol = TOK_FUNCTION;
    out << string(depth * 3, ' ')
        << get_name(node->children[0], *node->children[0]->lexinfo)
        << " (";

    for (size_t i = 1; i < node->children.size(); i++) {
        astree* arg = node->children[i];
        out << get_name(arg, *arg->lexinfo);
        if (i+1 != node->children.size()) {
            out << ", ";
        }
    }
    out << ")";
}

void oil_language::format_unary
(ofstream& out, astree* node, int depth) {
    astree* expr = node->children[0];
    out << "char b" << reg_counter++
        << " = " << *node->lexinfo
        << get_name(expr, *expr->lexinfo)
        << ";" << endl;
}

void oil_language::format_while
(ofstream& out, astree* node, int depth) {
    out << get_name(node, *node->lexinfo) << endl;

    format_condition(out, node->children[0], depth, node);
    out << "if (!b" << reg_counter-1
        << ") goto break_"
        << to_string(node->lloc.filenr) << "_"
        << to_string(node->lloc.linenr) << "_"
        << to_string(node->lloc.offset)
        << ";" << endl;

    format(out, node->children[1], depth, node);

    out << "goto "
        << get_name(node, *node->lexinfo) << endl;

    out << "break_"
        << to_string(node->lloc.filenr) << "_"
        << to_string(node->lloc.linenr) << "_"
        << to_string(node->lloc.offset)
        << ":" << endl;
}

void oil_language::format_equal
(ofstream& out, astree* node, int depth) {
    astree* left = node->children[0];
    astree* right = node->children[1];
    if (left->symbol != TOK_IDENT) {
        switch (right->symbol) {
            //case int x=y, char y=z
            case TOK_IDENT:     
                out << string(depth*3, ' ')
                << get_type(left, left->lexinfo)
                << " " << get_name(
                        left->children[0],
                        *left->children[0]->lexinfo)
                << " = " << get_name(right,
                                *right->lexinfo)
                << ";" << endl;
                break;
            case TOK_INTCON:
            case TOK_STRINGCON:
            case TOK_TRUE:
            case TOK_FALSE:     
                out << string(depth*3, ' ')
                << get_type(left, left->lexinfo)
                << " "
                << get_name(left->children[0],
                        *left->children[0]->lexinfo)
                << " = " << *right->lexinfo
                << ";" << endl;
                break;
            case TOK_ORD:
            case TOK_CHR:       
                out << string(depth*3, ' ')
                << get_type(left, left->lexinfo)
                << " " << get_name(
                        left->children[0],
                        *left->children[0]->lexinfo)
                << " = "
                << get_name(right,
                              *right->lexinfo)
                << " ("
                << get_name(
                      right->children[0],
                      *right->children[0]->lexinfo);
                out << ");" << endl;
                break;
            
            case TOK_NEWARRAY:      
                format_new(out, right, depth);
                out << get_type(
                        left->children[0],
                        left->children[0]->lexinfo)
                << "* "
                << *left->children[1]->lexinfo
                << " = p" << reg_counter-1
                << ";" << endl;
                break;
            case TOK_NEWSTRING:
            case TOK_NEW:
                format_new(out, right, depth);
                out << get_type(left, left->lexinfo)
                << " "
                << *left->children[0]->lexinfo
                << " = p" << reg_counter-1
                << ";" << endl;
                break;
            case '!':
                out << string(depth*3, ' ')
                << get_type(left, left->lexinfo) << " "
                << get_name(left->children[0],
                                *left->children[0]->lexinfo)
                << " = !"
                << get_name(
                            right->children[0],
                            *right->children[0]->lexinfo)
                << ";" << endl;
                break;

            case '+':
            case '-':       
                if (right->children.size() == 1) {
                    //its unary!
                    out << string(depth*3, ' ')
                    << get_type(left, left->lexinfo)
                    << " "
                    << get_name(left, *left->lexinfo)
                    << " = " << *right->lexinfo
                    << get_name(right->children[0],
                               *right->children[0]->lexinfo)
                    << ";" << endl;
                    break;
                }
            case '/':
            case '*':       
                out << string(depth*3, ' ')
                << get_type(left, left->lexinfo)
                << " " << get_reg(*left->lexinfo)
                << reg_counter++
                << " = ";
                format_expression(out, right, depth);
                out << ";" << endl;
                out << string(depth*3, ' ')
                << get_type(left, left->lexinfo)
                << " "
                << get_name(left->children[0],
                        *left->children[0]->lexinfo)
                << " = " << get_reg(*left->lexinfo)
                << reg_counter-1
                << ";" << endl;
                break;

            default:        
                out << string(depth*3, ' ')
                << get_type(left, left->lexinfo)
                << " " << get_reg(*left->lexinfo)
                << reg_counter++
                << " = ";
                format_call(out, right, depth);
                out << ";" << endl;
                out << string(depth*3, ' ')
                << get_type(left, left->lexinfo)
                << " "
                << get_name(left->children[0],
                        *left->children[0]->lexinfo)
                << " = " << get_reg(*left->lexinfo)
                << reg_counter-1
                << ";" << endl;
                break;
        }
    } else {
        switch (right->symbol) {
            //case x=y
            case TOK_IDENT:     
                out << string(depth*3, ' ')
                << get_name(left, *left->lexinfo)
                << " = " << get_name(right,
                            *right->lexinfo)
                << ";" << endl;
                break;

            case TOK_INTCON:
            case TOK_STRINGCON:
            case TOK_TRUE:
            case TOK_FALSE:     
                out << string(depth*3, ' ')
                << get_name(left, *left->lexinfo)
                << " = " << *right->lexinfo
                << ";" << endl;
                break;
            case TOK_ORD:
            case TOK_CHR:       
                out << string(depth*3, ' ')
                << get_name(left, *left->lexinfo)
                << " = "
                << get_name(right,
                        *right->lexinfo)
                << " ("
                << get_name(
                      right->children[0],
                      *right->children[0]->lexinfo);
                out << ");" << endl;
                break;
            
            case TOK_NEWARRAY:      
                format_new(out, right, depth);
                out << *left->lexinfo
                << " = p" << reg_counter-1
                << ";" << endl;
                break;
            case TOK_NEWSTRING:
            case TOK_NEW:       
                format_new(out, right, depth);
                out << *left->lexinfo
                << " = p" << reg_counter-1
                << ";" << endl;
                break;
            case '!':       
                out << string(depth*3, ' ')
                << get_name(left, *left->lexinfo)
                << " = " << *right->lexinfo
                << get_name(right->children[0],
                            *right->children[0]->lexinfo)
                << ";" << endl;
                break;
            case '+':
            case '-':       
                if (right->children.size() == 1) {
                    out << string(depth*3, ' ')
                        << get_name(left, *left->lexinfo)
                        << " = " << *right->lexinfo
                        << get_name(right->children[0],
                               *right->children[0]->lexinfo)
                        << ";" << endl;
                    break;
            }
            case '/':
            case '*':       
                out << string(depth*3, ' ')
                << "int " //do we want to hide this better?
                << get_reg(*left->lexinfo)
                << reg_counter++
                << " = ";
                format_expression(out, right, depth);
                out << ";" << endl;
                out << string(depth*3, ' ')
                << get_name(left, *left->lexinfo)
                << " = " << get_reg(*left->lexinfo)
                << reg_counter-1
                << ";" << endl;
                break;
            default:
                out << string(depth*3, ' ')
                << *left->lexinfo
                << " " << get_reg(*left->lexinfo)
                << reg_counter++
                << " = ";
                format_call(out, right, depth);
                out << ";" << endl;
                out << string(depth*3, ' ')
                << get_name(left, *left->lexinfo)
                << " = " << get_reg(*left->lexinfo)
                << reg_counter-1
                << ";" << endl;
                break;
        }
    }
}

void oil_language::format_return
(ofstream& out, astree* node, int depth) {
    astree* return_val = node->children[0];
    out << string(depth * 3, ' ');
    if (return_val != nullptr) {
        if (return_val->symbol == TOK_IDENT) {
            out << "return "
                << get_name(return_val, *return_val->lexinfo)
                << ";" << endl;
        } else {
            out << "return "
                << *return_val->lexinfo
                //<< " " << get_name(return_val->children[0],
                //*return_val->children[0]->lexinfo)
                << ";" << endl;
        }
    } else {
        out << "return;" << endl;
    }
}

void oil_language::format_binary
(ofstream& out, astree* node, int depth) {
    astree* left = node->children[0];
    astree* right = node->children[1];

    out << "char b" << reg_counter++
        << " = " << get_name(left, *left->lexinfo)
        << " " << *node->lexinfo
        << " " << get_name(right, *right->lexinfo)
        << ";" << endl;
}

void oil_language::format_struct
(ofstream& out, astree* child, int depth) {
    astree* struct_name = child->children[0];
    out << "struct "
        << get_name(child, *struct_name->lexinfo)
        << " {" << endl;

    depth++;
    for(size_t i = 1; i < child->children.size(); ++i) {
        astree* type  = child->children[i];
        astree* field = type->children[0];
        string old_name = *field->lexinfo;
        field->lexinfo = struct_name->lexinfo;
        out << string(depth * 3, ' ')
            << get_type(type, struct_name->lexinfo) << " "
            << get_name(field, old_name)
            << ";" << endl;
    }

    out << "};" << endl;
}

void oil_language::format_function
(ofstream& out, astree* node, int depth) {
    //gen function type and name
    astree* return_type = node->children[0];
    astree* name = return_type->children[0];
    name->symbol = TOK_FUNCTION;
    out << get_type(return_type, return_type->lexinfo)
        << " " << get_name(name, *name->lexinfo)
        << " (" << endl;

    //gen function parameter list
    depth++; int next = 2;
    astree* paramlist = node->children[1];
    if (paramlist->symbol == TOK_PARAMLIST) {
        for (size_t i = 0; i < paramlist->children.size(); i++) {
            astree* param_type = paramlist->children[i];
            astree* param = param_type->children[0];
            out << string(depth * 3, ' ')
                << get_type(param_type, param_type->lexinfo)
                << " " << get_name(param, *param->lexinfo);
            if (i+1 != paramlist->children.size())
                out << "," << endl;
        }
    } else {
        next = 1;
    }
    out << ")" << endl
        << "{" << endl;

    //gen function block
    astree* block = node->children[next];
    format(out, block, depth, nullptr);

    out << "}" << endl;
}

void oil_language::format_stringcon
(ofstream& out, astree* node, int depth) {
    if (node->symbol == '=') {
        if (node->children[1]->symbol == TOK_STRINGCON) {
            out << "char* "
                << get_name(node,
                        *node->children[0]->children[0]->lexinfo)
                << " = " << *node->children[1]->lexinfo
                << endl;
        }
    } else {
        for (astree* child: node->children) {
            format_stringcon(out, child, depth);
        }
    }
}

void oil_language::format_expression
(ofstream& out, astree* node, int depth){
    //handle first child
    //recur if +-/*
    astree* first = node->children[0];
    switch (first->symbol){
        case TOK_IDENT: 
            out << get_name(first,*first->lexinfo)
            << " " << *node->lexinfo << " ";
            break;

        case '+':
        case '-':       
            if (first->children.size() == 1) {
                out << *first->lexinfo
                << *first->children[0]->lexinfo;
                break;
            }
        case '/':
        case '*':       
            format_expression(out, first, depth);
            out << *first->lexinfo << " ";
            break;

        case TOK_STRINGCON:
        case TOK_INTCON:
            out << *first->lexinfo << " "
            << *node->lexinfo << " ";
            break;

        case TOK_CALL: 
            format_call(out, first, depth);
            break;
    }

    //handle second child
    astree* second = node->children[1];
    switch (second->symbol){
        case TOK_IDENT: 
            out << get_name(second, *second->lexinfo);
            break;
        case TOK_STRINGCON:
        case TOK_INTCON:
            out << *second->lexinfo << " ";
            break;
        case TOK_CALL: 
            format_call(out, second, depth);
            out << " ";
            break;
    }
}

void oil_language::format_condition
(ofstream& out, astree* node,
                     int depth, astree* extra) {
    size_t size = node->children.size();
    if (size == 2) {
        format_binary(out, node, depth);
    } else if (size==1){
        format_unary(out, node, depth);
    } else {
        out << "char b" << reg_counter++
        << " = "
        << get_name(node, *node->lexinfo)
        << ";" << endl;
    }
}

void oil_language::write_oil
(ofstream& out, astree* root, int depth) {
    //gen all structs
    for (astree* child: root->children) {
        if (child->symbol == TOK_STRUCT)
            format_struct(out, child, depth);
    }
    out << "after structs" << endl;
    //gen all string consts
    for (astree* child: root->children) {
        format_stringcon(out, child, depth);
    }
    out << "after string consts" << endl;
    //gen all global/top-level var decls
    for (astree* child: root->children) {
        if (child->symbol == '='
            && child->children[0]->children.size() != 0) {
            astree* type = child->children[0];
            astree* declid = type->children[0];
            if (declid->symbol == TOK_DECLID
                    && type->symbol != TOK_STRING) {
                out << get_type(type, nullptr) << " "
                    << get_name(type, *declid->lexinfo)
                    << ";" << endl;
            }
        }
    }
    out << "after global" << endl;
    //gen all functions
    for (astree* child: root->children) {
        if (child->symbol == TOK_FUNCTION) {
            format_function(out, child, depth);
        }
    }

    out << "void __ocmain (void)" << endl << "{" << endl;
    //gen all string consts
    for (astree* child: root->children) {
        if (child->symbol != TOK_FUNCTION
            && child->symbol != TOK_STRUCT)
            format(out, child, 1, nullptr);
    }
    out << "}" << endl;
    out << "end" <<endl;
}
