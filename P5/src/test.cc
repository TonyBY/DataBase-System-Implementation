#include "myGtest.h"

using namespace std;

int clear_pipe (Pipe &in_pipe, Schema *schema, bool print) {
	Record rec;
	int cnt = 0;
	std::cout << "Good" << std::endl;
	while (in_pipe.Remove (&rec)) {
		if (print) {
			rec.Print (schema);
		}
		cnt++;
	}
	std::cout << cnt << std::endl;
	return cnt;
}

std::string load_testcases (int idx) {
	std::ifstream in("all_test.txt");
	std::string content( (std::istreambuf_iterator<char>(in) ),
                       (std::istreambuf_iterator<char>()    ) );
	std::string s = content;
	std::string delimiter = ";";

	size_t pos = 0;
	std::string token;
	int i = 0;
	while ((pos = s.find(delimiter)) != std::string::npos) {
    	token = s.substr(0, pos);
		if (i == idx) {
			return token;
		}
		i++;
   		//std::cout << token << std::endl;
    	s.erase(0, pos + delimiter.length());
	}
	//std::cout << s << std::endl;
}

int main(int argc, char **argv) {
  	char* cnf[13] = {
		  "SELECT DISTINCT c1.c_name, c1.c_address, c1.c_acctbal \ 
FROM customer AS c1, customer AS c2 \
WHERE (c1.c_nationkey = c2.c_nationkey) AND \
	  (c1.c_name ='Customer#000070919')",
		  "SELECT DISTINCT s.s_name \
FROM supplier AS s, part AS p, partsupp AS ps \
WHERE (s.s_suppkey = ps.ps_suppkey) AND \
	  (p.p_partkey = ps.ps_partkey) AND \
	  (p.p_mfgr = 'Manufacturer#4') AND \
	  (ps.ps_supplycost < 350.0)",
		  "SELECT ps.ps_partkey, ps.ps_suppkey, ps.ps_availqty \
FROM partsupp AS ps \
WHERE (ps.ps_partkey < 100) AND (ps.ps_suppkey < 50)",
		  "SELECT SUM (ps.ps_supplycost) \
FROM part AS p, supplier AS s, partsupp AS ps \
WHERE (p.p_partkey = ps.ps_partkey) AND \
	  (s.s_suppkey = ps.ps_suppkey) AND \
	  (s.s_acctbal > 2500.0)",
	"SELECT s_suppkey, l_suppkey, l_orderkey, l_discount\
		FROM lineitem AS l \
		WHERE (l_orderkey < 10) AND (s_suppkey = l_suppkey)",
	" \
	SELECT SUM DISTINCT (l.l_discount) \
	FROM lineitem AS l, partsupp AS ps \
	WHERE (l.l_partkey = ps.ps_partkey) AND (l.l_returnflag = 'R') AND \ 
      	(l.l_discount < 0.04 OR l.l_shipmode = 'MAIL') GROUP BY l_discount, ps_suppkey  \
	",

	"SELECT SUM (ps.ps_supplycost), s.s_suppkey \
	FROM part AS p, supplier AS s, partsupp AS ps \ 
	WHERE (p.p_partkey = ps.ps_partkey) AND \
	  (s.s_suppkey = ps.ps_suppkey) \
	GROUP BY s.s_suppkey",
	
	"SELECT DISTINCT c1.c_name, c1.c_address, c1.c_acctbal \
FROM customer AS c1, customer AS c2 \
WHERE (c1.c_nationkey = c2.c_nationkey) AND \
	  (c1.c_name ='Customer#000070919')",

	  "SELECT SUM(l.l_discount) \
FROM customer AS c, orders AS o, lineitem AS l \
WHERE (c.c_custkey = o.o_custkey) AND \
      (o.o_orderkey = l.l_orderkey) AND \
      (c.c_name = 'Customer#000070919') AND \ 
	  (l.l_quantity > 30.0) AND (l.l_discount < 0.03)",

	  "SELECT l.l_orderkey \
FROM lineitem AS l \
WHERE (l.l_quantity > 30.0)",

"SELECT DISTINCT c.c_name \
FROM lineitem AS l, orders AS o, customer AS c, nation AS n, region AS r \ 
WHERE (l.l_orderkey = o.o_orderkey) AND \
      (o.o_custkey = c.c_custkey) AND \
	  (c.c_nationkey = n.n_nationkey) AND \
	  (n.n_regionkey = r.r_regionkey)",

"SELECT SUM (c.c_acctbal) \
FROM customer AS c, orders AS o \
WHERE (c.c_custkey = o.o_custkey) AND \
	  (o.o_totalprice < 10000.0)",

	  "SELECT l.l_orderkey, l.l_partkey, l.l_suppkey \
FROM lineitem AS l \
WHERE (l.l_returnflag = 'R') AND \ 
	  (l.l_discount < 0.04 OR l.l_shipmode = 'MAIL') AND \
	  (l.l_orderkey > 5000) AND (l.l_orderkey < 6000)"
	};

	int case_idx = atoi(argv[1]);
	//std::istringstream iss (std::string(argv[1]));
	//iss >> case_idx;
	std::string testcase = load_testcases(case_idx);
	std::cout << testcase << std::endl;

	yy_scan_string(testcase.c_str());
	yyparse();

	Statistics st;
	st.Read(Util::StatisticsFile);
	QueryPlan q(&st);
	q.createPlanTree();
	printQueryPlanTree((QueryPlanNode*)q.getRoot());
	//printInOrderQueryPlan((QueryPlanNode*)q.getRoot());
	//printQueryPlanTree((QueryPlanNode*)q.getRoot());
	//((QueryPlanNode*)(q.getRoot()))->print();
	q.execute();
	//q.~QueryPlan();
	/*
	yy_scan_string(cnf[0]);
	yyparse();

	Statistics st2;
	st2.Read(Util::StatisticsFile);
	QueryPlan q2(&st);
	q2.createPlanTree();
	printQueryPlanTree((QueryPlanNode*)q2.getRoot());
	printInOrderQueryPlan((QueryPlanNode*)q2.getRoot());
	//printQueryPlanTree((QueryPlanNode*)q.getRoot());
	//((QueryPlanNode*)(q.getRoot()))->print();
	q2.execute();
	*/
	//clear_pipe(*((Pipe*)(q.getOutPipe())), (Schema*) (q.getOutSchema()), true);
}

// 1. Should do something special for SUM DISTINCT (function)
// 2. Self join not support in Statistic
//