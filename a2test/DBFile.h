#ifndef DBFILE_H
#define DBFILE_H

#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"

typedef enum {heap, sorted, tree} fType;

class DBFile {

public:
	File file;
	
	DBFile (); 

	int Create (const char *fpath, fType file_type, void *startup);
	int Open (const char *fpath);
	int Close ();

	void Load (Schema &myschema, const char *loadpath);

	void MoveFirst ();
	void Add (Record &addme);
	int GetNext (Record &fetchme);
	int GetNext (Record &fetchme, CNF &cnf, Record &literal);
	
private: 
	//Record currentRecord;
	// Since the first page has no data, we only need to record the index of current page with data
	// So when currentDataPageIdx == 0, it means we are at the first DATA page, which is actually the second page in file.
	// currentDataPageIdx is 0-based.
	Page currentPage;
	off_t currentDataPageIdx;
	fType myType;
};
#endif
