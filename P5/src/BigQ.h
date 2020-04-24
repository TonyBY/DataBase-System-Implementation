#ifndef BIGQ_H
#define BIGQ_H

#include <algorithm>    // std::sort
#include <iostream>
#include <pthread.h>
#include <queue>
#include <stdexcept>
#include <string>
#include <vector>
#include <unistd.h>
#include "Comparison.h"
#include "ComparisonEngine.h"
//#include "DBFile.h"
#include "Defs.h"
#include "Pipe.h"
#include "Schema.h"
#include "Util.h"

using namespace std;

// OrderMaker cannot specify sorting order (ascending or descending)
// So we wrap the Compare function and apply self-defined symbol SortOrder for specifying the sorting order.
enum SortOrder {Ascending, Descending};

//typedef void * (*THREADFUNCPTR)(void *);

/*
 * Class for blocks in the second phase of TPMMS   
 */
class Block {
    private:
        long long m_blockSize;
        long long m_nextLoadPageIdx;
        long long m_runEndPageIdx;
        File m_inputFile;
        vector<Page*> m_pages; 
        
    public:
        Block();
        Block(long long size, pair<long long, long long> runStartEndPageIdx ,File &inputFile);
        
        // returns false if there are pages left in current run, 
        //         true if the run has exhausted
        bool noMorePages(); 

        // check if this block is full of pages
        bool isFull();

        // check if this block is empty
        bool isEmpty();

        // Load the next page for current run
        // Returns 1 -> success; 0 -> failure 
        int loadPage();

        // Get the front record without popping it
        // Returns 1 -> success; 0 -> failure
        int getFrontRecord(Record& front);
        
        // Pop the front record
        // Returns 1 -> success; 0 -> failure
        int popFrontRecord();

};

class BigQ {

private:
    
    Pipe *m_inputPipe; 
    Pipe *m_outputPipe;
    static SortOrder m_sortMode;
    static OrderMaker m_attOrder;
    long long m_runLength; // The maximum number of pages in one run
    long long m_numRuns; // The number of runs after the first phase. This cannot be pre-determined.
    string m_sortTmpFilePath;
    
    //Schema* m_myS;

    // Record page indices where each run starts (inclusive) and ends at (exclusive) in file 
    // [(startPageIdx, endPageIdx), (startPageIdx, endPageIdx), ...]
    vector< pair<long long, long long> > m_runStartEndLoc; 
    
    // Compare functions used in std::sort() and std::priority_queue

    /*
     * Compare function for two Records, this is used in std::sort()
     */ 
    static bool compare4Sort(Record *left, Record *right);
    
    /*
     * Compare function for Records in two pairs, this is used in std::priority_queue
     * The first element in the pair is index of block the record belongs to
     * The second element is the record itself
     */
    //bool compare4PQ(pair<int, Record>& left, pair<int, Record>& right);
    struct compare4PQ {
        bool operator() (pair<long long, Record*>& left, pair<long long, Record*>& right) {
            return compare4Sort(left.second, right.second);
        }
    };
    
    //void printVec(vector<Record*> &recs); 
    void sortRecords(vector<Record*> &recs, const OrderMaker &order, SortOrder mode);
    void TPMMS_Phase1(File &outputFile);

public:  
    /* 
     * Min or Max heap used in TPMMS phase 2 for merging all runs 
     */
    priority_queue< pair<long long, Record*>, vector< pair<long long, Record*> >, compare4PQ > m_heap;
    
    /*
     * Safely pushes a record into the heap / prioroty_queue
     */
    void safeHeapPush(long long idx, Record* pushMe);

    // Returns index of the next block which will pop its front record to output pipe
    // In another word, finds which block contains the min or max front record currently
    // and returns its index in vector 'blocks' 
    // NOTE: This function can be improved to speed up
    int nextPopBlock(vector<Block>& blocks);

    // Merge all blocks and write them to output pipe
    void mergeBlocks(vector<Block>& blocks);

    void TPMMS_Phase2(File &inputFile);
    
    void TPMMS();

    BigQ() {}
  
    BigQ (Pipe &inputPipe, Pipe &outputPipe, OrderMaker &order, int runLength);//, Schema* myS); // HERE
    
    ~BigQ();

};


#endif