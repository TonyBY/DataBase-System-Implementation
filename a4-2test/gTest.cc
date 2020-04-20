#include "gTest.h"

using namespace std;


TEST(QueryPlan, booleanToString) {
	char *cnf = "SELECT s_suppkey,ps_suppkey FROM a AS b WHERE ( a.s_suppkey = b.ps_suppkey ) AND ( s_suppkey < 10 OR s_suppkey > 3 OR ps_suppkey < 9 )";
	yy_scan_string(cnf);
	yyparse();
	cout << Util::ParseTreeToString(boolean) << endl;
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

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}