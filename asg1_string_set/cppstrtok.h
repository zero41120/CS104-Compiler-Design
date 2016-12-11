// $Id: string_set.h,v 1.2 2016-08-18 15:13:48-07 - - $

#ifndef __CPPSTRTOK__
#define __CPPSTRTOK__

#include <string>
using namespace std;

#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>

const string CPP = "/usr/bin/cpp";
constexpr size_t LINESIZE = 1024;

#include "stringset.h"


void chomp (char* string, char delim);
void cpplines (FILE* pipe, char* filename, int show, string_set &set);

#endif

