CC = g++ -O0 -Wno-deprecated 

tag = -i

ifdef linux
tag = -n
endif

all: a5.out gTest.out

a5.out: Util.o Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFBase.o DBFile.o Pipe.o BigQ.o RelOp.o Function.o Statistics.o QueryOptimizer.o RDBMS.o y.tab.o lex.yy.o main.o
	$(CC) -o a5.out Util.o Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFBase.o DBFile.o Pipe.o BigQ.o RelOp.o Function.o Statistics.o QueryOptimizer.o  RDBMS.o y.tab.o lex.yy.o main.o -lfl -lpthread

gTest.out: Util.o Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFBase.o DBFile.o Pipe.o BigQ.o RelOp.o Function.o Statistics.o QueryOptimizer.o y.tab.o yyfunc.tab.o lex.yy.o lex.yyfunc.o gTest.o
	$(CC) -o gTest.out Util.o Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFBase.o DBFile.o Pipe.o BigQ.o RelOp.o Function.o Statistics.o QueryOptimizer.o y.tab.o yyfunc.tab.o lex.yy.o lex.yyfunc.o gTest.o -lfl -lpthread -lgtest

main.o: main.cc
	$(CC) -g -c main.cc

gTest.o: gTest.cc
	$(CC) -g -c gTest.cc

RDBMS.o: RDBMS.cc
	$(CC) -g -c RDBMS.cc

QueryOptimizer.o: QueryOptimizer.cc
	$(CC) -g -c QueryOptimizer.cc

Util.o: Util.cc
	$(CC) -g -c Util.cc

Statistics.o: Statistics.cc
	$(CC) -g -c Statistics.cc

Comparison.o: Comparison.cc
	$(CC) -g -c Comparison.cc
	
ComparisonEngine.o: ComparisonEngine.cc
	$(CC) -g -c ComparisonEngine.cc
	
DBFBase.o: DBFBase.cc
	$(CC) -g -c DBFBase.cc

DBFile.o: DBFile.cc
	$(CC) -g -c DBFile.cc

Pipe.o: Pipe.cc
	$(CC) -g -c Pipe.cc

BigQ.o: BigQ.cc
	$(CC) -g -c BigQ.cc

RelOp.o: RelOp.cc
	$(CC) -g -c RelOp.cc

Function.o: Function.cc
	$(CC) -g -c Function.cc

File.o: File.cc
	$(CC) -g -c File.cc

Record.o: Record.cc
	$(CC) -g -c Record.cc

Schema.o: Schema.cc
	$(CC) -g -c Schema.cc
	
y.tab.o: Parser.y
	yacc -d Parser.y
	sed $(tag) y.tab.c -e "s/  __attribute__ ((__unused__))$$/# ifndef __cplusplus\n  __attribute__ ((__unused__));\n# endif/" 
	g++ -c y.tab.c

yyfunc.tab.o: ParserFunc.y
	yacc -p "yyfunc" -b "yyfunc" -d ParserFunc.y
	g++ -c yyfunc.tab.c

lex.yy.o: Lexer.l
	lex  Lexer.l
	gcc  -c lex.yy.c

lex.yyfunc.o: LexerFunc.l
	lex -Pyyfunc LexerFunc.l
	gcc  -c lex.yyfunc.c

clean: 
	rm -f *.o
	rm -f *.out
	rm -f y.tab.c
	rm -f lex.yy.c
	rm -f y.tab.h
	rm -f *.stat
	rm -f *.tmp
	rm -f intermediate_catalog_*
	rm -f answers_catalog
	rm -f *.bin.*