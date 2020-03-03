#include "gTest.h"

TEST(DBFile, metaInfoToStr) {
	DBFile db = DBFile();
	OrderMaker order = OrderMaker();
	MetaInfo meta = {
		sorted,
		100,
		order
	};
	string mete_string = "sorted\n100\n0";
	ASSERT_EQ(mete_string, db.metaInfoToStr(meta));
}

TEST(DBFile, readMetaInfo) {
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

TEST(DBFile, Open) {
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
	ASSERT_EQ(db.Open(string("test_open").c_str()), 1);
	db.Close();
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


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}