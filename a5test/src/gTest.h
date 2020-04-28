#ifndef MY_GTEST_H
#define MY_GTEST_H

#include <gtest/gtest.h>
#include <algorithm>    // std::sort
#include <iostream>
#include <pthread.h>
#include <queue>
#include <stdexcept>
#include <string>
#include <vector>
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "DBFile.h"
#include "Defs.h"
#include "Pipe.h"
#include "Schema.h"
#include "Statistics.h"
#include "BigQ.h"
#include "Util.h"
#include "QueryOptimizer.h"

extern "C" struct YY_BUFFER_STATE *yy_scan_string(const char*);
extern "C" int yyparse (void);

extern struct AndList *boolean;
//struct AndList *final;


#endif