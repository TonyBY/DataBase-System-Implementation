//
// Created by Tony's Macbook pro on 2020-02-02.
//

//
// Created by Tony's Macbook pro on 2020-02-02.
//
#include <iostream>
#include "gtest/gtest.h"
#include "DBFile.h"
#include "gTest.h"

// make sure that the file path/dir information below is correct
const char *dbfile_dir = "../../data/bin/heap/"; // dir where binary heap files should be stored
const char *tpch_dir ="../../data/tpch-dbgen/"; // dir where dbgen tpch files (extension *.tbl) can be found
const char *catalog_path = "../src/catalog"; // full path of the catalog file

relation *rel;

TEST(DBFileTest, test1) {
    //arrange
    //act
    //assert

    setup (catalog_path, dbfile_dir, tpch_dir);
    relation *rel_ptr[] = {n, r, c, p, ps, o, li, s};
    rel = rel_ptr [2];
    DBFile dbfile;

    EXPECT_EQ (dbfile.Create (rel->path(), heap, NULL), 1);

    char tbl_path[100]; // construct path of the tpch flat text file
    sprintf (tbl_path, "%s%s.tbl", tpch_dir, rel->name());
    dbfile.Load (*(rel->schema ()), tbl_path);

    EXPECT_EQ (dbfile.Close (),  1);
    EXPECT_EQ (dbfile.Open (rel->path()),  1);
    EXPECT_EQ (dbfile.Close (),  1);
}

int main(int argc, char **argv) {

    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
