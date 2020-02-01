#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "DBFile.h"
#include "Defs.h"
#include <iostream>

// stub file .. replace it with your own DBFile.cc

DBFile::DBFile (): whichPage(0) {

}

DBFile::~DBFile() {

}

int DBFile::Create (const char *f_path, fType myType, void *startup) {
    dbFile.Open(0, (char *)f_path);
    return 1;
}

void DBFile::Load (Schema &f_schema, const char *loadpath) {
    FILE *tableFile = fopen(loadpath, "r");
    if (!tableFile) {
		cerr << "BAD: can't open the .tbl file." << endl;
    }
    Record curr; 
    while (curr.SuckNextRecord(&f_schema, tableFile)) {
        Add(curr);
    }
    if (dbFile.GetLength()) {
        dbFile.AddPage(&currPage, dbFile.GetLength() - 1);
    } else {
        dbFile.AddPage(&currPage, 0);
    }

}


int DBFile::Open (const char *f_path) {
    dbFile.Open(1, (char *)f_path);
    return 1;
}

void DBFile::MoveFirst () {
    whichPage = 0;
    dbFile.GetPage(&currPage, whichPage);
}

int DBFile::Close () {
    dbFile.Close();
    return 1;
}

void DBFile::Add (Record &rec) {
    if (!currPage.Append(&rec)) {
        if (dbFile.GetLength()) {
            dbFile.AddPage(&currPage, dbFile.GetLength() - 1);
        } else {
            dbFile.AddPage(&currPage, 0);
        }

        currPage.EmptyItOut();
        currPage.Append(&rec);
    }
}

int DBFile::GetNext (Record &fetchme) {
    while (!currPage.GetFirst(&fetchme)) {
        whichPage++;

        if (whichPage > dbFile.GetLength() - 2) {
            return 0;// if the page index overflows.
        } else {
            dbFile.GetPage(&currPage, whichPage);
        } 
    }

    return 1;
}


int DBFile::GetNext (Record &fetchme, CNF &cnf, Record &literal) {
    ComparisonEngine comp;
    while (GetNext(fetchme)) {
        if (comp.Compare(&fetchme, &literal, &cnf)) {
            return 1;        
        }
    }
    return 0;
}
