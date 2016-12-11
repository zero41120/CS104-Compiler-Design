#-----------------------------------------------------------------
#  Makefile for CMPS 104 Programming Assignment 5
#-----------------------------------------------------------------

MKFILE         = Makefile
MODULES        = string_set auxlib lyutils astree type_checker oil
LSOURCES       = scanner.l
YSOURCES       = parser.y
CLGEN          = yylex.cpp
HYGEN          = yyparse.h
CYGEN          = yyparse.cpp
LREPORT        = yylex.output
YREPORT        = yyparse.output
HEADERS        = ${MODULES:=.h}
SOURCES        = ${MODULES:=.cpp} main.cpp
OTHERS         = ${LSOURCES} ${YSOURCES}
OBJECTS        = ${SOURCES:.cpp=.o} ${CLGEN:.cpp=.o} ${CYGEN:.cpp=.o}
DEPENDS        = ${MKFILE}.dep
ALLSOURCES     = ${HEADERS} ${SOURCES} ${MKFILE} ${OTHERS}

SUBMITFILES    = ${ALLSOURCES} README
COMPILE        = g++ -std=gnu++14 -g -O0 -Wall -Wextra
MAKEDEP        = g++ -std=gnu++14 -MM
NOINCL         = ci clean spotless

LINK           = gcc -o
REMOVE         = rm -f
MEMCHECK       = valgrind --leak-check=full

EXECBIN        = oc

all : ${CLGEN} ${CYGEN} ${HYGEN} ${EXECBIN} 

${EXECBIN} : ${OBJECTS}
	${COMPILE} -o ${EXECBIN} ${OBJECTS}

%.o : %.cpp
	${COMPILE} -c $<


${CLGEN} : ${LSOURCES}
	flex --outfile=${CLGEN} ${LSOURCES} 2>${LREPORT}
	- grep -v '^  ' ${LREPORT}

${CYGEN} ${HYGEN} : ${YSOURCES}
	bison --defines=${HYGEN} --output=${CYGEN} ${YSOURCES}

clean :
	- rm ${OBJECTS} 

spotless :
	- rm ${OBJECTS} ${EXECBIN} ${CLGEN} ${CYGEN} ${HYGEN}
	- rm *.str *.tok *.ast *.sym *.oil
	- rm *.output *.err *.out *.log *.oc

ci : 
	- checksource ${ALLSOURCES}

dep : ${SOURCES} ${HEADERS}
	@ echo "# ${DEPENDS} created `LC_TIME=C date`" >${DEPENDS}
	${MAKEDEP} ${SOURCES} >>${DEPENDS}

preview :
	echo $(SUBMITFILES)
submit : 
	submit cmps104a-wm.f16 asg4 $(SUBMITFILES)

