#ifndef DBFILE_H
#define DBFILE_H

#include <fstream>
#include "BigQ.h"
#include "DBFBase.h"
#include "Defs.h"
#include "Pipe.h"
#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "Util.h"
 
void twoWayMerge(Pipe* pipe, DBFBase* sorted_file, DBFBase* out, OrderMaker* order);

void emptyFile(string fpath);

struct MetaInfo {
	fType fileType;
	int runLength;
	OrderMaker order; 
};

struct SortInfo { 
	OrderMaker *myOrder; 
	int runLength;
};

struct BigQSuite {
	BigQ* sorter;
	Pipe* inPipe;
	Pipe* outPipe;
};


class GenericDBFile {
	protected:
		fType m_fileType;
		//File* m_file;

		DBFBase* m_file; // = DBFBase(m_file);
		DBFBase* m_cache;
		string m_file_path;
		string m_cache_path;
	
		//Page m_currentPage;
		//off_t m_currentDataPageIdx;

	public:
		//virtual int Open (const char *fpath) {}
		virtual ~GenericDBFile() {}
		virtual int Create (const char *fpath, fType file_type, void *startup) {}
		virtual int Close () {}
		virtual void Load (Schema &myschema, const char *loadpath) {}
		virtual void MoveFirst () {}
		virtual void Add (Record &addme) {}
		virtual int GetNext (Record &fetchme) {}
		virtual int GetNext (Record &fetchme, CNF &cnf, Record &literal) {} 
		int GetNextNoFlush (Record &fetchme);

		fType getFileType();
		
};

class Heap : public virtual GenericDBFile {
	public:
		Heap(DBFBase* file, DBFBase* cache, string fpath, string cache_path);
		~Heap() {}
		//Heap(const char *fpath);
		//fType getFileType();
		int Create (const char *fpath, fType file_type, void *startup);
		//int Open (const char *fpath) {return m_fbase.Open(fpath);};
		int Close ();
		void Load (Schema &myschema, const char *loadpath);
		void MoveFirst ();
		void Add (Record &addme);
		int GetNext (Record &fetchme);
		int GetNext (Record &fetchme, CNF &cnf, Record &literal);
}; 

class Sorted : public virtual GenericDBFile {
	private:
		OrderMaker m_order;
		int m_runLength;
		fMode m_mode;
		BigQSuite m_sorter;
	
		OrderMaker queryOrder;
		bool continuousGetNext;

		void createSorter() {
    		m_sorter.inPipe = new Pipe(BigQBuffSize);
			m_sorter.outPipe = new Pipe(BigQBuffSize);
    		m_sorter.sorter = new BigQ(*(m_sorter.inPipe), *(m_sorter.outPipe), m_order, m_runLength);
  		}

  		void deleteSorter() {
    		delete m_sorter.inPipe; delete m_sorter.outPipe; delete m_sorter.sorter;
    		m_sorter.sorter = NULL; m_sorter.inPipe = m_sorter.outPipe = NULL;
		}
		
		void startReading() {
			if (m_mode == writing) {
				m_mode = reading;
				m_sorter.inPipe->ShutDown();
				m_cache->Close();
				emptyFile(m_cache_path);
				m_cache->Open(m_cache_path.c_str());
				m_cache->MoveFirst();
				twoWayMerge(m_sorter.outPipe, m_file, m_cache, &m_order);
				deleteSorter();
			}
		}

		void startWriting() {
			if (m_mode == reading) {
				m_mode = writing;
				createSorter();
			}
		}
		
		void moveCacheToFile() {
			//cout << "file length move: " << m_file->file->GetLength() << endl;
	        //cout << "cache length move: " << m_cache->file->GetLength() << endl;

			if (m_cache->file->GetLength() <= 1) {
				return;
			}
			m_file->Close();
			emptyFile(m_file_path);
			m_file->Open(m_file_path.c_str());	
			m_file->MoveFirst();
			m_cache->MoveFirst();
			Record record;
			while (m_cache->GetNext(record)) {
				m_file->Add(record);
			}
			m_file->MoveFirst();
			m_cache->Close();
			emptyFile(m_cache_path);
			m_cache->Open(m_cache_path.c_str());
			m_cache->MoveFirst();
			//cout << "file length After move: " << m_file->file->GetLength() << endl;
    	    //cout << "cache length After move: " << m_cache->file->GetLength() << endl;
		}
		
	public:
		Sorted() {}
		Sorted(OrderMaker myOrder, int runLength, DBFBase* file, DBFBase* cache,  string fpath, string cache_path);
		~Sorted() {}
		//fType getFileType();
		int Create (const char *fpath, fType file_type, void *startup);
		//int Open (const char *fpath);
		int Close ();
		void Load (Schema &myschema, const char *loadpath);
		void MoveFirst ();
		void Add (Record &addme);
		int GetNext (Record &fetchme);
		
		int GetNextByBinarySearch (Record &fetchme, CNF &cnf, Record &literal);
		int GetNext (Record &fetchme, CNF &cnf, Record &literal);
		//int Sorted::GetNextNoFlush (Record &fetchme);
		OrderMaker getOrder();
		
}; 

class DBFile {

public:
	//fstream* meta;
	DBFBase* m_file; 
	DBFBase* m_cache;
	string m_metaPath;
	MetaInfo m_metaInfo;

	DBFile (); 
	
	string metaInfoToStr(MetaInfo meta_info);
	MetaInfo readMetaInfo(string& meta_file);
	void writeMetaInfo(string& meta_file, MetaInfo& meta_info);

	int Open (const char *fpath);
	
	int Create (const char *fpath, fType file_type, void *startup);
	
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
	//Page currentPage;
	//off_t currentDataPageIdx;
	//fType myType;
	//File* m_file;
	
	string m_file_path;
	string m_cache_path;
	GenericDBFile* m_instance;
	
};
#endif
