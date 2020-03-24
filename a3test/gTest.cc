#include "gTest.h"



Pipe inPipe, outPipe;

TEST(RelOp, Project) {
	
	Project p;
	int keepMe[3] = {1, 2, 4};
	p.Run(inPipe, outPipe, keepMe, 10, 3);

	ASSERT_EQ(1, 1);
}

TEST(RelOp, DuplicateRemoval) {
	
	DuplicateRemoval d;
	Attribute att = {"att", Int};
	Schema sch ("sch", 1, &att);
	d.Run(inPipe, outPipe, sch);

	ASSERT_EQ(1, 1);
}

TEST(Heap, Create) {
	DBFile db = DBFile();
	
	OrderMaker order = OrderMaker();
	MetaInfo meta = {
		heap,
		100,
		order
	};
	const char *fpath = string("test_create").c_str();
	ASSERT_EQ(db.Create(fpath, heap, NULL), 1);
	db.Close();
}


TEST(RelOp, WriteOut) {
	
	WriteOut w;
	char *fwpath = "ps.w.tmp";
	FILE *writefile = fopen (fwpath, "w");
	Attribute att = {"att", Int};
	Schema sch ("sch", 1, &att);
	w.Run(inPipe, writefile, sch);

	ASSERT_EQ(1, 1);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}