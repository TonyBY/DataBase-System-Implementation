#include <fstream>

// #include "TwoWayList.h"
// #include "Record.h"
// #include "Schema.h"
// #include "File.h"
// #include "Comparison.h"
// #include "ComparisonEngine.h"
#include "DBFile.h"
#include "HeapFile.h"
// #include "Defs.h"

// stub file .. replace it with your own DBFile.cc

using std::string;
using std::ifstream;
using std::ofstream;

int DBFile::Create (char* fpath, fType myType, void* startup) {
    createFile(myType);
    return db->Create(fpath, startup);
}

void DBFile::Load (Schema& myschema, char* loadpath) {
    return db->Load(myschema, loadpath);
}

int DBFile::Close () {
    return db->Close();
}

int DBFile::Open (char* fpath) {
  int ftype = HEAP;  // use heap file by default
  string metaFile = DBFileBase::getTableName(fpath)+".meta";
  ifstream ifs(metaFile.c_str());
  if (ifs) {
    ifs >> ftype;  // the first line contains file type
    ifs.close();
  }
  createFile(static_cast<fType>(ftype));
  return db->Open(fpath);
}

void DBFile::MoveFirst () {
    return db->MoveFirst();
}

void DBFile::Add (Record &rec) {
    return db->Add(rec);
}

int DBFile::GetNext (Record &fetchme) {
  return db->GetNext(fetchme);
}


int DBFile::GetNext (Record &fetchme, CNF &cnf, Record &literal) {
    return db->GetNext(fetchme, cnf, literal);
}

void DBFile::createFile(fType myType) {
    switch (myType) {
        case HEAP: db = new HeapFile; break;
        // case SORTED: db = new SortedFile; break;
        default: db = NULL;
  }
}

DBFile::DBFile(): db(NULL) {}
DBFile::~DBFile() { delete db; }

int DBFileBase::Create(char* fpath, void* startup) {
    theFile.Open(0, fpath);
    return 1;
}

int DBFileBase::Open (char* fpath) {
    theFile.Open(1, fpath);
    return 1;
}

void DBFileBase::Load (Schema& myschema, char* loadpath) {
    startWrite();
    FILE* ifp = fopen(loadpath, "r");

    Record next;
    curPage.EmptyItOut();  // creates the first page
    while (next.SuckNextRecord(&myschema, ifp)) Add(next);
    // theFile.addPage(&curPage);  // writes the last page  -- added in close()
}

int DBFileBase::GetNext (Record& fetchme) {
    while (!curPage.GetFirst(&fetchme)) {
        if(++curPageIdx > theFile.lastIndex()) return 0;  // no more records
        theFile.GetPage(&curPage, curPageIdx);
    }
    return 1;
}