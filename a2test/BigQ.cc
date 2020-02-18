#include "BigQ.h"

using namespace std;

OrderMaker BigQ::m_attOrder;
SortOrder BigQ::m_sortMode = Ascending; 

BigQ::BigQ(Pipe &inPipe, Pipe &outPipe, OrderMaker &order, int runLen) {
    if (runLen <= 0) {
        throw invalid_argument("In BigQ::BigQ, 'runLen' cannot be zero or negative");
    }
    m_inputPipe = &inPipe;
    m_outputPipe = &outPipe;
    m_attOrder = order;
    m_runLength = runLen; // the maximum number of pages in one run
    m_numRuns = 0;
    //m_sortMode = Ascending; // sort in ascending order by default
    m_sortTmpFilePath = "intermediate.tmp";
    //m_myS = myS;

    typedef void * (*THREADFUNCPTR)(void *);
    pthread_t worker;
	pthread_create (&worker, NULL, (THREADFUNCPTR) &BigQ::TPMMS, this);
    pthread_join (worker, NULL);
}

BigQ::~BigQ() {}

bool BigQ::compare4Sort(Record *left, Record *right) {

    ComparisonEngine compEngine; 
    int res = compEngine.Compare(left, right, &m_attOrder);
    if (m_sortMode == Ascending){ // ascending order
        // (res > 0) will make it descending actually
        // Because we will write the records in reversed order later (by using vector.pop_back()),
        // so we need make it descending if we want ascending finally.
        return (res > 0);  
    }
    else if (m_sortMode == Descending){ // descending order
        // Ascending actually, same reason as above
        return (res < 0); 
    }
    
}

void BigQ::sortRecords(vector<Record*> &recs, const OrderMaker &order, SortOrder mode){
    sort(recs.begin(), recs.end(), compare4Sort);
}

/*
void BigQ :: printVec(vector<Record*> &recs) {
    for (int i=0; i<recs.size(); i++) {
        recs[i]->Print(m_myS);
    }
}
*/

void BigQ::TPMMS_Phase1(File &outputFile){
    
    vector<Record*> runBuffer;
    Record curRecord;
    Page tmpPage;
    int curPageIdxInFile = 0;

    // If we find out that after we reading the last record, total bytes exceed run size,
    // we cannot re-insert it into input pipe
    // Suppose the input pipe is full so producer is sleeping,
    // in such case, if we insert it into input pipe, the BigQ thread will also sleep as the pipe is full.
    // Then producer and BigQ threads are both sleeping, which makes the program freeze.
    // So we use a variable to store the last record and put it into the next run later. 
    Record* remainRecord = NULL; 

    while (true){

        long long totalReadBytes = 0;
        
        if (remainRecord != NULL) {
            runBuffer.push_back(remainRecord);
            totalReadBytes += remainRecord->length();
            remainRecord = NULL;
        }

        while (totalReadBytes < PAGE_SIZE * m_runLength && m_inputPipe->Remove(&curRecord) != 0){
            Record* tmp = new Record();
            tmp->Copy(&curRecord);
            runBuffer.push_back(tmp);
            totalReadBytes += curRecord.length();
        }
        
        if (totalReadBytes > PAGE_SIZE * m_runLength) {
            // Total bytes will exceed capacity, so do NOT load it 
            // And as we have removed curRecord from inputPipe but won't load it in run, 
            // we need to store it in remainRecord temporarily.
            remainRecord = runBuffer.back();
            runBuffer.pop_back();
            totalReadBytes -= curRecord.length();
            //m_inputPipe->Insert(&curRecord); No re-inserting here!
        }

        // sort in ascending order by default
        sortRecords(runBuffer, m_attOrder, m_sortMode); 
        
        if (!runBuffer.empty()) {
            // A new run is waiting to be written to file
            // Record its starting page index first
            // As we cannot know how long this run would be, we initialize the length as zero here.
            m_runStartEndLoc.push_back(pair<int, int>(curPageIdxInFile, curPageIdxInFile)); 
            m_numRuns++;
        }
        // write run buffer into file
        // size of File can be infinite (no limit in our program), so we only need one File instance here,
        // no matter how large the runs are.
        while (!runBuffer.empty()) {   
            tmpPage.EmptyItOut();
            while (!runBuffer.empty() && tmpPage.Append(runBuffer.back())){
                delete runBuffer.back();
                runBuffer.pop_back();
            }
            outputFile.AddPage(&tmpPage, curPageIdxInFile);
            curPageIdxInFile++;
        }

        if (!m_runStartEndLoc.empty() && curPageIdxInFile > m_runStartEndLoc.back().second) {
            // curPageIdxInFile has moved, 
            // which means there is a new run and the 'if' and 'while' above are both executed.
            // So we assign it to the second number (ending page index) of the last pair in m_runStartEndLoc             
            m_runStartEndLoc[m_numRuns - 1].second = curPageIdxInFile;
        }      

        if (m_inputPipe->isClosed() && m_inputPipe->isEmpty() && remainRecord == NULL) {
            return;
        }

    } 

}

void BigQ::safeHeapPush(int idx, Record* pushMe) {
    m_heap.push(pair<int, Record*>(idx, pushMe));
    pushMe = NULL;
}

int BigQ::nextPopBlock(vector<Block>& blocks) {
    if (blocks.empty() || m_heap.empty()) {
        return -1;
    }
    
    return m_heap.top().first;
}

void BigQ::mergeBlocks(vector<Block>& blocks) {
    int nextPop = nextPopBlock(blocks);
    if (nextPop == -1) {
        throw runtime_error("In BigQ::mergeBlocks, 'blocks' and 'm_heap' must be both non-empty initially.");
    }
    
    while (nextPop >= 0) {

        Record* front = new Record();
        
        // Pop front record in that block to output pipe
        blocks[nextPop].getFrontRecord(*front);
        blocks[nextPop].popFrontRecord();
        // Insert will consume front (delete its bits pointer)
        m_outputPipe->Insert(front);

        // Pop the corresponding record from heap
        delete m_heap.top().second;
        m_heap.pop();

        // Insert the next record (the new front record) from that block to heap
        // Note that after the popping above, blocks[nextPop] may has been empty, so we need to check it first.
        if (blocks[nextPop].getFrontRecord(*front)) {
            // if get new front record successfully, insert it into heap
            safeHeapPush(nextPop, front);
        }
        else {
            delete front; 
            front = NULL;
        }
        // if cannot get new front record, which means No.nextPop run has exhausted, then just do nothing

        nextPop = nextPopBlock(blocks);
    }

}

void BigQ::TPMMS_Phase2(File &inputFile){
    // blockSize is the maximum number of pages that one block can contain 
    /*
    int blockSize = m_runLength / m_numRuns;
    if (blockSize < 1) {
        throw runtime_error("In BigQ::TPMMS_Phase2, block size cannot be less than one page.");
    }
    */
    int blockSize = (MaxMemorySize / m_numRuns) / PAGE_SIZE;
    if (blockSize < 1) {
        throw runtime_error("In BigQ::TPMMS_Phase2, no enough memory!");
    }

    // Maintain a block for each run 
    vector<Block> blocks;
    for (int i = 0; i < m_numRuns; i++) {
        Block newBlock(blockSize, m_runStartEndLoc[i], inputFile);
        //if (newBlock.isEmpty()) {
            // No more pages left in this run
        //}
        blocks.push_back(newBlock);

        Record* tmp = new Record();
        newBlock.getFrontRecord(*tmp);
        safeHeapPush(i, tmp);
    }
    
    // Merge all blocks and write them to output pipe
    mergeBlocks(blocks);

    m_outputPipe->ShutDown();

}

void BigQ::TPMMS(){
    File tmpFile;
    // Phase 1
    tmpFile.Open(0, (char*)(m_sortTmpFilePath.c_str()));
    TPMMS_Phase1(tmpFile);
    tmpFile.Close();
    // Phase 2
    tmpFile.Open(1, (char*)(m_sortTmpFilePath.c_str()));
    TPMMS_Phase2(tmpFile);
    tmpFile.Close();
    // Empty the tmp file
    tmpFile.Open(0, (char*)(m_sortTmpFilePath.c_str()));
    tmpFile.Close();
}


Block::Block() {}

Block::Block(int size, pair<int, int> runStartEndPageIdx, File &inputFile) {
    m_blockSize = size;
    m_nextLoadPageIdx = runStartEndPageIdx.first;
    m_runEndPageIdx = runStartEndPageIdx.second;
    m_inputFile = inputFile;
    while (loadPage());
}

bool Block::noMorePages() {
    return (m_nextLoadPageIdx >= m_runEndPageIdx);
}

bool Block::isFull() {
    return (m_pages.size() >= m_blockSize);
}

bool Block::isEmpty() {
    return m_pages.empty();
}

int Block::loadPage() {
    if (noMorePages() || isFull()) {
        return 0;
    }
    m_pages.push_back(new Page());
    m_inputFile.GetPage(m_pages.back(), m_nextLoadPageIdx);
    m_nextLoadPageIdx++;
    return 1;
}

int Block::getFrontRecord(Record& front) {
    if (isEmpty()) {
        return 0;
    }
    return m_pages[0]->GetFirstNoConsume(front);
}

int Block::popFrontRecord() {
    if (isEmpty()) {
        return 0;
    }
    
    Record tmp;
    m_pages[0]->GetFirst(&tmp);
    
    if (m_pages[0]->IsEmpty()) {
        // m_pages[0] has exhausted, pop it and load a new page
        
        // Remember to delete page pointers as we create them using 'new' 
        delete m_pages[0];
        m_pages[0] = NULL;
        m_pages.erase(m_pages.begin());
        loadPage();
    }

    return 1;
}