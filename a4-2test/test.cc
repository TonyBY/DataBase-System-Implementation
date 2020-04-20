#include "gTest.h"

using namespace std;

int main() {
	
	yyparse();

	Statistics st;
	st.Read(Util::StatisticsFile);
	QueryPlan q(&st);
	std::cout << "enter:" << std::endl;
	q.createPlanTree();
	// printQueryPlanTree((QueryPlanNode*)q.getRoot());
	std::cout << "PRINTING TREE IN ORDER:" << std::endl;
	std::cout << std::endl;
	// printInOrderQueryPlan((QueryPlanNode*)q.getRoot());
	printLevelOrder((QueryPlanNode*)q.getRoot());
	// printInOrderQueryPlan_BFS((QueryPlanNode*)q.getRoot());
}
