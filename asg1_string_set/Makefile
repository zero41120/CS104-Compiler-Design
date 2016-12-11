#-----------------------------------------------------------------
#  Makefile for CMPS 104 Programming Assignment 1
#-----------------------------------------------------------------

MKFILE         = Makefile
MODULES        = stringset auxlib cppstrtok 
HEADERS        = ${MODULES:=.h}
SOURCES        = ${MODULES:=.cpp} main.cpp
OBJECTS        = ${SOURCES:.cpp=.o}
DEPENDS        = ${MKFILE}.dep
ALLSOURCES     = ${HEADERS} ${SOURCES} ${MKFILE} main.cpp

SUBMIT_FILES   = ${ALLSOURCES} README
COMPILE        = g++ -std=gnu++14 -g -O0 -Wall -Wextra
MAKEDEP        = g++ -std=gnu++14 -MM
NOINCL         = ci clean spotless

LINK           = gcc -o
REMOVE         = rm -f
MEMCHECK       = valgrind --leak-check=full

EXECBIN        = oc

all : ${EXECBIN}

${EXECBIN} : ${OBJECTS}
	${COMPILE} -o $@ ${OBJECTS}

%.o : %.cpp
	${COMPILE} -c $<

clean :
	- rm ${OBJECTS} 

spotless :
	- rm ${OBJECTS} ${EXECBIN}

ci : 
	- checksource ${ALLSOURCES}

dep : ${SOURCES} ${HEADERS}
	@ echo "# ${DEPENDS} created `LC_TIME=C date`" >${DEPENDS}
	${MAKEDEP} ${SOURCES} >>${DEPENDS}

submit : 
	submit cmps104a-wm.f16 asg1 $(SUBMIT_FILES)

