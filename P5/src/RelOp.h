#ifndef REL_OP_H
#define REL_OP_H

#include <iostream>
#include <pthread.h>
#include <string>
#include <unistd.h>
#include "Pipe.h"
#include "DBFile.h"
#include "Record.h"
#include "Function.h"
#include "Schema.h"
#include "Util.h"

class RelationalOp {
	protected:
	pthread_t worker;

	public:
	// blocks the caller until the particular relational operator 
	// has run to completion
	
	// original 
	//virtual void WaitUntilDone () = 0;
	// Changed by yifan
	void WaitUntilDone () {pthread_join (worker, NULL);}

	// tell us how much internal memory the operation can use
	virtual void Use_n_Pages (int n) = 0;
};

class RunFunc {
	public:
		struct args_SelectFile {
	    	DBFile *inFile; 
			Pipe *outPipe; 
			CNF *selOp; 
			Record *literal;
			Schema *schema;
		};
		struct args_SelectPipe {
	    	Pipe *inPipe; 
			Pipe *outPipe; 
			CNF *selOp; 
			Record *literal;
			Schema *schema;
		};
		struct args_Project {
	    	Pipe *inPipe; 
			Pipe *outPipe; 
			int *keepMe;
			int numAttsInput;
			int numAttsOutput;
		};
		struct args_Join {
	    	Pipe *inPipeL; 
			Pipe *inPipeR; 
			Pipe *outPipe; 
			CNF *selOp; 
			Record *literal;
			// Need to be fixed
			int numAttsLeft;
			int numAttsRight;
			int *attsToKeep;
			int numAttsToKeep;
			int startOfRight;
		};
		struct args_DupRemoval {
	    	Pipe *inPipe; 
			Pipe *outPipe; 
			Schema *mySchema;
		};
		struct args_Sum {
	    	Pipe *inPipe; 
			Pipe *outPipe; 
			Function *func;
		};
		struct args_GroupBy {
	    	Pipe *inPipe; 
			Pipe *outPipe;
			OrderMaker *groupAtts; 
			Function *func;
		};
		struct args_WriteOut {
	    	Pipe *inPipe; 
			FILE *outFile; 
			Schema *mySchema;
		};
		static void SelectFile (void* raw_args);
		static void SelectPipe (void* raw_args);
		static void Project (void* raw_args);
		
		static void Join (void* raw_args);
		static void MergeJoin (struct args_Join* args, Pipe* bigq_outL, Pipe* bigq_outR, OrderMaker* orderL, OrderMaker* orderR);
		static void LoopJoin (struct args_Join* args);
		
		static void DuplicateRemoval (void* raw_args);
		static void Sum (void* raw_args);
		static void GroupBy (void* raw_args);
		
		static void WriteOut (void* raw_args);
		static void WriteRecord (Record* rec, FILE* outFile, Schema* mySchema);
		
};

class SelectFile : public RelationalOp { 

	private:
	// pthread_t worker;
	// Record *buffer;

	public:

	void Run (DBFile &inFile, Pipe &outPipe, CNF &selOp, Record &literal);
	//void WaitUntilDone ();
	void Use_n_Pages (int n) {return;}

};

class SelectPipe : public RelationalOp {
	public:
	void Run (Pipe &inPipe, Pipe &outPipe, CNF &selOp, Record &literal);
	//void WaitUntilDone ();
	void Use_n_Pages (int n) {return;}
};

class Project : public RelationalOp { 
	public:
	void Run (Pipe &inPipe, Pipe &outPipe, int *keepMe, int numAttsInput, int numAttsOutput);
	//void WaitUntilDone ();
	void Use_n_Pages (int n) {return;}
};

class Join : public RelationalOp { 
	public:
	void Run (Pipe &inPipeL, Pipe &inPipeR, Pipe &outPipe, CNF &selOp, Record &literal, 			
			int numAttsLeft,
			int numAttsRight,
			int *attsToKeep,
			int numAttsToKeep,
			int startOfRight );
	//void WaitUntilDone () { }
	void Use_n_Pages (int n) { return; }
};

class DuplicateRemoval : public RelationalOp {
	public:
	void Run (Pipe &inPipe, Pipe &outPipe, Schema &mySchema);
	//void WaitUntilDone () { }
	void Use_n_Pages (int n) { return; }
};
class Sum : public RelationalOp {
	public:
	void Run (Pipe &inPipe, Pipe &outPipe, Function &computeMe);
	//void WaitUntilDone () { }
	void Use_n_Pages (int n) { return; }
};
class GroupBy : public RelationalOp {
	public:
	void Run (Pipe &inPipe, Pipe &outPipe, OrderMaker &groupAtts, Function &computeMe);
	//void WaitUntilDone () { }
	void Use_n_Pages (int n) { return; }
};
class WriteOut : public RelationalOp {
	public:
	void Run (Pipe &inPipe, FILE *outFile, Schema &mySchema);
	//void WaitUntilDone () { }
	void Use_n_Pages (int n) { return; }
};
#endif
