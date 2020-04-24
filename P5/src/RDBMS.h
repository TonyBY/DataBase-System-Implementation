#ifndef RDBMS_H
#define RDBMS_H

#include <algorithm>
#include <stdint.h>
#include <iostream>
#include <map>
//#include <set>
#include <string>
#include <vector>
#include <list>
#include <utility>
#include <string.h>
#include "Comparison.h"
#include "DBFile.h"
#include "Defs.h"
#include "Function.h"
#include "Pipe.h"
#include "Record.h"
#include "RelOp.h"
#include "Schema.h"
#include "Statistics.h"
#include "Util.h"
#include "QueryOptimizer.h"

#define Undefined -1
#define NOT_FOUND -1

#define SELECT_FILE 0
#define SELECT_PIPE 1
#define PROJECT 2
#define JOIN 3
#define DUP_REMOVAL 4
#define SUM 5
#define GROUP_BY 6
#define WRITE_OUT 7

#define NONE 0
#define SINGLE 1
#define DOUBLE 2

extern FuncOperator* finalFunction;
extern TableList* tables;
extern AndList* boolean;
extern NameList* groupingAtts;
extern NameList* attsToSelect;
extern int distinctAtts;
extern int distinctFunc;
extern struct AttrList *newattrs;
extern char *newtable;
extern char *newfile;
extern char *oldtable;
extern char *deoutput;
extern int exitDB;

class RDBMS {
    public:
        QueryPlan *plan;
        Statistics *stat;
        std::vector<std::string> relations;

        RDBMS();
        ~RDBMS();
        void start();
        void create();
        void insert();
        void drop();
        void setOutput();
        void exitMe();
};

#endif