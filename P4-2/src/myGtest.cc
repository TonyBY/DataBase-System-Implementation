#include "myGtest.h"

using namespace std;


TEST(QueryPlan, booleanToString) {
	char *cnf = "SELECT s_suppkey,ps_suppkey FROM a AS b WHERE ( a.s_suppkey = b.ps_suppkey ) AND ( s_suppkey < 10 OR s_suppkey > 3 OR ps_suppkey < 9 )";
	yy_scan_string(cnf);
	yyparse();
	cout << Util::ParseTreeToString(boolean) << endl;
	ASSERT_EQ(1, 1);
}

TEST(QueryPlan, splitAndList) {
	char *cnf = "SELECT s_suppkey,ps_suppkey FROM a AS b WHERE ( s_suppkey = ps_suppkey ) AND ( s_suppkey < 10 OR s_suppkey > 3 ) AND ( ps_suppkey < 9 )";
	yy_scan_string(cnf);
	yyparse();
	vector<AndList*> single_lists = Util::splitAndList(boolean);
	cout << Util::ParseTreeToString(boolean) << endl << endl;
	for (int i = 0; i < single_lists.size(); i++) {
		cout << Util::ParseTreeToString(single_lists[i]) << endl;
	}
	ASSERT_EQ(1, 1);
}

TEST(QueryPlan, getPrefix) {
	ASSERT_EQ(Util::getPrefix("ps_suppkey", '_'), "ps");
}

TEST(QueryPlan, splitString) {
	vector<string> res = Util::splitString("supplier&lineitem&region&nation", '&');
	Util::printVector<string>(res);
	ASSERT_EQ(1, 1);
}


TEST(QueryPlan, getRelName) {
	char *cnf = "SELECT s_suppkey,ps_suppkey FROM a AS b WHERE ( s_suppkey = ps_suppkey ) AND ( s_suppkey < 10 OR s_suppkey > 3 ) AND ( ps_suppkey < 9 )";
	yy_scan_string(cnf);
	yyparse();

	Statistics st;
	st.Read(Util::StatisticsFile);
	QueryPlan q(&st);

	vector<AndList*> single_lists = Util::splitAndList(boolean);
	cout << Util::ParseTreeToString(boolean) << endl << endl;
	for (int i = 0; i < single_lists.size(); i++) {
		cout << Util::ParseTreeToString(single_lists[i]) << " Table: ";
		std::vector<std::string> rels;// = q.getRelNames(single_lists[i]);
		for (int j = 0; j < rels.size(); j++) {
			cout << rels[j] << ", ";
		}
		cout << endl;
	}
	ASSERT_EQ(1, 1);
}

TEST(QueryPlan, getParseTreeFromVector) {
	char *cnf = "SELECT s_suppkey,ps_suppkey FROM a AS b WHERE ( a.s_suppkey = b.ps_suppkey ) AND ( s_suppkey < 10 OR s_suppkey > 3 ) AND ( ps_suppkey < 9 )";
	yy_scan_string(cnf);
	yyparse();
	vector<AndList*> single_lists = Util::splitAndList(boolean);
	cout << Util::ParseTreeToString(boolean) << endl << endl;
	for (int i = 0; i < single_lists.size(); i++) {
		cout << Util::ParseTreeToString(single_lists[i]) << endl;
	}
	AndList* reconstruct = Util::getParseTreeFromVector(single_lists);
	cout << "Re-construct AndList from vector: " << std::endl;
	cout << Util::ParseTreeToString(reconstruct) << endl;
	ASSERT_EQ(1, 1);
}

TEST(QueryPlan, estimateTotalCost) {
	//char *cnf = "SELECT s_suppkey FROM a AS b WHERE (l_orderkey = o_orderkey) AND \
      (o_custkey = c_custkey) AND \
	  (c_nationkey = n_nationkey) AND \
	  (n_regionkey = r_regionkey)";
	char* cnf = "SELECT l.l_orderkey, l.l_partkey, l.l_suppkey \
		FROM lineitem AS l \
		WHERE (l.l_returnflag = 'R') AND \ 
      		(l.l_discount < 0.04 OR l.l_shipmode = 'MAIL')";
	yy_scan_string(cnf);
	yyparse();

	Statistics st;
	st.Read(Util::StatisticsFile);
	QueryPlan q(&st);
	q.optimizePredicate();

	ASSERT_EQ(1, 1);
}

TEST(QueryPlan, createQueryPlanTree1) {
	//char *cnf = "SELECT s_suppkey FROM a AS b WHERE (l_orderkey = o_orderkey) AND \
      (o_custkey = c_custkey) AND \
	  (c_nationkey = n_nationkey) AND \
	  (n_regionkey = r_regionkey)";
	char* cnf = "SELECT l.l_orderkey, l.l_partkey, l.l_suppkey \
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

	ASSERT_EQ(1, 1);
}

TEST(QueryPlan, createQueryPlanTree2) {
	//char *cnf = "SELECT s_suppkey FROM a AS b WHERE (l_orderkey = o_orderkey) AND \
      (o_custkey = c_custkey) AND \
	  (c_nationkey = n_nationkey) AND \
	  (n_regionkey = r_regionkey)";
	char* cnf = "SELECT SUM (l.l_discount*2) \
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
	((QueryPlanNode*)q.getRoot())->print();
	ASSERT_EQ(1, 1);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}