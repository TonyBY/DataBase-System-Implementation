#include "myGtest.h"

using namespace std;

int main() {
//   	char* cnf[6] = {
// 	" \
// 	SELECT SUM DISTINCT (l.l_discount) \
// 	FROM lineitem AS l, partsupp AS ps \
// 	WHERE (l.l_partkey = ps.ps_partkey) AND (l.l_returnflag = 'R') AND \ 
//       	(l.l_discount < 0.04 OR l.l_shipmode = 'MAIL') GROUP BY l_discount, ps_suppkey  \
// 	",

// 	"SELECT SUM (ps.ps_supplycost), s.s_suppkey \
// 	FROM part AS p, supplier AS s, partsupp AS ps \ 
// 	WHERE (p.p_partkey = ps.ps_partkey) AND \
// 	  (s.s_suppkey = ps.ps_suppkey) \
// 	GROUP BY s.s_suppkey",
	
// 	"SELECT DISTINCT c1.c_name, c1.c_address, c1.c_acctbal \
// FROM customer AS c1, customer AS c2 \
// WHERE (c1.c_nationkey = c2.c_nationkey) AND \
// 	  (c1.c_name ='Customer#000070919')",

// 	  "SELECT SUM(l.l_discount) \
// FROM customer AS c, orders AS o, lineitem AS l \
// WHERE (c.c_custkey = o.o_custkey) AND \
//       (o.o_orderkey = l.l_orderkey) AND \
//       (c.c_name = 'Customer#000070919') AND \ 
// 	  (l.l_quantity > 30.0) AND (l.l_discount < 0.03)",

// 	  "SELECT l.l_orderkey \
// FROM lineitem AS l \
// WHERE (l.l_quantity > 30.0)",

// "SELECT DISTINCT c.c_name \
// FROM lineitem AS l, orders AS o, customer AS c, nation AS n, region AS r \ 
// WHERE (l.l_orderkey = o.o_orderkey) AND \
//       (o.o_custkey = c.c_custkey) AND \
// 	  (c.c_nationkey = n.n_nationkey) AND \
// 	  (n.n_regionkey = r.r_regionkey)",

// 	};

// 	yy_scan_string(cnf[5]);
	yyparse();

	Statistics st;
	st.Read(Util::StatisticsFile);
	QueryPlan q(&st);
	q.createPlanTree();
	printQueryPlanTree((QueryPlanNode*)q.getRoot());
	printInOrderQueryPlan((QueryPlanNode*)q.getRoot());
	//printQueryPlanTree((QueryPlanNode*)q.getRoot());
	//((QueryPlanNode*)(q.getRoot()))->print();
}

// 1. Should do something special for SUM DISTINCT (function)
// 2. Self join not support in Statistic
//