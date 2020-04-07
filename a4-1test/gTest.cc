#include "gTest.h"

TEST(Statistics, Write) {
	Statistics stat;
	stat.AddRel("testRel", 100);
	stat.AddAtt("testRel", "testAtt1", -1);
	stat.AddAtt("testRel", "testAtt2", 50);
	stat.Write("./testWrite.stat");
	ASSERT_EQ(1, 1);
}

TEST(Statistics, Read) {
	Statistics stat, stat_empty;
	stat.AddRel("testRel4Read", 100);
	stat.AddAtt("testRel4Read", "testAtt1", -1);
	stat.AddAtt("testRel4Read", "testAtt2", 50);
	stat.Write("./testRead.stat");

	stat_empty.Read("./testRead.stat");
	
	ASSERT_EQ(stat_empty.data["testRel4Read"].numTuples, 100);
	ASSERT_EQ(stat_empty.data["testRel4Read"].data["testAtt1"].numDistincts, 100);
	ASSERT_EQ(stat_empty.data["testRel4Read"].data["testAtt2"].numDistincts, 50);
}

TEST(Statistics, CopyRel) {
	Statistics stat;
	stat.AddRel("test", 100);
	stat.AddAtt("test", "testAtt1", -1);
	stat.AddAtt("test", "testAtt2", 50);
	stat.CopyRel("test", "new_test");

	ASSERT_EQ(stat.data["new_test"].name, "new_test");
	ASSERT_EQ(stat.data["new_test"].numTuples, 100);
	ASSERT_EQ(stat.data["new_test"].data["testAtt1"].numDistincts, 100);
	ASSERT_EQ(stat.data["new_test"].data["testAtt2"].numDistincts, 50);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}