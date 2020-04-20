#include "gTest.h"

using namespace std;

int main() {
  	//char* cnf = "SELECT l.l_orderkey, l.l_partkey, l.l_suppkey \
		FROM lineitem AS l \
		WHERE (l.l_returnflag = 'R') AND \ 
    //  		(l.l_discount < 0.04 OR l.l_shipmode = 'MAIL')";
	char* cnf = "SELECT SUM (l.l_discount*3+2) \
		FROM lineitem AS l \
		WHERE (l.l_returnflag = 'R') AND \ 
      		(l.l_discount < 0.04 OR l.l_shipmode = 'MAIL')";
	yy_scan_string(cnf);
	yyparse();

	Statistics st;
	st.Read(Util::StatisticsFile);
	QueryPlan q(&st);
	q.createPlanTree();
	printQueryPlanTree((QueryPlanNode*)q.getRoot());
	((QueryPlanNode*)(q.getRoot()))->print();
}