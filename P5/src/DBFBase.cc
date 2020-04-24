#include <iostream>
#include <fstream>
#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "DBFBase.h"
#include "Defs.h"

using namespace std;

DBFBase::DBFBase() {
    file = new File();
    currentDataPageIdx = -1;
    //continuousGetNext = false;
    //cache = new File();
}

//DBFBase::DBFBase (File* file) {
 //   this->file = file;
//}

DBFBase::~DBFBase() {
    if (file != NULL) {
        delete file;
        file = NULL;
    }
    //if (cache != NULL) {
     //   delete cache;
      //  cache = NULL;
    //}
}

int DBFBase::Create (const char *f_path, fType f_type, void *startup) {
    file->Open(0, (char*)f_path);
    myType = f_type;
    currentDataPageIdx = -1; // No data, so it points to the first page which has no data.
    currentPage.EmptyItOut();
    return 1;
}

void DBFBase::Load (Schema &f_schema, const char *loadpath) {

    // Load data file
    FILE* data_file = fopen(loadpath, "r");

    // Create new page
    Page new_page;

    // Create new Record object
    Record* new_record = new Record();    
    
    // Write data into new page
    int cnt = 0;
    while (new_record->SuckNextRecord(&f_schema, data_file)){ // suck next record into new_record successfully
        
        //new_record->Print(&f_schema);
        
        if (!new_page.Append(new_record)) // cannot append new_record since new_page is full
        {
            file->AddPage(&new_page, currentDataPageIdx + 1); 
            // currentDataPageIdx is the index of DATA page where currentRecord points to. 
            // So it means that No.currentDataPageIdx has existed
            // So when we add new page, we need to add it to the next page index, that is, currentDataPageIdx + 1.
            currentDataPageIdx++;
            new_page.EmptyItOut();
            new_page.Append(new_record);
        }
        cnt++;
    }
    
    std::cout << "The number of loaded records: " << cnt << std::endl;

    if (!new_page.IsEmpty()) {
        file->AddPage(&new_page, currentDataPageIdx + 1); 
        currentDataPageIdx++;
        new_page.EmptyItOut();
    }

    // Move the pointer 'currentRecord' to the first record in file
    file->GetPage(&currentPage, currentDataPageIdx);
    this->MoveFirst();
    delete new_record;
    new_record = NULL;

    // No need to write meta-data into the first page here.
    // The meta-data will be written when calling Close() to close this file.
}

int DBFBase::Open (const char *f_path) {
    file->Open(1, (char*)f_path);
#ifdef verbose
    cout << "[Info] In DBFile::Open (const char *f_path): Length of file " << f_path << " : " << file->GetLength() << endl;
#endif

    MoveFirst();

    return 1;
}

void DBFBase::MoveFirst () {// Notice: file may be empty.
    //continuousGetNext = false;
    if (file->GetLength() > 1) {
        file->GetPage(&currentPage, 0); // get the first page in this file
        currentPage.MoveToFirst(); // get the first record in this page
        currentDataPageIdx = 0;
    }
    else{
#ifdef verbose
        cerr << "[Error] In funciton DBFile::MoveFirst (): No data in file" << endl;
#endif
    }
}

int DBFBase::Close () {
    try{
        //continuousGetNext = false;  
        int state = file->Close();
        currentDataPageIdx = -1;
        currentPage.EmptyItOut();
        return state;
    }
    catch (exception e){
        cerr << "[Error] In function DBFile::Close (): " << e.what() << endl;
        return 0;
    }
}

void DBFBase::Add (Record &rec) { // Notice: file may be empty.

    //continuousGetNext = false;

    off_t cur_len = file->GetLength();
    off_t last_data_page_idx = cur_len - 2;
    
    Page last_data_page;
    
    if (last_data_page_idx >= 0){ // File is non-empty
        file->GetPage(&last_data_page, last_data_page_idx);
    }

    if (!last_data_page.Append(&rec)) { // The last data page is full.
        Page new_page;
        new_page.Append(&rec);
        file->AddPage(&new_page, last_data_page_idx + 1);
    }
    else { // The last data page is not full and add record successfully, then rewrite that page in file
        if (last_data_page_idx >= 0) { // last page is from the file
            file->AddPage(&last_data_page, last_data_page_idx);
        }
        else { // last page is newly created
            file->AddPage(&last_data_page, 0);
        }
    }

    if (currentDataPageIdx < 0) {
        MoveFirst();
    }
}

int DBFBase::GetNext (Record &fetchme) { // Notice: file may be empty.
    off_t cur_len = file->GetLength();
    off_t last_data_page_idx = cur_len - 2;
    if (last_data_page_idx < 0){ // File is empty
#ifdef verbose
        cerr << "[Error] In funciton DBFile::GetNext (Record &fetchme): No data in file" << endl;
#endif
        return 0;
    }
    int status = currentPage.GetNextRecord(fetchme);
    if (status == 2){ // Has arrived at the last record in current page.
        if (currentDataPageIdx + 1 > last_data_page_idx){ // Current page is the last page in current file.
#ifdef verbose
            cout << "[Info] In function DBFile::GetNext (Record &fetchme): Has arrived at the end of current file." << endl;
#endif
            return 0;
        }
        // Get the next page in this file.
        currentDataPageIdx++;
        file->GetPage(&currentPage, currentDataPageIdx);
        currentPage.GetFirstNoConsume(fetchme);
        //currentPage.MoveToFirst();
        return 1;
    }
    return status;
}

int DBFBase::GetNext (Record &fetchme, CNF &cnf, Record &literal) { // Notice: file may be empty.
    ComparisonEngine comp_engin; 
    while (GetNext(fetchme)){ // Get next record successfully
        if (comp_engin.Compare(&fetchme, &literal, &cnf) == 1){ // Record fetchme satisfies the CNF rules
            return 1; // Matched
        }
    }
    return 0; // No match
}
