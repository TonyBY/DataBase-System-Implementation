#include "RDBMS.h"
extern "C" struct YY_BUFFER_STATE *yy_scan_string(const char*);
extern "C" int yyparse (void);

const int num_dbfile_related_suffix = 2;
const std::string dbfile_dir = "../data/";
const std::string temp_ans_dir ="../tpch-dbgen/answers.tbl";
const std::string dbfile_suffix[num_dbfile_related_suffix] = {".cache", ".meta"};


RDBMS::RDBMS() {}

RDBMS::~RDBMS() {}

void RDBMS::start() {
    while (true){
        yyparse();
        if (exitDB) {
            exitMe();
        }
        else if (newtable) {
            create();
        }
        else if (oldtable) {
            if (newfile){
                insert();
            }
            else {
                drop();
            }
        }
        else if (deoutput) {
            setOutput();
        }
        else {
            // if (relations.empty()){
            //     std::cout << "relation list is empty!" << std::endl;
            // }
            
            // if (find(relations.begin(), relations.end(), std::string(tables->tableName)) == relations.end()) {
            //     std::cerr << "Table " << tables->tableName << " not exists" << std::endl;
            //     return;
            // }
            stat = new Statistics();
            stat->Read(Util::StatisticsFile);
	        plan = new QueryPlan(stat);
	        
            // std::cout << std::endl;
            // plan->createPlanTree();
	        // std::cout << "PRINTING TREE IN ORDER:" << std::endl;
	        // std::cout << std::endl;
            // printLevelOrder((QueryPlanNode*)plan->getRoot());

            // std::cout << "Created Plan Tree" << std::endl;
            // std::cout << std::endl;
            // std::cout << std::endl;
            // std::cout << "Start execute the Plan Tree:" << std::endl;
            std::cout << std::endl;
            std::cout << "Please set output option: " << std::endl;
            std::cout << std::endl;
            yyparse();
            if (strcmp(deoutput, "NONE") == 0){
                std::cout << std::endl;
                std::cout << "***************************************************************************************************************************" << std::endl;
                std::cout << "enter:" << std::endl;
                plan->createPlanTree();
                std::cout << "PRINTING TREE IN ORDER:" << std::endl;
                std::cout << std::endl;
                printLevelOrder((QueryPlanNode*)plan->getRoot());
                std::cout << "***************************************************************************************************************************" << std::endl;
            }
            else{
                plan->createPlanTree();
                plan->execute();
                if (strcmp(deoutput, "STDOUT") == 0) {
                plan -> printAnswers();
                }
                else{
                    system(("cp " + temp_ans_dir + " "+ dbfile_dir + "results/" + deoutput + ".tbl").c_str());
                }
            }
            delete stat;
            delete plan;
            std::cout << "Query done" << std::endl;
        }
    }
    
}

void RDBMS::create() {
    std::string relname = std::string(newtable);
    relations.push_back(relname);
    std::ofstream out;
    out.open((dbfile_dir + relname + "_catalog").c_str(), std::ofstream::trunc | std::ofstream::out);
    //std::cout << "catalog create: " << out.is_open() << std::endl;
    std::string out_str = "BEGIN\n" + relname + "\n" + relname + std::string(".tbl") + "\n";
    while (newattrs) {
        out_str += std::string(newattrs->name) + " " + TypeStr[newattrs->type] + "\n";
        // std::cout << newattrs->type << std::endl;
        newattrs = newattrs->next;
    }
    out_str += "END\n";
    out << out_str;
    out.close();
    std::cout << "Create done" << std::endl;
}

void RDBMS::insert() {
    std::string relname(oldtable);
    if (find(relations.begin(), relations.end(), relname) == relations.end()) {
        std::cerr << "Table " << relname << " not exists" << std::endl;
        return;
    }
    DBFile db;
    std::string dbfile = dbfile_dir + relname + ".bin";
    std::string catalog = dbfile_dir + relname + "_catalog";
        
    db.Create(dbfile.c_str(), heap, NULL);
    Schema sch((char*)catalog.c_str(), (char*)relname.c_str());
    db.Load(sch, newfile);
    db.Close();
    //db.Open(dbfile.c_str());
    std::cout << "Insert done" << std::endl;
}

void RDBMS::drop() {
    std::string relname(oldtable);
    std::vector<std::string>::iterator it;
    if ( (it = find(relations.begin(), relations.end(), relname)) == relations.end()) {
        std::cerr << "Table " << relname << " not exists" << std::endl;
        return;
    }
    std::string catalog = dbfile_dir + relname + "_catalog";
    std::string binary = dbfile_dir + relname + ".bin";
    system(("exec rm " + catalog + " " + binary).c_str());
    for (int i = 0; i < num_dbfile_related_suffix; i++) {
        std::string dbfile_related = dbfile_dir + relname + ".bin" + dbfile_suffix[i];
        system(("exec rm " + dbfile_related).c_str());
    }
    relations.erase(it);
    std::cout << "Drop done" << std::endl;
}

void RDBMS::setOutput() {
    if (strcmp(deoutput, "STDOUT") == 0) {
        std::cout << "Printing the latest result table in the cache:" << std::endl;
        plan -> printAnswers();
    }
    else if (strcmp(deoutput, "NONE") == 0) {
        std::cout << "Please into your select query first: " <<std::endl;
    }
    else{
        std::cout << "Outputing the latest result table in the cache to: " + dbfile_dir + "results/" + deoutput + ".tbl"<< std::endl;
        system(("cp " + temp_ans_dir + " "+ dbfile_dir + "results/" + deoutput + ".tbl").c_str());
    }
}

void RDBMS::exitMe() {
    exit(0);
}