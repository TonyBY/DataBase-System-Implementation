CC = g++ -O2 -Wno-deprecated

ifeq ($(UNAME_S),Linux)
tag = -n
else
tag = -i
endif

ifeq ($(UNAME_S),Linux)
flag = -lfl
else
flag = -ll
endif

UNAME_S := $(shell uname -s)

test.out: Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o y.tab.o lex.yy.o test.o
	$(CC) -o test.out Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o y.tab.o lex.yy.o test.o $(flag)

ifeq ($(UNAME_S),Linux)
gTest: Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o y.tab.o lex.yy.o gTest.o
	$(CC) -std=c++17 -I h -pthread -o gTest Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o y.tab.o lex.yy.o gTest.o /usr/lib/libgtest.a $(flag)
else
gTest: Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o y.tab.o lex.yy.o gTest.o
	$(CC) -std=c++17 -I h -pthread -o gTest Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o y.tab.o lex.yy.o gTest.o /usr/local/lib/libgtest.a $(flag)
endif

main: Record.o Comparison.o ComparisonEngine.o Schema.o File.o y.tab.o lex.yy.o main.o
	$(CC) -o main Record.o Comparison.o ComparisonEngine.o Schema.o File.o y.tab.o lex.yy.o main.o $(flag)
	
test.o: test.cc
	$(CC) -g -c test.cc

ifeq ($(UNAME_S),Linux)
gTest.o: gTest.cpp
	g++ -std=c++17 -Wall -I h -I /usr/include -c gTest.cpp -o gTest.o
else
gTest.o: gTest.cpp
	g++ -std=c++17 -Wall -I h -I /usr/local/include -c gTest.cpp -o gTest.o
endif

main.o: main.cc
	$(CC) -g -c main.cc
	
Comparison.o: Comparison.cc
	$(CC) -g -c Comparison.cc
	
ComparisonEngine.o: ComparisonEngine.cc
	$(CC) -g -c ComparisonEngine.cc
	
DBFile.o: DBFile.cc
	$(CC) -g -c DBFile.cc

File.o: File.cc
	$(CC) -g -c File.cc

Record.o: Record.cc
	$(CC) -g -c Record.cc

Schema.o: Schema.cc
	$(CC) -g -c Schema.cc

ifdef linux
y.tab.o: Parser.y
	yacc -d Parser.y
	sed $(tag) y.tab.c -e "s/  __attribute__ ((__unused__))$$/# ifndef __cplusplus\n  __attribute__ ((__unused__));\n# endif/" 
	g++ -c y.tab.c
else
y.tab.o: Parser.y
	yacc -d Parser.y
	sed $(tag) -e "s/  __attribute__ ((__unused__))$$/# ifndef __cplusplus\n  __attribute__ ((__unused__));\n# endif/" y.tab.c
	g++ -c y.tab.c
endif

lex.yy.o: Lexer.l
	lex  Lexer.l
	gcc  -c lex.yy.c

clean: 
	rm -f *.o
	rm -f *.out
	rm -f y.tab.c
	rm -f lex.yy.c
	rm -f y.tab.h
	rm -f main
	rm -f gTest
