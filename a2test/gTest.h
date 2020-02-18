//
// Created by Tony's Macbook pro on 2020-02-03.
//

#ifndef DSI_GTEST_H
#define DSI_GTEST_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "DBFile.h"
#include "Record.h"
using namespace std;

extern "C" {
int yyparse(void);   // defined in y.tab.c
}

extern struct AndList *final;

class relation {

private:
    const char *rname;
    const char *prefix;
    char rpath[100];
    Schema *rschema;
public:
    relation (const char *_name, Schema *_schema, const char *_prefix) :
            rname (_name), rschema (_schema), prefix (_prefix) {
        sprintf (rpath, "%s%s.bin", prefix, rname);
    }
    const char* name () { return rname; }
    const char* path () { return rpath; }
    Schema* schema () { return rschema;}
    void info () {
    }

};

const char *supplier = "supplier";
const char *partsupp = "partsupp";
const char *part = "part";
const char *nation = "nation";
const char *customer = "customer";
const char *orders = "orders";
const char *region = "region";
const char *lineitem = "lineitem";

relation *s, *p, *ps, *n, *li, *r, *o, *c;

void setup (const char *catalog_path, const char *dbfile_dir, const char *tpch_dir) {

    s = new relation (supplier, new Schema (catalog_path, supplier), dbfile_dir);
    ps = new relation (partsupp, new Schema (catalog_path, partsupp), dbfile_dir);
    p = new relation (part, new Schema (catalog_path, part), dbfile_dir);
    n = new relation (nation, new Schema (catalog_path, nation), dbfile_dir);
    li = new relation (lineitem, new Schema (catalog_path, lineitem), dbfile_dir);
    r = new relation (region, new Schema (catalog_path, region), dbfile_dir);
    o = new relation (orders, new Schema (catalog_path, orders), dbfile_dir);
    c = new relation (customer, new Schema (catalog_path, customer), dbfile_dir);
}

void cleanup () {
    delete s, p, ps, n, li, r, o, c;
}

#endif //DSI_GTEST_H
