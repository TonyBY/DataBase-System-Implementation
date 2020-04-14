#include "QueryOptimizer.h"

char* QueryPlan::catalog_path = "catalog"; 
int QueryPlanNode::next_assign_pipeID = 0;
int QueryPlanNode::next_assign_fileID = 0;

//QueryPlan::QueryPlan() {}

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

void printInOrderQueryPlan(QueryPlanNode *root) {
    if (root == NULL) {
        return;
    }
    printInOrderQueryPlan(root->left);
    root->print();
    printInOrderQueryPlan(root->right);
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

QueryPlan::QueryPlan(Statistics *st) {
    initPrefixMap();        
    stat = st;
    // initialize optimal predicate as the original input predicate
    optimal_predicate = boolean;
    // remove relation name from predicate, like "l.l_name" -> "l_name". 
    // It is because our Statistics does not support predicate with relation name prefix.
    Util::removeRelNameFromPred(optimal_predicate);
    Util::removeRelNameFromPred(attsToSelect);
    Util::removeRelNameFromPred(groupingAtts);
    Util::removeRelNameFromPred(finalFunction);
    // then optimize it
    optimizePredicate();

    min_order = Util::splitAndList(optimal_predicate);
}

const QueryPlanNode* QueryPlan::getRoot() {
    return root;
}

void QueryPlan::createPlanTree() {
    createLoadDataNodes();  
    //for (int i = 0 ; i < leaves.size(); i++) {
    //    leaves[i]->print();
    //}
    createPredicateNodes();
    //printQueryPlanTree(root);
    createProjectNodes();
    createDupRemovalNodes();
    createSumNodes();
    createGroupByNodes();
    createWriteOutNodes();
}

// create nodes for loading data 
void QueryPlan::createLoadDataNodes() {
    // create nodes for loading data from files

    Util::ParseTreeToString(min_order);

    std::map<std::string, int> all_need_rels;
    for (int i = 0;i < min_order.size(); i++) {
        std::vector<std::string> relnames = getRelNames(min_order[i]);
        for (int j = 0; j < relnames.size(); j++) {
            all_need_rels[relnames[j]] = 1;
        }
    }
    for (std::map<std::string, int>::iterator it = all_need_rels.begin(); it != all_need_rels.end(); it++) {
        SelectNode *leaf = new SelectNode(SELECT_FILE, NULL, NULL, catalog_path, (char*)(it->first.c_str()));
        leaves.push_back(leaf);
        relname_node[it->first] = leaf;
        if (node_relnames.find(leaf) == node_relnames.end()) {
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
        std::vector<std::string> relnames = getRelNames(min_order[i]);
        if (relnames.size() == 1) {
            // select pipe node
            std::string cur_rel = relnames[0];
            QueryPlanNode *child = relname_node[cur_rel];
            SelectNode *select_pipe_node = new SelectNode(SELECT_PIPE, min_order[i], child, catalog_path, (char*)(cur_rel.c_str()));
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
            JoinNode *join_node = new JoinNode(min_order[i], left, right);
            //relname_node[cur_rel_left] = join_node;
            //relname_node[cur_rel_right] = join_node;
            updateRelnameNodeMap(join_node, left, right);
            new_root = join_node;
        }
        
    }
    this->root = new_root;
    return new_root;
}

void QueryPlan::createProjectNodes() {
    if (attsToSelect && !finalFunction && !groupingAtts) {
        QueryPlanNode *new_root = new ProjectNode(attsToSelect, this->root);
        this->root = new_root;
    }
}

void QueryPlan::createDupRemovalNodes() {
    if (distinctAtts != NULL || distinctFunc != NULL) {
        this->root = new DupRemovalNode(this->root);
        //throw runtime_error("Sorry, we are not supporting DISTINCT now, it is under developing...\n");
    }
}

void QueryPlan::createSumNodes() {
    QueryPlanNode *new_root = NULL;
    //if (groupingAtts) {
    //    if (distinctFunc) {
            //new_root = new DupRemovalNode();
    //        createDupRemovalNodes();
    //    }
        //new_root = new GroupByNode();
    //    createGroupByNodes();
    //} 
    if (finalFunction && !groupingAtts) {
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

void QueryPlan::createWriteOutNodes() {

}   

QueryPlan::~QueryPlan() {
    deleteQueryPlanTree(this->root);
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
    std::sort(clauses.begin(), clauses.end());
    do {
        //std::cout << Util::ParseTreeToString(clauses) << std::endl;
        double cur_cost = estimateTotalCost(clauses);
        //std::cout << "Cost: " << cur_cost << std::endl;
        if (cur_cost < min_cost) {
            min_cost = cur_cost;
            min_order = clauses;
        }
    } while (std::next_permutation(clauses.begin(), clauses.end()));

    optimal_predicate = Util::getParseTreeFromVector(min_order);
    std::cout << "Optimal predicate (cost " << min_cost << "): " << Util::ParseTreeToString(optimal_predicate) << std::endl;
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
    //char* relNames[MAX_NUM_RELS];
    //int numToJoin = 0;
//    if (rels.size() == 1) {
        // predicate is selection
//        relNames[0] = (char*)rels[0].c_str();
//        numToJoin = 1;
//    }
//    else {
        // join
        //std::vector<std::string> all_orig_rels;
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

std::vector<std::string> QueryPlan::getRelNames(AndList *single_pred){
    if (single_pred->rightAnd != NULL) {
        throw runtime_error("[Error] In function QueryPlan::getRelNames(AndList *predicate): Not a single AndList");
    }
    Operand* left_operand = single_pred->left->left->left;
    Operand* right_operand = single_pred->left->left->right;
        
    std::vector<std::string> relations;
    //std::string left_att_name = Util::getSuffix(std::string(left_operand->value), '.');
    std::string left_prefix = Util::getSuffix(Util::getPrefix(std::string(left_operand->value), '_'), '.');
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
    output_schema = NULL;
}

int QueryPlanNode::getNewPipeID() {
    int new_pipe_id = next_assign_pipeID;
    next_assign_pipeID++;
    return new_pipe_id;
}

int QueryPlanNode::getType() {
    return type;
}
/*
void QueryPlanNode::print(int arg_type, int arg_left_pipe_id, int arg_right_pipe_id, int arg_out_pipe_id, int arg_file_id) {
    std::cout << "Operation: " << name << std::endl;
    if (arg_left_pipe_id >= 0) {
        std::cout << "Input pipe ID " << arg_left_pipe_id << ", ";
        if (arg_right_pipe_id >= 0) {
            std::cout << "Input pipe ID " << arg_right_pipe_id << ", ";
        }
    }
    else if (arg_type == SELECT_FILE && arg_file_id >= 0) {
        std::cout << "Input file ID " << arg_file_id << ", ";
    }
    else {
        throw std::runtime_error("[Error] In function QueryPlanNode::print(): No associated input pipes or files in operation node " + name);
    }

    if (arg_out_pipe_id >= 0) {
        std::cout << "Output pipe ID " << arg_out_pipe_id << std::endl;
    }
    else {
        throw std::runtime_error("[Error] In function QueryPlanNode::print(): No associated output pipe in operation node " + name);
    }

    std::cout << "Output schema:" << std::endl;
    std::cout << output_schema->toString() << std::endl;
    printSpecInfo();
}
*/
void QueryPlanNode::print() {
    std::cout << "Operation: " << name << std::endl;
    if (left_pipe_id >= 0) {
        std::cout << "Input pipe ID " << left_pipe_id << ", ";
        if (right_pipe_id >= 0) {
            std::cout << "Input pipe ID " << right_pipe_id << ", ";
        }
    }
    else if (type == SELECT_FILE && file_id >= 0) {
        std::cout << "Input file ID " << file_id << ", ";
    }
    else {
        throw std::runtime_error("[Error] In function QueryPlanNode::print(): No associated input pipes or files in operation node " + name);
    }

    if (out_pipe_id >= 0) {
        std::cout << "Output pipe ID " << out_pipe_id << std::endl;
    }
    else {
        throw std::runtime_error("[Error] In function QueryPlanNode::print(): No associated output pipe in operation node " + name);
    }

    std::cout << "Output schema:" << std::endl;
    std::cout << output_schema->toString() << std::endl;
    printSpecInfo();
    std::cout << std::endl;
}



SelectNode::SelectNode() {}

SelectNode::SelectNode(int select_where, AndList *select_conditions, QueryPlanNode *child, 
                        char *catalog_path, char *rel_name, char* alias) {
    //file_id = Undefined;
    load_all_data = false;
    if (select_where == SELECT_FILE) {
        name = "SelectFile";
        if (child != NULL) {
            throw runtime_error("[Error] In function SelectNode::SelectNode: Trying to initialize a SelectFile node with a child");
        }
        num_children = 0;
        file_id = next_assign_fileID++;
        //next_assign_fileID++;
    }
    else if (select_where == SELECT_PIPE) {
        name = "SelectPipe";
        if (child == NULL) {
            throw runtime_error("[Error] In function SelectNode::SelectNode: Trying to initialize a SelectPipe node without a child");
        }
        num_children = 1;
        left_pipe_id = child->out_pipe_id;
        left = child;
    }
    else {
        throw runtime_error("[Error] In function SelectNode::SelectNode: Invalid data source type '" + Util::toString<int>(select_where) + "'");
    }
    
    type = select_where;
    
    //out_pipe_id = next_assign_pipeID++; 
    out_pipe_id = getNewPipeID();
    
    output_schema = new Schema(catalog_path, rel_name);
   
    if (select_conditions != NULL){
        //this->cnf = cnf;
        load_all_data = false;
        cnf = new CNF();
        literal = new Record();
        cnf->GrowFromParseTree(select_conditions, output_schema, *literal);
    }
    else {
        load_all_data = true;
        cnf = NULL;
        literal = NULL;
    }
    
}

SelectNode::~SelectNode() {
    delete output_schema;
    delete cnf;
    delete literal;
}

void SelectNode::printSpecInfo() {
    std::cout << "CNF:" << std::endl;
    if (!load_all_data){
        cnf->Print();
    }
    else {
        std::cout << "No CNF because this node is only for loading all data" << std::endl;
    }
}

void SelectNode::execute() {

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
    if (atts == NULL || child == NULL) {
        throw runtime_error("[Error] In function ProjectNode::ProjectNode(NameList *atts, QueryPlanNode *child): atts and child must be both not null");
    }
    name = "Project";
    type = PROJECT;
    num_children = SINGLE;
    left = child;
    left_pipe_id = child->out_pipe_id;
    //out_pipe_id = next_assign_pipeID++;
    out_pipe_id = getNewPipeID();

    Schema *input_schema = child->output_schema;
    Attribute *input_atts = input_schema->GetAtts();
    
    numAttsInput = input_schema->GetNumAtts();
    keep_atts = (Attribute*) malloc (numAttsInput * sizeof(Attribute));
    keepMe = (int*) malloc (numAttsInput * sizeof(int));
    numAttsOutput = 0;
    while (atts != NULL) {
        char *att_name = atts->name;
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
}
/*
ProjectNode::ProjectNode(int *keepMe, int numAttsInput, int numAttsOutput, QueryPlanNode *child) {

}
*/      
ProjectNode::~ProjectNode() {
    free(keep_atts);
    free(keepMe);
    delete output_schema; 
}

void ProjectNode::printSpecInfo() {
    std::cout << "Attributes to keep:" << std::endl;
    for (int i = 0; i < numAttsOutput; i++) {
        std::cout << keep_atts[i].name << ": " << TypeStr[keep_atts[i].myType] << "; ";
    }
    std::cout << std::endl;
}

void ProjectNode::execute() {
    
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
    left_pipe_id = left->out_pipe_id;
    right_pipe_id = right->out_pipe_id; 
    out_pipe_id = getNewPipeID(); 
    
    cnf = new CNF();
    literal = new Record();
    cnf->GrowFromParseTree(join_statement, left->output_schema, right->output_schema, *literal); 
	//this->literal = literal;
	numAttsLeft = left->output_schema->GetNumAtts();
    numAttsRight = right->output_schema->GetNumAtts();
	numAttsToKeep = numAttsLeft + numAttsRight;
	startOfRight = numAttsLeft;

    // keep all attributes from both tables
    attsToKeep = (int*) malloc (numAttsToKeep * sizeof(int));
    output_atts = (Attribute*) malloc (numAttsToKeep * sizeof(Attribute));
    for (int i = 0; i < numAttsLeft; i++) {
        attsToKeep[i] = i;
        output_atts[i] = left->output_schema->GetAtts()[i];
    }
    for (int i = 0; i < numAttsRight; i++) {
        attsToKeep[i+numAttsLeft] = i;
        output_atts[i+numAttsLeft] = right->output_schema->GetAtts()[i];
    }
    
    output_schema = new Schema("", numAttsToKeep, output_atts);

}


JoinNode::~JoinNode() {
    delete cnf;
    delete literal;
    free(attsToKeep);
    free(output_atts);
    delete output_schema;
}

void JoinNode::printSpecInfo() {
    std::cout << "CNF:" << std::endl;
    cnf->Print();
}

void JoinNode::execute() {

}

SumNode::SumNode() {}

SumNode::SumNode(FuncOperator *agg_func, QueryPlanNode *child) {
    name = "Sum";
    type = SUM;
    num_children = SINGLE;
    left = child;
    left_pipe_id = left->out_pipe_id; // IDs of left and right input pipes corresponding to left and right children
    out_pipe_id = getNewPipeID(); // ID of output pipe
    
    func = new Function();
    func->GrowFromParseTree(agg_func, *(left->output_schema));
    Type sum_type = func->getReturnType();

    Attribute output_atts = {"sum", sum_type};
    output_schema = new Schema("", 1, &output_atts);

}

SumNode::~SumNode() {
    delete func;
    delete output_schema;
}

void SumNode::printSpecInfo() {
    std::cout << "Aggregation Function (represented in Reverse Polish notation):" << std::endl;
    func->Print();
}

void SumNode::execute() {

}

GroupByNode::GroupByNode() {}

GroupByNode::GroupByNode(FuncOperator *agg_func, NameList *group_att_names, QueryPlanNode *child) {
    name = "Group By";
    type = GROUP_BY;
    num_children = SINGLE;
    left = child;
    left_pipe_id = left->out_pipe_id; 
    out_pipe_id = getNewPipeID(); // ID of output pipe
    
    // Make OrderMaker
    //Attribute *group_atts = (Attribute*) malloc (MAX_NUM_ATTS * sizeof(Attribute));
    Attribute *group_atts = new Attribute[MAX_NUM_ATTS];
    int num_atts = 0;
    while (group_att_names != NULL) {
        group_atts[num_atts].name = group_att_names->name;
        group_atts[num_atts].myType = left->output_schema->FindType(group_att_names->name);
        num_atts++;
        group_att_names = group_att_names->next;
    }
    Schema group_sch("", num_atts, group_atts);
    order = new OrderMaker(&group_sch);

    func = new Function();
    func->GrowFromParseTree(agg_func, *(left->output_schema));

    Attribute *output_atts = new Attribute[MAX_NUM_ATTS];
    output_atts[0].name = "sum";
    output_atts[0].myType = func->getReturnType();
    for (int i = 0; i < num_atts; i++) {
        output_atts[1+i] = group_atts[i];
    }

    output_schema = new Schema("", num_atts+1, output_atts);

    delete[] group_atts;
    delete[] output_atts;
}

GroupByNode::~GroupByNode() {
    delete order;
    delete func;
    delete output_schema;
}

void GroupByNode::printSpecInfo() {
    std::cout << "OrderMaker: " << std::endl;
    order->Print();
    std::cout << "Aggregate function (represented in Reverse Polish notation): " << std::endl;
    func->Print();
}

void GroupByNode::execute() {

}

DupRemovalNode::DupRemovalNode() {}

DupRemovalNode::DupRemovalNode(QueryPlanNode *child) {
    name = "Duplicate Removal";
    type = DUP_REMOVAL;
    num_children = SINGLE;
    left = child;
    left_pipe_id = left->out_pipe_id; // IDs of left and right input pipes corresponding to left and right children
    out_pipe_id = getNewPipeID(); // ID of output pipe
    output_schema = new Schema(*(left->output_schema));
}

DupRemovalNode::~DupRemovalNode() {
    delete output_schema;
}

void DupRemovalNode::printSpecInfo() {}

void DupRemovalNode::execute() {

}