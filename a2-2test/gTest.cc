#include "gTest.h"

extern "C" {
	typedef struct yy_buffer_state *YY_BUFFER_STATE;
	int yyparse(void);   // defined in y.tab.c
	YY_BUFFER_STATE yy_scan_string (const char *yy_str);
	int yylex_destroy (void );
}

extern struct AndList *final;

void get_sort_order (OrderMaker &sortorder, const string& cnf_str) {
	yy_scan_string(cnf_str.c_str());
	if (yyparse() != 0) {
		cout << "Can't parse your sort CNF.\n";
		exit (1);
	}
	cout << " \n";
	Record literal;
	CNF sort_pred;
	OrderMaker dummy;
	sort_pred.GetSortOrders (sortorder, dummy);
}

TEST(metaInfoToStr, metaInfoToStr) {
	DBFile db = DBFile();
	OrderMaker order = OrderMaker();
	MetaInfo meta = {
		sorted,
		100,
		order
	};
	cout << db.metaInfoToStr(meta) << endl;
	ASSERT_EQ(1, 1);
}

TEST(metaInfoReadWrite, metaInfoReadWrite_Sorted) {
	DBFile db = DBFile();
	OrderMaker order = OrderMaker();
	MetaInfo meta = {
		sorted,
		100,
		order
	};
	
	string meta_str = db.metaInfoToStr(meta);
	string meta_file = "test_meta";
	db.writeMetaInfo(meta_file, meta);
	MetaInfo read = db.readMetaInfo(meta_file);
	ASSERT_EQ(meta_str, db.metaInfoToStr(read));

}

TEST(metaInfoReadWrite, metaInfoReadWrite_Heap) {
	DBFile db = DBFile();
	OrderMaker order = OrderMaker();
	MetaInfo meta = {
		heap,
		-1,
		order
	};
	
	string meta_str = db.metaInfoToStr(meta);
	string meta_file = "test_meta";
	db.writeMetaInfo(meta_file, meta);
	MetaInfo read = db.readMetaInfo(meta_file);
	ASSERT_EQ(meta_str, db.metaInfoToStr(read));

}

TEST(Open, Open) {
	DBFile db = DBFile();
	OrderMaker order = OrderMaker();
	MetaInfo meta = {
		heap,
		100,
		order
	};
	
	string meta_str = db.metaInfoToStr(meta);
	string meta_file = "test_open.meta";
	db.writeMetaInfo(meta_file, meta);
	db.Open(string("test_open").c_str());
	db.Close();
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
	db.Create(fpath, heap, NULL);
	db.Close();
	ASSERT_EQ(1, 1);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}