#include "QueryOptimizer.h"

const char *dbfile_dir = "../data/bin/"; // dir where binary heap files should be stored
const char *tpch_dir ="../data/tpch-dbgen/"; // dir where dbgen tpch files (extension *.tbl) can be found
const char *catalog_path = "catalog"; // full path of the catalog file
const char *dbfile_ext_name = ".bin"; // extension name of binary db data file
const char *tpch_ext_name = ".tbl"; 

const string answers_catalog = "answers_catalog";
const string answers_relname = "answers";

const string intermediate_file_prefix = "intermediate";
const string intermediate_catalog_prefix = "intermediate_catalog";

const bool reset = false;

int QueryPlanNode::next_assign_pipeID = 1;
int QueryPlanNode::next_assign_fileID = 0;


void printQueryPlanTree(QueryPlanNode *root) {
    std::pair<QueryPlanNode*, int> node_level;
    std::list<std::pair<QueryPlanNode*, int> > q;
    q.push_back({root, 0});
    int cur_level = 0;
    while (!q.empty()) {
        node_level = q.front();
        q.pop_front();
        QueryPlanNode *node = node_level.first;
        if (node_level.second > cur_level) {
            std::cout << std::endl;
            cur_level = node_level.second;
        }

        std::cout << node->name << "(" << node->file_id << ", " << node->left_pipe_id << ", " << node->right_pipe_id << ", " << node->out_pipe_id << ")  ";
        
        if (node->left != NULL) {
            q.push_back({node->left, node_level.second+1});
        }
        if (node->right != NULL) {
            q.push_back({node->right, node_level.second+1});
        }
    }
    std::cout << std::endl;
}

// Print the QueryPlan in a deep first search manner.
void printInOrderQueryPlan(QueryPlanNode *root) {
    if (root == NULL) {
        return;
    }
    printInOrderQueryPlan(root->left);
    root->print();
    printInOrderQueryPlan(root->right);
}

// Print the QueryPlan in a breath first search manner.
/* Function to print level  
order traversal a tree*/
/* Print nodes at a given level */
void printGivenLevel(QueryPlanNode* root, int level)  
{  
    if (root == NULL)  
        return;  
    if (level == 1)  
        root->print();  
    else if (level > 1)  
    {  
        printGivenLevel(root->left, level-1);  
        printGivenLevel(root->right, level-1);  
    }  
}

void printLevelOrder(QueryPlanNode* root)  
{  
    int h = height(root);  
    int i;  
    for (i = h; i >= 0; i--)  
        printGivenLevel(root, i);  
}  

/* Compute the "height" of a tree -- the number of  
    nodes along the longest path from the root node  
    down to the farthest leaf node.*/
int height(QueryPlanNode* node)  
{  
    if (node == NULL)  
        return 0;  
    else
    {  
        /* compute the height of each subtree */
        int lheight = height(node->left);  
        int rheight = height(node->right);  
  
        /* use the larger one */
        if (lheight > rheight)  
            return(lheight + 1);  
        else return(rheight + 1);  
    }  
}  


void printInOrderQueryPlan_BFS(QueryPlanNode *root) 
{ 
    // Base Case 
    if (root == NULL)  return; 
  
    // Create an empty queue for level order tarversal 
    queue<QueryPlanNode *> q; 
  
    // Enqueue Root and initialize height 
    q.push(root); 
  
    while (q.empty() == false) 
    { 
        // Print front of queue and remove it from queue 
        QueryPlanNode *node = q.front(); 
        node->print();
        q.pop(); 
  
        /* Enqueue left child */
        if (node->left != NULL) 
            q.push(node->left); 
  
        /*Enqueue right child */
        if (node->right != NULL) 
            q.push(node->right); 
    } 
} 

void deleteQueryPlanTree(QueryPlanNode *root) {
    if (root == NULL) {
        return;
    }
    deleteQueryPlanTree(root->left);
    deleteQueryPlanTree(root->right);
    //std::cout << "Deleting node: " << root->name << "(" << root->file_id << ", " << root->left_pipe_id << ", " << root->right_pipe_id << ", " << root->out_pipe_id << ")\n";
    delete root;
}

void executeQueryPlan(QueryPlanNode *root, const std::map<int, Pipe*> &all_pipes) {
    //executeQueryPlanExceptJoin(root, all_pipes);
    //sleep(1);
    //executeQueryPlanOnlyJoin(root, all_pipes);
    if (root == NULL) {
        return;
    }
    executeQueryPlan(root->left, all_pipes);
    executeQueryPlan(root->right, all_pipes);
    if (root->left && root->left->type == JOIN) {
        //root->left->executor->WaitUntilDone();
        sleep(10);
        root->left_reader_from_join->execute(all_pipes);
    }
    if (root->right && root->right->type == JOIN) {
        //root->right->executor->WaitUntilDone();
        sleep(10);
        root->right_reader_from_join->execute(all_pipes);
    }
    root->execute(all_pipes);
}

void executeQueryPlanExceptJoin(QueryPlanNode *root, const std::map<int, Pipe*> &all_pipes) {
    if (root == NULL) {
        return;
    }
    executeQueryPlanExceptJoin(root->left, all_pipes);
    if (root->type != JOIN) {
        root->execute(all_pipes);
    }
    executeQueryPlanExceptJoin(root->right, all_pipes);
}

void executeQueryPlanOnlyJoin(QueryPlanNode *root, const std::map<int, Pipe*> &all_pipes) {
    if (root == NULL) {
        return;
    }
    executeQueryPlanOnlyJoin(root->left, all_pipes);
    if (root->type == JOIN) {
        root->execute(all_pipes);
        //sleep(2);
    }
    executeQueryPlanOnlyJoin(root->right, all_pipes);
}


void waitUntilExecuteDone(QueryPlanNode *root) {
    if (root == NULL) {
        return;
    }
    waitUntilExecuteDone(root->left);
    waitUntilExecuteDone(root->right);
    if (root->executor != NULL)
        root->executor->WaitUntilDone();
}

void printFile(std::string filepath) {
    std::ifstream in;
    in.open(filepath.c_str());
    std::string line;
    int cnt = 0;
    while (getline(in, line)) {
        std::cout << line << std::endl;
        cnt ++;
    }
    std::cout << "Print " << cnt << " rows from file '" << filepath << "' " << std::endl;
}

QueryPlan::QueryPlan(Statistics *st) {
    initPrefixMap();        
    stat = st;
    // initialize optimal predicate as the original input predicate
    optimal_predicate = boolean; //extern struct AndList *boolean; // the predicate in the WHERE clause //SQL: SELECT WhatIWant FROM Tables WHERE AndList
    // remove relation name from predicate, like "l.l_name" -> "l_name". 
    // It is because our Statistics does not support predicate with relation name prefix.
    Util::removeRelNameFromPred(optimal_predicate);
    Util::removeRelNameFromPred(attsToSelect);
    Util::removeRelNameFromPred(groupingAtts);
    Util::removeRelNameFromPred(finalFunction);
    // then optimize it
    optimizePredicate();

    min_order = Util::splitAndList(optimal_predicate);
    number_of_joins = 0;
    number_of_selects = 0;
}

QueryPlan::~QueryPlan() {
    deleteQueryPlanTree(this->root);
    QueryPlanNode::next_assign_fileID = 0;
    QueryPlanNode::next_assign_pipeID = 0;
    for (std::map<int, Pipe*>::iterator it = pipes.begin(); it != pipes.end(); it++) {
        delete it->second;
    }
}

void QueryPlan::execute() {
    int num_pipes = QueryPlanNode::next_assign_pipeID;
    for (int i = 0; i < num_pipes; i++) {
        pipes[i] = new Pipe(pipe_size);
        std::cout << "PipeID " << i << ", Addr: " << pipes[i] << std::endl;
    }
    if (reset) {
        std::cout << "================== [Attention!!!] Reseting all data... ==================" << std::endl;
        system(("exec rm " + std::string(dbfile_dir) + "*").c_str());
    }
    executeQueryPlan(this->root, pipes);
    waitUntilExecuteDone(this->root);
    if (this->root->output_file) {
        fclose(this->root->output_file);
        this->root->output_file = NULL;
    }
    printAnswers();
}

void QueryPlan::printAnswers() {
    DBFile db;
    std::string dbfile_path = std::string(dbfile_dir) + answers_relname + std::string(dbfile_ext_name);
    std::string tpch_file_path = std::string(tpch_dir) + answers_relname + std::string(tpch_ext_name);
    //std::cout << dbfile_path << " - " << tpch_file_path << std::endl;
    db.Create(dbfile_path.c_str(), heap, NULL);
    Schema sch((char*)(answers_catalog.c_str()), (char*)(answers_relname.c_str()));
    
    //printFile(tpch_file_path);
    
    db.Load(sch, (char*)(tpch_file_path.c_str()));
    db.Close();
    db.Open(dbfile_path.c_str());
    db.MoveFirst();
    Record ans;
    int cnt = 0;
    while (db.GetNext(ans)) {
        ans.Print(&sch);
        cnt ++;
    }
    std::cout << "Answers: " << cnt << " rows" << std::endl;
}

const QueryPlanNode* QueryPlan::getRoot() {
    return root;
}

const Pipe* QueryPlan::getOutPipe() {
    return pipes.at(QueryPlanNode::next_assign_pipeID - 1);
}

const Schema* QueryPlan::getOutSchema() {
    return root->output_schema;
}

void QueryPlan::createPlanTree() {
    //Count number of selects and joins.
    for (int i = 0;i < min_order.size(); i++) {
        if(isJoin(min_order[i])){
            number_of_joins++;
        }
        else{
            number_of_selects++;
        }
    }
    std::cout << "Number of selects: " << number_of_selects << std::endl;
    std::cout << "Number of joins: " << number_of_joins << std::endl;
    createLoadDataNodes();  
    // printf("DONE: createLoadDataNodes\n");
    //for (int i = 0 ; i < leaves.size(); i++) {
    //    leaves[i]->print();
    //}
    createPredicateNodes();
    // printf("DONE: createPredicateNodes\n");
    //printQueryPlanTree(root);
    createGroupByNodes();
    // printf("DONE: createGroupByNodes\n");
    createSumNodes();
    // printf("DONE: createSumNodes\n");
    createProjectNodes();
    // printf("DONE: createProjectNodes\n");
    createDupRemovalNodes();
    // printf("DONE: createDupRemovalNodes\n");
    
    // createWriteOutNodes(std::string(tpch_dir) + answers_relname + std::string(tpch_ext_name));
    // root->saveSchema(answers_catalog, answers_relname);
}

// create nodes for loading data 
void QueryPlan::createLoadDataNodes() {
    // create nodes for loading data from files
    // std::string output = "";
    // output = Util::ParseTreeToString(min_order);
    // std::cout << "$$$$$$$$$$$$$$$$$$$$$$$" << std::endl;
    // std::cout << output << std::endl;

    std::map<std::string, int> all_need_rels;
    for (int i = 0;i < min_order.size(); i++) {
        std::vector<std::string> relnames = getRelNames(min_order[i]);
        for (int j = 0; j < relnames.size(); j++) {
            all_need_rels[relnames[j]] = 1;
        }
    }
    for (std::map<std::string, int>::iterator it = all_need_rels.begin(); it != all_need_rels.end(); it++) {
        // std::cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << std::endl;
        // std::cout << it->first << std::endl;
        SelectNode *leaf = new SelectNode(SELECT_FILE, NULL, NULL, false, (char*)catalog_path, (char*)(it->first.c_str()));
        leaves.push_back(leaf);
        relname_node[it->first] = leaf; // it->first if the relation name string
        if (node_relnames.find(leaf) == node_relnames.end()) {// if this is a new node to be added into the map: node_relnames
            node_relnames[leaf] = std::vector<std::string>();    
        }
        node_relnames[leaf].push_back(it->first);
    }
}

void QueryPlan::updateRelnameNodeMap(QueryPlanNode *new_root, QueryPlanNode *left_child, QueryPlanNode *right_child) {
    if (left_child == NULL) {
        throw runtime_error("[Error] In function QueryPlan::updateRelnameNodeMap(QueryPlanNode *new_root, QueryPlanNode *left_child, QueryPlanNode *right_child): left child must be not NULL");
    }
    // update relname_node
    std::vector<std::string> left_relnames = node_relnames[left_child];
    for (int i = 0; i < left_relnames.size(); i++) {
        relname_node[left_relnames[i]] = new_root;
    }
    // update node_relnames
    node_relnames[new_root] = std::vector<std::string>(left_relnames);
    node_relnames.erase(left_child);

    if (right_child != NULL) { // join
        // update relname_node
        std::vector<std::string> right_relnames = node_relnames[right_child];
        for (int i = 0; i < right_relnames.size(); i++) {
            relname_node[right_relnames[i]] = new_root;
            node_relnames[new_root].push_back(right_relnames[i]);
        }
        // update node_relnames
        //node_relnames[new_root] = std::vector<std::string>(left_relnames);
        node_relnames.erase(right_child); 
    }

}

QueryPlanNode* QueryPlan::createPredicateNodes() {
    QueryPlanNode *new_root = NULL;
    for (int i = 0; i < min_order.size(); i++) {
        // std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << std::endl;
        // std::cout << Util::ParseTreeToString(min_order[i]) << std::endl;
        std::vector<std::string> relnames = getRelNames(min_order[i]);
        if (relnames.size() == 1) {
            // select pipe node
            std::string cur_rel = relnames[0];
            QueryPlanNode *child = relname_node[cur_rel];
            SelectNode *select_pipe_node = new SelectNode(SELECT_PIPE, min_order[i], child, false, (char*)catalog_path, (char*)(cur_rel.c_str()));
            //relname_node[cur_rel] = select_pipe_node;
            updateRelnameNodeMap(select_pipe_node, child, NULL);
            new_root = select_pipe_node;
        }
        else {
            if (relnames.size() == 0) {
                throw runtime_error("[Error] In function QueryPlan::createPredicateNodes(): No matched relation found");
            }
            std::string cur_rel_left = relnames[0];
            std::string cur_rel_right = relnames[1];
            QueryPlanNode *left = relname_node[cur_rel_left];
            QueryPlanNode *right = relname_node[cur_rel_right];
            // std::cout << "-----------------------------------------" << std::endl;
            // if (node_relnames[left].size() == 1) {
            //     std::cout << node_relnames[left][0] << std::endl;
            // }
            // else{
            //     std::cout << node_relnames[left][0] << ", " << node_relnames[left][1] << std::endl;
            // }
            // if (node_relnames[right].size() == 1) {
            //     std::cout << node_relnames[right][0] << std::endl;
            // }
            // else{
            //     std::cout << node_relnames[right][0] << ", " << node_relnames[right][1] << std::endl;
            // }
            JoinNode *join_node = new JoinNode(min_order[i], left, right);
 
            updateRelnameNodeMap(join_node, left, right);
            new_root = join_node;
        }
        
    }
    this->root = new_root;
    return new_root;
}

void QueryPlan::createProjectNodes() {
    // std::cout << attsToSelect << std::endl;
    // if (attsToSelect && !finalFunction && !groupingAtts) {
    //     QueryPlanNode *new_root = new ProjectNode(attsToSelect, this->root);
    //     this->root = new_root;
    // }
    QueryPlanNode *new_root = new ProjectNode(attsToSelect, this->root);
    this->root = new_root;
}

void QueryPlan::createDupRemovalNodes() {
    if (distinctAtts != NULL || distinctFunc != NULL) {
        this->root = new DupRemovalNode(this->root);
        //throw runtime_error("Sorry, we are not supporting DISTINCT now, it is under developing...\n");
    }
}

void QueryPlan::createSumNodes() {
    QueryPlanNode *new_root = NULL;

    // if (finalFunction && !groupingAtts) {
    //     new_root = new SumNode(finalFunction, this->root);
    //     this->root = new_root;
    // }

    if (finalFunction) {
        new_root = new SumNode(finalFunction, this->root);
        this->root = new_root;
    }
}

void QueryPlan::createGroupByNodes() {
    if (groupingAtts != NULL) {
        //throw runtime_error("Sorry, we are not supporting GROUP BY now, it is under developing...\n");
        this->root = new GroupByNode(finalFunction, groupingAtts, this->root);
    }
}

void QueryPlan::createWriteOutNodes(std::string out_filepath) {
    this->root = new WriteOutNode(out_filepath, this->root);
}   

void QueryPlan::initPrefixMap() {
    prefix_relation["r"] = "region";
    prefix_relation["n"] =  "nation";
    prefix_relation["p"] =  "part";
    prefix_relation["s"] =  "supplier";
    prefix_relation["ps"] =  "partsupp";
    prefix_relation["c"] =  "customer";
    prefix_relation["o"] =  "orders";
    prefix_relation["l"] =  "lineitem";
}

void QueryPlan::optimizePredicate() {
    std::vector<AndList*> clauses = Util::splitAndList(optimal_predicate);
    //std::cout << "good" << std::endl;
    //std::cout << Util::ParseTreeToString(optimal_predicate) << std::endl;
    //std::cout << Util::ParseTreeToString(clauses[1]) << std::endl;
    if (clauses.empty()) {
        throw runtime_error("[Error] In function QueryPlan::optimizePredicate(): No predicate to be optimized");
    }
    min_cost = INT64_MAX;
    //std::vector<AndList*> min_order;
    removeSelfJoinPred(clauses);
    std::sort(clauses.begin(), clauses.end());
    do {
        //if (!isValidPred(clauses)) {
        //    continue;
        //}
        //std::cout << Util::ParseTreeToString(clauses) << std::endl;
        try{
            double cur_cost = estimateTotalCost(clauses);
            //std::cout << "Cost: " << cur_cost << std::endl;
            if (cur_cost < min_cost) {
                min_cost = cur_cost;
                min_order = clauses;
            }
        }
        catch (const exception &e) {
            std::cerr << "[Warning] " << e.what() << std::endl;
        }
        
    } while (std::next_permutation(clauses.begin(), clauses.end()));

    optimal_predicate = Util::getParseTreeFromVector(min_order);
    // std::cout << "Optimal predicate (cost " << min_cost << "): " << Util::ParseTreeToString(optimal_predicate) << std::endl;
}

bool QueryPlan::isValidPred(std::vector<AndList*> &predicates) {
    std::map<std::string, int> which_rel;
    int next_id = 0;
    for (int i = 0; i < predicates.size(); i++) {
        if (isJoin(predicates[i])) {
            std::string names[2] = {std::string(predicates[i]->left->left->left->value), 
                                std::string(predicates[i]->left->left->right->value)};
            for (int k = 0; k < 2; k++) {
                if (which_rel.find(names[k]) != which_rel.end()) {
                    if (which_rel.find(names[1-k]) != which_rel.end()) {
                        int where_is_other = which_rel[names[1-k]];
                        for (std::map<std::string, int>::iterator it = which_rel.begin(); it != which_rel.end(); it++) {
                            if (it->second == where_is_other) {
                                it->second = which_rel[names[k]];
                            }
                        }
                    }
                    else {
                        which_rel[names[1-k]] = which_rel[names[k]];
                    }
                    break;
                }
            }
                          
        }
        else {
            OrList *ors = predicates[i]->left;
            ComparisonOp *cur = ors->left;
            int where_is_prev = -1;
            while (cur != NULL) {
                if (where_is_prev == -1) {
                    where_is_prev = which_rel[std::string(cur->left->value)];
                }
                if (where_is_prev != which_rel[std::string(cur->left->value)]) {
                    return false;
                }
                where_is_prev = which_rel[std::string(cur->left->value)];
                ors = ors->rightOr;
                if (ors) {
                    cur = ors->left;
                }
                else {
                    break;
                }
            }
        }
    }
    return true;

}

double QueryPlan::estimateTotalCost(std::vector<AndList*> &whole_pred) {
    Statistics tmp_stat;
    tmp_stat.Read(Util::StatisticsFile);
    char* tmpFile = "sjdjueu836dncydhn.txt";
    double cost = 0;
    for (int i = 0; i < whole_pred.size(); i++) {
        if (i < whole_pred.size() - 1){
            cost += estimateAndApplySinglePred(whole_pred[i], &tmp_stat, tmpFile);
        }
        else {
            //std::cout << "final output num of rows: " << estimateAndApplySinglePred(whole_pred[i], &tmp_stat, tmpFile) << std::endl;
            estimateAndApplySinglePred(whole_pred[i], &tmp_stat, tmpFile);
        }
        //tmp_stat.Read(tmpFile);
    }
    return cost;
}

void QueryPlan::getRelsForStat(char* relNames[], int &numToJoin, AndList *single_pred, Statistics *tmp_stat){
    std::vector<std::string> rels = getRelNames(single_pred);

    std::vector<std::string> src_joined_rel1 = Util::splitString(tmp_stat->joined_relations[rels[0]]->name, '&'); 
        //relNames = (char**) malloc (numToJoin * sizeof(char*)); 
    for (int i = 0; i < src_joined_rel1.size(); i++) {
        //char char_arr[src_joined_rel1[i].length()];
        strcpy(relNames[i], src_joined_rel1[i].c_str());
    }
    numToJoin = src_joined_rel1.size();

    if (rels.size() == 2) { // join
        std::vector<std::string> src_joined_rel2 = Util::splitString(tmp_stat->joined_relations[rels[1]]->name, '&'); 
        numToJoin += src_joined_rel2.size();
        for (int i = 0; i < src_joined_rel2.size(); i++) {
            //relNames[i+src_joined_rel1.size()] = (char*)src_joined_rel2[i].c_str();
            strcpy(relNames[i+src_joined_rel1.size()], src_joined_rel2[i].c_str());
        }
    }
          
//    }
}

double QueryPlan::estimateAndApplySinglePred(AndList* single_pred, Statistics *tmp_stat, char* tmpFile) {
    char* relNames[MAX_NUM_RELS];
    for (int i = 0; i < MAX_NUM_RELS; i++) {
        relNames[i] = (char*) malloc (MAX_LEN_RELNAME * sizeof(char));
    }
    int numToJoin = 0;
    getRelsForStat(relNames, numToJoin, single_pred, tmp_stat);
    //char *relNames[] = {"supplier","partsupp"};
    //int numToJoin = 2;
    //printf(relNames[0]);
    double cost = tmp_stat->Estimate(single_pred, relNames, numToJoin);
    //getRelsForStat(relNames, numToJoin, single_pred, tmp_stat);
    ///std::cout << "ME" << std::endl;
    //printf("\n");
    //printf(relNames[0]);
    tmp_stat->Apply(single_pred, relNames, numToJoin);
    tmp_stat->Write(tmpFile);

    for (int i = 0; i < MAX_NUM_RELS; i++) {
        free(relNames[i]);
    }

    return cost;
}

bool QueryPlan::isJoin(AndList *single_pred) {
    if (single_pred->rightAnd != NULL) {
        throw runtime_error("[Error] In function isJoin(AndList *predicate): Not a single AndList");
    }
    return (single_pred->left->left->left->code == NAME && single_pred->left->left->right->code == NAME);
}

bool QueryPlan::isSelfJoin(AndList *single_pred) {
    if (single_pred->rightAnd != NULL) {
        throw runtime_error("[Error] In function isJoin(AndList *predicate): Not a single AndList");
    }
    return isJoin(single_pred) && (strcmp(single_pred->left->left->left->value, single_pred->left->left->right->value) == 0);
}

void QueryPlan::removeSelfJoinPred(std::vector<AndList*> &predicates) {
    for (std::vector<AndList*>::iterator it = predicates.begin(); it != predicates.end(); it++) {
        if (isSelfJoin(*it)) {
            predicates.erase(it);
        }
    }
}

std::vector<std::string> QueryPlan::getRelNames(AndList *single_pred){
    if (single_pred->rightAnd != NULL) {
        throw runtime_error("[Error] In function QueryPlan::getRelNames(AndList *predicate): Not a single AndList");
    }
    Operand* left_operand = single_pred->left->left->left;
    Operand* right_operand = single_pred->left->left->right;
        
    std::vector<std::string> relations;
    //std::string left_att_name = Util::getSuffix(std::string(left_operand->value), '.');

    std::string left_prefix = Util::getSuffix(Util::getPrefix(std::string(left_operand->value), '_'), '.');
    // std::cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << std::endl;
    // std::cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << std::endl;
    // std::cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << std::endl;
    // std::cout << std::string(left_operand->value) << std::endl;
    // std::cout << left_prefix << std::endl;
    // std::cout << std::string(right_operand->value) << std::endl;
    // std::cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << std::endl;
    // std::cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << std::endl;
    // std::cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << std::endl;

    //std::string left_relname = prefix_relation[left_prefix];
    //Schema left_sch(catalog_path, (char*)(left_relname.c_str()));
    
    //if (left_sch.Find((char*)(left_att_name.c_str())) == -1) {
    //    throw std::runtime_error("[Error] In function QueryPlan::getRelNames(AndList *predicate): ")
    //}

    relations.push_back(prefix_relation[left_prefix]);
    
    if (isJoin(single_pred)) {
        std::string right_prefix = Util::getSuffix(Util::getPrefix(std::string(right_operand->value), '_'), '.');
        relations.push_back(prefix_relation[right_prefix]);
    }

    return relations;
}

void QueryPlan::printPredicate() {
    Util::ParseTreeToString(optimal_predicate);
}




QueryPlanNode::QueryPlanNode() {
    name = "";
    type = Undefined;
    num_children = -1;
    left = right = NULL;
    file_id = -1;
    left_pipe_id = right_pipe_id = -1; // IDs of left and right input pipes corresponding to left and right children
    out_pipe_id = -1; // ID of output pipe
    intermediate_out_pipe_id = -1;
    output_schema = NULL;
    executor = NULL;
    left_reader_from_join = NULL;
    right_reader_from_join = NULL;
}

int QueryPlanNode::getNewPipeID() {
    int new_pipe_id = next_assign_pipeID;
    next_assign_pipeID++;
    return new_pipe_id;
}

int QueryPlanNode::getType() {
    return type;
}

void QueryPlanNode::processJoinChild(QueryPlanNode *me) {
    if (me->left != NULL && me->left->type == JOIN) {
        me->left_reader_from_join = new SelectNode(SELECT_FILE, NULL, NULL, true, 
                                            me->left->intermediate_catalog.c_str(), me->left->intermediate_rel_name.c_str());
        me->left_pipe_id = me->left_reader_from_join->out_pipe_id;
    }
    if (me->right != NULL && me->right->type == JOIN) {
        me->right_reader_from_join = new SelectNode(SELECT_FILE, NULL, NULL, true, 
                                            me->right->intermediate_catalog.c_str(), me->right->intermediate_rel_name.c_str());
        me->right_pipe_id = me->right_reader_from_join->out_pipe_id;
    }
}

void QueryPlanNode::saveSchema(std::string save_to, std::string relname) {
    std::ofstream out;
    out.open(save_to.c_str(), std::ofstream::trunc | std::ofstream::out);
    //std::cout << "catalog create: " << out.is_open() << std::endl;
    std::string out_str = "BEGIN\n" + relname + "\n" + relname + std::string(tpch_ext_name) + "\n";
    for (int i = 0; i < output_schema->numAtts; i++) {
        out_str += std::string(output_schema->GetAtts()[i].name) + " " + TypeStr[output_schema->GetAtts()[i].myType] + "\n";
    }
    out_str += "END\n";
    out << out_str;
    out.close();
}

void QueryPlanNode::print() {
    std::cout << " ***********" << std::endl;
    std::cout << name << " operation" << std::endl;
    if (type == SELECT_FILE && file_id >= 0) {
        // std::cout << "Input file " << file_id << std::endl;
        std::cout << "Input pipe " << 0 << std::endl;
    }
    else {
        if (right_pipe_id == -1) {
            std::cout << "Input Pipe " << left_pipe_id << std::endl;
            }
        else{
            std::cout << "Left Input Pipe " << left_pipe_id << std::endl;
            std::cout << "Right Input Pipe " << right_pipe_id << std::endl;
        }
    }
    if (type == WRITE_OUT && file_id >= 0) {
        std::cout << "Output file ID " << file_id << std::endl;
    }
    else if (type == JOIN){
        std::cout << "Output pipe ID " << intermediate_out_pipe_id << std::endl;
    }
    else {
        std::cout << "Output pipe ID " << out_pipe_id << std::endl;
    }
    std::cout << "Output Schema:" << std::endl;
    std::cout << output_schema->toString() << std::endl;
    printSpecInfo();
    std::cout << std::endl;
}



SelectNode::SelectNode() {}

SelectNode::SelectNode(int select_where, AndList *select_conditions, QueryPlanNode *child, bool is_intermediate, 
                        const char *catalog_filepath, const char *rel_name, const char* alias, int specific_out_pipe_id) {
    //file_id = Undefined;
    load_all_data = false;
    db = NULL;
    type = select_where;
    by_catalog = std::string(catalog_filepath);
    relation_name = std::string(rel_name);
    this->is_intermediate = is_intermediate;

    if (select_where == SELECT_FILE) {
        name = "SelectFile";
        if (child != NULL) {
            throw runtime_error("[Error] In function SelectNode::SelectNode: Trying to initialize a SelectFile node with a child");
        }
        num_children = 0;
        // file_id = next_assign_fileID++;
        file_id = 0;
        //relation_name = std::string(rel_name);
        //next_assign_fileID++;
        output_schema = new Schema((char*)(by_catalog.c_str()), (char*)relation_name.c_str());
   
    }
    else if (select_where == SELECT_PIPE) {
        name = "SelectPipe";
        if (child == NULL) {
            throw runtime_error("[Error] In function SelectNode::SelectNode: Trying to initialize a SelectPipe node without a child");
        }
        num_children = 1;
        left = child;

        processJoinChild(this);
        if (left->type == JOIN){
            left_pipe_id = left->intermediate_out_pipe_id;
        }
        else{
            left_pipe_id = left->out_pipe_id;
        }

        if (left_reader_from_join) {
            output_schema = new Schema(*(left_reader_from_join->output_schema));
        }
        else {
            output_schema = new Schema(*(left->output_schema));
        }
    }
    else {
        throw runtime_error("[Error] In function SelectNode::SelectNode: Invalid data source type '" + Util::toString<int>(select_where) + "'");
    }
    
    //out_pipe_id = next_assign_pipeID++; 
    if (specific_out_pipe_id >= 0){
        out_pipe_id = specific_out_pipe_id;
    }
    else if (is_intermediate){
        out_pipe_id = -1;
    }
    else {
        out_pipe_id = getNewPipeID();
    }
    
    //output_schema = new Schema((char*)(by_catalog.c_str()), (char*)relation_name.c_str());
   
    if (select_conditions != NULL){
        //this->cnf = cnf;
        load_all_data = false;
        cnf = new CNF();
        literal = new Record();
        cnf->GrowFromParseTree(select_conditions, output_schema, *literal);
        Util::addRelNameBackToPred(select_conditions);
        CNF_string = Util::ParseTreeToString(select_conditions); 
        //Attribute lit_att = {"val", Int};
        //literal->Print(new Schema("", 1, &lit_att));
    }
    else {
        load_all_data = true;
        cnf = new CNF();
        cnf->numAnds = 0;
        literal = NULL;
    }
    
}

SelectNode::~SelectNode() {
    delete output_schema;
    delete cnf;
    delete literal;
    delete executor;
    if (type == SELECT_FILE) {
        db->Close();
        delete db;
    }
}

void SelectNode::printSpecInfo() {
    if (!load_all_data){
        std::cout << "CNF:" << std::endl;
        std::cout << CNF_string << std::endl;
        // cnf->Print();
    }
    else {
        // std::cout << "No CNF because this node is only for loading all data" << std::endl;
    }
}

void SelectNode::execute(const std::map<int, Pipe*> &pipes) {
    if (type == SELECT_FILE) {
        db = new DBFile();
        std::string dbfile_path = std::string(dbfile_dir) + relation_name + std::string(dbfile_ext_name);
        std::string tpch_file_path = std::string(tpch_dir) + relation_name + std::string(tpch_ext_name);
        if (Util::exists(dbfile_path) && !is_intermediate) {
            db->Open(dbfile_path.c_str());
        }    
        else {
            //throw std::runtime_error("[Error] In function SelectNode::execute: " + std::string(e.what()));
            // No such binary data file, we need to create it
            db->Create(dbfile_path.c_str(), heap, NULL);
            //std::cout << "DBF: " << dbfile_path << std::endl;
            Schema sch((char*)(by_catalog.c_str()), (char*)(relation_name.c_str()));
            //std::cout << sch.toString() << std::endl;
            db->Load(sch, (char*)(tpch_file_path.c_str()));
            //std::cout << "SCH: " << tpch_file_path << std::endl;
            
            db->Close();
            db->Open(dbfile_path.c_str());
        }
        db->MoveFirst();
        SelectFile *sf = new SelectFile();
        sf->Run(*db, *(pipes.at(out_pipe_id)), *cnf, *literal);
        executor = sf;
        //executor.WaitUntilDone();
    }
    else if (type == SELECT_PIPE) {
        SelectPipe *sp = new SelectPipe();
        sp->Run(*(pipes.at(left_pipe_id)), *(pipes.at(out_pipe_id)), *cnf, *literal);
        executor = sp;
        //executor.WaitUntilDone();
    }
}
/*
SelectFileNode::SelectFileNode() {}

SelectFileNode::SelectFileNode(QueryPlanNode *child, int in_pipe_id, int out_pipe_id, 
                                CNF *cnf, char *, char *rel_name, char* alias) {
    name = "SelectPipe";
    num_children = 1;
    left = child;
    //right = NULL;
    left_pipe_id = in_pipe_id;
    //right_pipe_id = -1; 
    this->out_pipe_id = out_pipe_id; 
    this->cnf = cnf;
    output_schema = new Schema(catalog_path, rel_name);
}

SelectFileNode::~SelectFileNode() {
    delete output_schema;
}

void SelectFileNode::printSpecInfo() {
    cnf->Print();
}
*/

ProjectNode::ProjectNode() {}

ProjectNode::ProjectNode(NameList *atts, QueryPlanNode *child) {
    // if (atts == NULL || child == NULL) {
    //     throw runtime_error("[Error] In function ProjectNode::ProjectNode(NameList *atts, QueryPlanNode *child): atts and child must be both not null");
    // }
     if (child == NULL) {
        throw runtime_error("[Error] In function ProjectNode::ProjectNode(NameList *atts, QueryPlanNode *child): atts and child must be both not null");
    }
    name = "Project";
    type = PROJECT;
    num_children = SINGLE;
    left = child;

    // std::cout << "Project Node out_pipe_id: " << out_pipe_id << std::endl;

    Schema *input_schema = NULL;
    processJoinChild(this);
    if (child->type == JOIN){
        left_pipe_id = child->intermediate_out_pipe_id;
    }
    else{
        left_pipe_id = child->out_pipe_id;
    }
    out_pipe_id = getNewPipeID();
    // std::cout << "Project child Node name: " << child->name << std::endl;
    // std::cout << "Project Node in_pipe_id: " << left_pipe_id << std::endl;
    if (left_reader_from_join) {
        input_schema = new Schema(*(left_reader_from_join->output_schema));
    }
    else {
        input_schema = new Schema(*(left->output_schema));
    } 

    // std::cout << "Project input sch: " << input_schema->toString() << std::endl;

    if (child->type == SUM){
         output_schema = input_schema;
         return;
    }

    Attribute *input_atts = input_schema->GetAtts();
    numAttsInput = input_schema->GetNumAtts();
    keep_atts = (Attribute*) malloc (numAttsInput * sizeof(Attribute));
    keepMe = (int*) malloc (numAttsInput * sizeof(int));
    numAttsOutput = 0;
    while (atts != NULL) {
        char *att_name = atts->name;
        // std::cout << "^^^^^^^^^^^^^^^^^^^^^^" << std::endl;
        // std::cout << att_name << std::endl;
        int idx = input_schema->Find(att_name);
        if (idx == NOT_FOUND) {
            throw runtime_error("[Error] In function ProjectNode::ProjectNode(NameList *atts, QueryPlanNode *child): Trying to project non-existing attribute");
        }
        keepMe[numAttsOutput] = idx;
        keep_atts[numAttsOutput].name = att_name;
        keep_atts[numAttsOutput].myType = input_atts[idx].myType;
        numAttsOutput++;
        atts = atts->next;
    }

    output_schema = new Schema("", numAttsOutput, keep_atts);
    //delete input_schema;
    //processJoinChild(this);

}
/*
ProjectNode::ProjectNode(int *keepMe, int numAttsInput, int numAttsOutput, QueryPlanNode *child) {

}
*/      
ProjectNode::~ProjectNode() {
    free(keep_atts);
    free(keepMe);
    delete output_schema; 
    delete executor;
}

void ProjectNode::printSpecInfo() {
    // std::cout << "Attributes to keep:" << std::endl;
    // for (int i = 0; i < numAttsOutput; i++) {
    //     std::cout << "        Att " << keep_atts[i].name << ": " << TypeStr[keep_atts[i].myType] << "; ";
    // }
    // std::cout << std::endl;
}

void ProjectNode::execute(const std::map<int, Pipe*> &pipes) {
    Project *p = new Project();
    p->Run(*(pipes.at(left_pipe_id)), *(pipes.at(out_pipe_id)), keepMe, numAttsInput, numAttsOutput);
    executor = p;
}

JoinNode::JoinNode() {}

JoinNode::JoinNode(AndList *join_statement, QueryPlanNode *left_child, QueryPlanNode *right_child) {
    if (left_child == NULL ||  right_child == NULL) {
        throw runtime_error("[Error] In function JoinNode::JoinNode(QueryPlanNode *left_child, QueryPlanNode *right_child, CNF *selOp, Record *literal): left and right children must be both not null");
    }
    name = "Join";
    type = JOIN;
    num_children = DOUBLE;
    left = left_child;
    right = right_child;

    processJoinChild(this);

    if (left->type == JOIN){
        left_pipe_id = left->intermediate_out_pipe_id;
    }
    else{
        left_pipe_id = left->out_pipe_id;
    }
    if (right->type == JOIN){
        right_pipe_id = right->intermediate_out_pipe_id;
    }
    else{
        right_pipe_id = right->out_pipe_id;
    }
    //out_pipe_id = getNewPipeID(); 
    intermediate_out_pipe_id = getNewPipeID();

    Schema *left_sch = NULL, *right_sch = NULL;
    if (left_reader_from_join) {
        left_sch = new Schema(*(left_reader_from_join->output_schema));
    }
    else {
        left_sch = new Schema(*(left->output_schema));
    }
    if (right_reader_from_join) {
        right_sch = new Schema(*(right_reader_from_join->output_schema));
    }
    else {
        right_sch = new Schema(*(right->output_schema));
    }

    cnf = new CNF();
    literal = new Record();
    /*
    cnf->GrowFromParseTree(join_statement, left->output_schema, right->output_schema, *literal); 
	//this->literal = literal;
	numAttsLeft = left->output_schema->GetNumAtts();
    numAttsRight = right->output_schema->GetNumAtts();
	*/
    cnf->GrowFromParseTree(join_statement, left_sch, right_sch, *literal);
    Util::addRelNameBackToPred(join_statement);
    CNF_string = Util::ParseTreeToString(join_statement);
	//this->literal = literal;
	numAttsLeft = left_sch->GetNumAtts();
    numAttsRight = right_sch->GetNumAtts();
    numAttsToKeep = numAttsLeft + numAttsRight;
	startOfRight = numAttsLeft;

    // keep all attributes from both tables
    attsToKeep = (int*) malloc (numAttsToKeep * sizeof(int));
    output_atts = (Attribute*) malloc (numAttsToKeep * sizeof(Attribute));
    for (int i = 0; i < numAttsLeft; i++) {
        attsToKeep[i] = i;
        output_atts[i] = left_sch->GetAtts()[i];
    }
    for (int i = 0; i < numAttsRight; i++) {
        attsToKeep[i+numAttsLeft] = i;
        output_atts[i+numAttsLeft] = right_sch->GetAtts()[i];
    }
    
    output_schema = new Schema("", numAttsToKeep, output_atts);

    std::string random_suffix = Util::randomStr(10);
    intermediate_rel_name = intermediate_file_prefix + std::string("_") + random_suffix;
    intermediate_file = intermediate_rel_name + std::string(tpch_ext_name);
    intermediate_catalog = intermediate_catalog_prefix + "_" + random_suffix;
    //std::cout << "1" <<std::endl;
    saveSchema(std::string(intermediate_catalog), intermediate_rel_name);
    //std::cout << "2" <<std::endl;
    // writer = new WriteOutNode(std::string(tpch_dir) + intermediate_file, this);
    //std::cout << "Writer in join, in pipe: " << writer->left_pipe_id << std::endl;
    //writer->print();
    //std:cout << std::endl;
    //reader = new SelectNode(SELECT_FILE, NULL, NULL, true, intermediate_catalog.c_str(), intermediate_rel_name.c_str());
    //reader->print();
    
    //out_pipe_id = reader->out_pipe_id;
    //processJoinChild(this);
    //delete left_sch;
    //delete right_sch;

}
/*
void JoinNode::saveSchema(std::string save_to) {
    std::ofstream out;
    out.open(save_to.c_str(), std::ofstream::trunc | std::ofstream::out);
    //std::cout << "catalog create: " << out.is_open() << std::endl;
    std::string out_str = "BEGIN\n" + intermediate_rel_name + "\n" + intermediate_file + "\n";
    for (int i = 0; i < output_schema->numAtts; i++) {
        out_str += std::string(output_schema->GetAtts()[i].name) + " " + TypeStr[output_schema->GetAtts()[i].myType] + "\n";
    }
    out_str += "END\n";
    out << out_str;
    out.close();
    //std::cout << "catalog close: " << !out.is_open() << std::endl;
}
*/
JoinNode::~JoinNode() {
    delete cnf;
    delete literal;
    free(attsToKeep);
    free(output_atts);
    delete output_schema;
    //delete executor;
    //delete writer;
    //delete reader;
}

void JoinNode::printSpecInfo() {
    std::cout << "CNF:" << std::endl;
    std::cout << "        " + CNF_string << std::endl;
    // cnf->Print();
}

void JoinNode::execute(const std::map<int, Pipe*> &pipes) {
    Join *j = new Join();
    j->Run(*(pipes.at(left_pipe_id)), *(pipes.at(right_pipe_id)), *(pipes.at(intermediate_out_pipe_id)), *cnf, *literal,
            numAttsLeft, numAttsRight, attsToKeep, numAttsToKeep, startOfRight);
    //executor = j;
    writer->execute(pipes);
    //std::cout << "Good000" << std::endl;
    j->WaitUntilDone();
    //std::cout << "Good111" << std::endl;
    writer->executor->WaitUntilDone();
    //std::cout << "Good" << std::endl;
    
    delete j;
    //fclose(writer->output_file);
    //writer->output_file = NULL;
    
    //fclose(writer->output_file);
    //writer->output_file = NULL;
    delete writer;

    //saveSchema(intermediate_catalog);

    //printFile(std::string(tpch_dir) + intermediate_file);
    //exit(1);
    //reader->execute(pipes);
    //executor = reader->executor;
    executor = NULL;//writer->executor;
    //sleep(2);
}

SumNode::SumNode() {}

SumNode::SumNode(FuncOperator *agg_func, QueryPlanNode *child) {
    name = "Sum";
    type = SUM;
    num_children = SINGLE;
    left = child;

    processJoinChild(this);
    if (left->type == JOIN){
        left_pipe_id = child->intermediate_out_pipe_id;// IDs of left and right input pipes corresponding to left and right children
    }
    else{
        left_pipe_id = child->out_pipe_id;
    } 
    out_pipe_id = getNewPipeID(); // ID of output pipe
    // std::cout << "Sum child Node name: " << child->name << std::endl;
    // std::cout << "Sum Node in_pipe_id: " << left_pipe_id << std::endl;
    Schema *left_sch = NULL;
    if (left_reader_from_join) {
        left_sch = new Schema(*(left_reader_from_join->output_schema));
    }
    else {
        left_sch = new Schema(*(left->output_schema));
    }

    // std::cout << "SUM input sch: " << left_sch->toString() << std::endl;

    func = new Function();
    //func->GrowFromParseTree(agg_func, *(left->output_schema));
    func->GrowFromParseTree(agg_func, *left_sch);
    Type sum_type = func->getReturnType();

    

    Attribute *output_atts = new Attribute[MAX_NUM_ATTS];
    output_atts[0].name = "sum";
    output_atts[0].myType = func->getReturnType();
    output_schema = new Schema("", 1, output_atts);
    // if (!groupingAtts){
        
    //     output_schema = new Schema("", 1, output_atts);
    // }
    // else{
    //     NameList *group_att_names = groupingAtts;
    //     Attribute *group_atts = new Attribute[MAX_NUM_ATTS];
    //     int num_atts = 0;
    //     std::string numAttsStr, whichAttsStr, whichTypesStr;
    //     while (group_att_names != NULL) {
    //         // printf("1\n");
    //         group_atts[num_atts].name = (char*) malloc (MAX_LEN_ATTNAME * sizeof(char));
    //         strcpy(group_atts[num_atts].name, group_att_names->name);
    //         //group_atts[num_atts].myType = left->output_schema->FindType(group_att_names->name);
    //         group_atts[num_atts].myType = left_sch->FindType(group_att_names->name);
            
    //         int idx = left_sch->Find(group_att_names->name);
    //         if (idx == NOT_FOUND) {
    //             throw runtime_error("[Error] In function GroupByNode::GroupByNode(FuncOperator *agg_func, NameList *group_att_names, QueryPlanNode *child): Trying to group by non-existing attribute");
    //         }
    //         if (num_atts > 0) {
    //             whichAttsStr += " ";
    //             whichTypesStr += " ";
    //         }
    //         whichAttsStr += Util::toString<int>(idx);
    //         whichTypesStr += TypeStr[left_sch->FindType(group_att_names->name)];

    //         std::cout << group_att_names->name << std::endl;
    //         std::cout << left_sch->FindType(group_att_names->name) << std::endl;

    //         num_atts++;
    //         group_att_names = group_att_names->next;
    //     }
    //     std::cout << num_atts << std::endl;
    //     for (int i = 0; i < num_atts; i++) {
    //         output_atts[1+i].name = (char*) malloc (MAX_LEN_ATTNAME * sizeof(char));
    //         strcpy(output_atts[1+i].name, group_atts[i].name);
    //         output_atts[1+i].myType = group_atts[i].myType;
    //     }
    //     output_schema = new Schema("", num_atts+1, output_atts);
    // }

}

SumNode::~SumNode() {
    delete func;
    delete output_schema;
    delete executor;
}

void SumNode::printSpecInfo() {
    // std::cout << "Aggregate function (in Reverse Polish notation): " << std::endl;
    std::cout << "Function (in Reverse Polish notation): " << std::endl;
    std::cout << "        SUM ( ";
    func->Print();
    std::cout << ")" << std::endl;
}

void SumNode::execute(const std::map<int, Pipe*> &pipes) {
    Sum *s = new Sum();
    s->Run(*(pipes.at(left_pipe_id)), *(pipes.at(out_pipe_id)), *func);
    executor = s;
}

GroupByNode::GroupByNode() {}

GroupByNode::GroupByNode(FuncOperator *agg_func, NameList *group_att_names, QueryPlanNode *child) {
    name = "Group By";
    type = GROUP_BY;
    num_children = SINGLE;
    left = child;
    left_pipe_id = left->out_pipe_id; 
    
    processJoinChild(this);

    if (left->type == JOIN){
        left_pipe_id = child->intermediate_out_pipe_id;
    }
    else{
        left_pipe_id = child->out_pipe_id;
    }
    out_pipe_id = getNewPipeID(); // ID of output pipe

    Schema *left_sch = NULL;
    if (left_reader_from_join) {
        left_sch = new Schema(*(left_reader_from_join->output_schema));
    }
    else {
        left_sch = new Schema(*(left->output_schema));
    }

    // Make OrderMaker
    //Attribute *group_atts = (Attribute*) malloc (MAX_NUM_ATTS * sizeof(Attribute));
    
    Attribute *group_atts = new Attribute[MAX_NUM_ATTS];
    int num_atts = 0;
    std::string numAttsStr, whichAttsStr, whichTypesStr;
    while (group_att_names != NULL) {

        // std::cout << "group_att_names->name :" << std::endl;
        // std::cout << group_att_names->name << std::endl;

        group_atts[num_atts].name = (char*) malloc (MAX_LEN_ATTNAME * sizeof(char));
        strcpy(group_atts[num_atts].name, group_att_names->name);
        //group_atts[num_atts].myType = left->output_schema->FindType(group_att_names->name);
        // std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << std::endl;
        // std::cout << group_att_names->name << std::endl;
        // std::cout << "TESTING FINDTYPE: " << std::endl;
        group_atts[num_atts].myType = left_sch->FindType(group_att_names->name);
        // std::cout << "TESTING END: " << std::endl;
        // std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << std::endl;
        // std::cout << left_sch->toString() << std::endl;

        
        int idx = left_sch->Find(group_att_names->name);
        if (idx == NOT_FOUND) {
            throw runtime_error("[Error] In function GroupByNode::GroupByNode(FuncOperator *agg_func, NameList *group_att_names, QueryPlanNode *child): Trying to group by non-existing attribute");
        }
        if (num_atts > 0) {
            whichAttsStr += " ";
            whichTypesStr += " ";
        }
        whichAttsStr += Util::toString<int>(idx);
        whichTypesStr += TypeStr[left_sch->FindType(group_att_names->name)];

        std::cout << "GROUPING ON " << group_att_names->name << std::endl;
        std::string output = "";
        output += "        Att " + std::string(group_att_names->name) + ": " + TypeStr[group_atts[num_atts].myType] + "\n";
        std::cout << output << std::endl;

        num_atts++;
        group_att_names = group_att_names->next;
    }
    //Schema group_sch("", num_atts, group_atts);
    numAttsStr = Util::toString<int>(num_atts);

    // std::cout << "GROUPBY input sch: " << std::endl << left_sch->toString() << std::endl;
    
    order = new OrderMaker(numAttsStr, whichAttsStr, whichTypesStr);

    // std::cout << "Order: " << std::endl;
    // order->Print();

    output_schema = left_sch;
    
    // func = new Function();
    // //func->GrowFromParseTree(agg_func, *(left->output_schema));
    // func->GrowFromParseTree(agg_func, *left_sch);

    // Attribute *output_atts = new Attribute[MAX_NUM_ATTS];
    // output_atts[0].name = "sum";
    // output_atts[0].myType = func->getReturnType();
    // for (int i = 0; i < num_atts; i++) {
    //     output_atts[1+i].name = (char*) malloc (MAX_LEN_ATTNAME * sizeof(char));
    //     strcpy(output_atts[1+i].name, group_atts[i].name);
    //     output_atts[1+i].myType = group_atts[i].myType;
    // }

    // output_schema = new Schema("", num_atts+1, output_atts);

}

GroupByNode::~GroupByNode() {
    delete order;
    delete func;
    delete output_schema;
    delete executor;
}

void GroupByNode::printSpecInfo() {
    std::cout << "OrderMaker: " << std::endl;
    order->Print();
    // // std::cout << "Aggregate function (in Reverse Polish notation): " << std::endl;
    // std::cout << "SUM ( ";
    // func->Print();
    // std::cout << ") (in Reverse Polish notation): " << std::endl;
}

void GroupByNode::execute(const std::map<int, Pipe*> &pipes) {
    GroupBy *g = new GroupBy();
    g->Run(*(pipes.at(left_pipe_id)), *(pipes.at(out_pipe_id)), *order, *func);
    executor = g;
    //std::cout << "====================== GROUP BY START ===================" << std::endl;
}

DupRemovalNode::DupRemovalNode() {}

DupRemovalNode::DupRemovalNode(QueryPlanNode *child) {
    // name = "Duplicate Removal";
    name = "Distinct";
    type = DUP_REMOVAL;
    num_children = SINGLE;
    left = child;
    
    processJoinChild(this);

    if (child->type == JOIN){
        left_pipe_id = child->intermediate_out_pipe_id;
    }
    else{
        left_pipe_id = child->out_pipe_id;
    }
    out_pipe_id = getNewPipeID();
    Schema *left_sch = NULL;
    if (left_reader_from_join) {
        left_sch = new Schema(*(left_reader_from_join->output_schema));
    }
    else {
        left_sch = new Schema(*(left->output_schema));
    }

    //output_schema = new Schema(*(left->output_schema));
    output_schema = new Schema(*(left_sch));

    //delete left_sch;
}

DupRemovalNode::~DupRemovalNode() {
    delete output_schema;
    delete executor;
}

void DupRemovalNode::printSpecInfo() {}

void DupRemovalNode::execute(const std::map<int, Pipe*> &pipes) {
    DuplicateRemoval *dr = new DuplicateRemoval();
    dr->Run(*(pipes.at(left_pipe_id)), *(pipes.at(out_pipe_id)), *output_schema);
    executor = dr;
}

WriteOutNode::WriteOutNode() {}

WriteOutNode::WriteOutNode(std::string out_filepath, QueryPlanNode *child) {
    // No need for processJoinChild() because child of WriteOutNode cannot be JoinNode
    name = "Write Out";
    type = WRITE_OUT;
    num_children = SINGLE;
    left = child;
    
    output_filepath = out_filepath;
    file_id = next_assign_fileID++;
    output_file = NULL;

    if (left->getType() == JOIN) {
        left_pipe_id = left->intermediate_out_pipe_id;
    }
    else {
        left_pipe_id = left->out_pipe_id; 
    }
    
    output_schema = new Schema(*(left->output_schema));
}

WriteOutNode::~WriteOutNode() {
    if (output_file != NULL){
        fclose(output_file);
        output_file = NULL;
    }
    delete output_schema;
    delete executor;
}

void WriteOutNode::printSpecInfo() {
    std::cout << "Write to file: " << output_filepath << std::endl;
}

void WriteOutNode::execute(const std::map<int, Pipe*> &pipes) {
    output_file = fopen(output_filepath.c_str(), "w+");
    WriteOut *w = new WriteOut();
    w->Run(*(pipes.at(left_pipe_id)), output_file, *output_schema);
    executor = w;

    //processJoinChild(this);

}