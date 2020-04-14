#ifndef DBFBASE_H
#define DBFBASE_H

#include "BigQ.h"
#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"

enum fType {heap, sorted, tree};
enum fMode {reading, writing};

const string fTypeStr[] = {string("heap"), string("sorted"), string("tree")};
const int BigQBuffSize = 1000;

class DBFBase {

public:
	File* file;
	//File* cache;

	Page currentPage;
	off_t currentDataPageIdx;
	fType myType;

	//OrderMaker queryOrder;
	//bool continuousGetNext;

	DBFBase (); 
	//DBFBase (File* file);
	~DBFBase();

	off_t lastDataPageIdx() {
		off_t cur_len = file->GetLength();
    	off_t last_data_page_idx = cur_len - 2;
		return last_data_page_idx;
	}

	int Create (const char *fpath, fType file_type, void *startup);
	int Open (const char *fpath);
	int Close ();

	void Load (Schema &myschema, const char *loadpath);

	void MoveFirst ();
	void Add (Record &addme);
	int GetNext (Record &fetchme);
	int GetNext (Record &fetchme, CNF &cnf, Record &literal);

	//int isNextMatching (Record &fetchme, CNF &cnf, Record &literal);
	//int GetNextByLinearScan (Record &fetchme, CNF &cnf, Record &literal);
	//int GetNextByBinarySearch (Record &fetchme, CNF &cnf, Record &literal);

	//int GetNext4Heap (Record &fetchme, CNF &cnf, Record &literal);
	//int GetNext4Sorted (Record &fetchme, CNF &cnf, Record &literal);
	
private: 
	//Record currentRecord;
	// Since the first page has no data, we only need to record the index of current page with data
	// So when currentDataPageIdx == 0, it means we are at the first DATA page, which is actually the second page in file.
	// currentDataPageIdx is 0-based.
	
};
#endif
