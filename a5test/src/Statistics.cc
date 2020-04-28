#include "Statistics.h"

AttributeInfo::AttributeInfo() {}

AttributeInfo::AttributeInfo(std::string att_name, int num_distincts) 
{
	this->name = att_name;
	this->numDistincts = num_distincts;
}

std::string AttributeInfo::toString()
{
	std::string output;
	output += name + "\n" + 
				Util::toString<int>(numDistincts);
	return output;
}

RelationInfo::RelationInfo() {}

RelationInfo::RelationInfo(std::string rel_name, int num_tuples)
{
	this->name = rel_name;
	this->numTuples = num_tuples;
}

std::string RelationInfo::toString()
{
	std::string output;
	output += name + "\n" + 
				Util::toString<int>(numTuples) + "\n";
			
	int num_atts = data.size();
	output += Util::toString<int>(num_atts) + "\n";
	for (std::map<std::string, AttributeInfo>::iterator it = data.begin(); it != data.end(); it++) 
	{
		output += it->second.toString() + "\n";
	}

	return output;
}

Statistics::Statistics()
{
    this->final_est_num_tuples = -1;
}

// deep copy 
Statistics::Statistics(Statistics &copyMe)
{
    this->data = copyMe.data;
    for (std::map<std::string, RelationInfo*>::iterator it = copyMe.joined_relations.begin(); it != copyMe.joined_relations.end(); it++)
    {
        this->joined_relations[it->first] = &(this->data[it->second->name]);
    }
    this->final_est_num_tuples = copyMe.final_est_num_tuples;
}

Statistics::~Statistics()
{
}

// Update allowed
void Statistics::AddRel(char *relName, int numTuples)
{
    std::string relation_name(relName);
    if (exists<std::string, RelationInfo>(data, relation_name))
    {
        data[relation_name].numTuples = numTuples;
    }
    else
    { 
        data[relation_name] = RelationInfo(relation_name, numTuples);
        joined_relations[relation_name] = &(data.at(relation_name));
    }  
}

void Statistics::AddAtt(char *relName, char *attName, int numDistincts)
{
    std::string relation_name(relName);
    std::string att_name(attName);
    if (!exists<std::string, RelationInfo>(data, relation_name))
    {
        std::cerr << "[Error] In function Statistics::AddAtt(): Relation '" << relation_name << "' does not exist in Statistics" << std::endl;     
    }
    else
    { 
        if (numDistincts == -1) 
        {
            numDistincts = data[relation_name].numTuples;
        }
    
        if (exists<std::string, AttributeInfo>(data[relation_name].data, att_name))
        { 
            data[relation_name].data[att_name].numDistincts = numDistincts;
        }
        else 
        {
            data[relation_name].data[att_name] = AttributeInfo(att_name, numDistincts);
        }
    }
}

void Statistics::CopyRel(char *oldName, char *newName)
{
    std::string old_name(oldName);
    std::string new_name(newName);
    
    if (!exists<std::string, RelationInfo>(data, old_name))
    {
        std::cerr << "[Error] In function Statistics::CopyRel(): Old Relation '" << old_name << "' does not exist in Statistics" << std::endl;     
    }
    else 
    {
        if (old_name != new_name)
        {
            data[new_name] = data[old_name];
            data[new_name].name = new_name;
            joined_relations[new_name] = &(data[new_name]);
        }
    }
}

std::string Statistics::joinedRelToStr()
{
    std::string output;
    int join_rel_size = joined_relations.size();
    output += Util::toString<int>(join_rel_size) + "\n";
    for (std::map<std::string, RelationInfo*>::iterator it = joined_relations.begin(); it != joined_relations.end(); it++)
    {
        output += it->first + "\n" + it->second->name + "\n";
    }
    return output;
}

void Statistics::Read(char *fromWhere)
{
    std::ifstream in_file(fromWhere);
    if (!in_file.is_open()) 
    { 
        std::ofstream empty_file(fromWhere);
        empty_file.close();
        std::cerr << "[Warning] In function Statistics::Read(): trying to read an object from a non-existing file" << std::endl;
    }
    else if (Util::isEmptyFile(in_file))
    {
        std::cerr << "[Warning] In function Statistics::Read(): trying to read an object from an empty file" << std::endl;
    }
    else
    {
        std::string relation_name, att_name;
        int num_relations, num_atts, num_tuples, num_distincts, num_joined_rels;
        
        std::string line;

        in_file >> num_relations;
        for (int i = 0; i < num_relations; i++) 
        {
            in_file >> relation_name;
            in_file >> num_tuples;
            AddRel((char*)(relation_name.c_str()), num_tuples);

            in_file >> num_atts;
            for (int j = 0; j < num_atts; j++) 
            {
                in_file >> att_name;
                in_file >> num_distincts;
                AddAtt(((char*)relation_name.c_str()), ((char*)att_name.c_str()), num_distincts);
            }
        } 

        in_file >> num_joined_rels;
        for (int i = 0; i < num_joined_rels; i++)
        {
            std::string single_rel_name, joined_rel_name;
            in_file >> single_rel_name;
            in_file >> joined_rel_name;
            joined_relations[single_rel_name] = &(data[joined_rel_name]);
        }
    }
}

void Statistics::Write(char *fromWhere)
{
    std::ofstream out_file;
    out_file.open(fromWhere);
    std::string output;
    
    int num_relations = data.size();
    output += Util::toString<int>(num_relations) + "\n";
    for (std::map<std::string, RelationInfo>::iterator it = data.begin(); it != data.end(); it++) 
    {
        output += it->second.toString();
    }
    output += joinedRelToStr();

    out_file << output;
    out_file.close();
}

bool Statistics::moreThan2Partitions(RelationInfo **partition1, RelationInfo **partition2, char *relNames[], int numToJoin)
{
    *partition1 = *partition2 = NULL;
    for (int i = 0; i < numToJoin; i++)
    {
        RelationInfo *tmp = joined_relations.at(std::string(relNames[i]));
        if (*partition1 == NULL)
        {
            (*partition1) = tmp;
        }
        else 
        {
            if (tmp != *partition1)
            {
                if (*partition2 == NULL)
                {
                    (*partition2) = tmp;
                }
                else
                {
                    if (tmp != *partition2)
                    {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

RelationInfo* Statistics::applyJoin(double &est_num_tuples_after_join, 
                                    RelationInfo *relation1, RelationInfo *relation2, 
                                    const std::string &att_name_left, const std::string &att_name_right, 
                                    double est_num_tuples1, double est_num_tuples2, 
                                    char *relNames[], int numToJoin)
{
    RelationInfo *relation_left = NULL, *relation_right = NULL;
    int num_dictincts_left = 0, num_dictincts_right = 0;
    if (exists<std::string, AttributeInfo>(relation1->data, att_name_left) && exists<std::string, AttributeInfo>(relation2->data, att_name_right))
    {
        relation_left = relation1;
        relation_right = relation2;
    }
    else if (exists<std::string, AttributeInfo>(relation2->data, att_name_left) && exists<std::string, AttributeInfo>(relation1->data, att_name_right))
    {
        relation_left = relation2;
        relation_right = relation1;
    }
    else
    {
        throw std::runtime_error("[Error] In function Statistics::applyJoin(): cannot find both of attributes '" 
                                    + att_name_left + "' and '" + att_name_right 
                                    + "' in relations '" + relation1->name + "' and '" + relation2->name + "'");
    }

    num_dictincts_left = relation_left->data.at(att_name_left).numDistincts;
    num_dictincts_right = relation_right->data.at(att_name_right).numDistincts;

    int larger = std::max(num_dictincts_left, num_dictincts_right);
    int new_num_tuples = relation_left->numTuples * (relation_right->numTuples / (double)larger);
    est_num_tuples_after_join = est_num_tuples1 * (est_num_tuples2 / (double)larger);

    std::string new_rel_name = relation_left->name + "&" + relation_right->name;
    AddRel((char*)(new_rel_name.c_str()), new_num_tuples);
    for (std::map<std::string, AttributeInfo>::iterator it = relation_left->data.begin(); it != relation_left->data.end(); it++)
    {
        AddAtt((char*)(new_rel_name.c_str()), (char*)(it->second.name.c_str()), it->second.numDistincts);
    } 
    for (std::map<std::string, AttributeInfo>::iterator it = relation_right->data.begin(); it != relation_right->data.end(); it++)
    {
        AddAtt((char*)(new_rel_name.c_str()), (char*)(it->second.name.c_str()), it->second.numDistincts);
    }
    
    // Update partitions (joined_relations)
    for (int i = 0; i < numToJoin; i++)
    {
        joined_relations[std::string(relNames[i])] = &(data.at(new_rel_name));
    }

    // Remove old relations
    data.erase(relation_left->name);
    data.erase(relation_right->name);
    joined_relations.erase(new_rel_name);

    return &(data.at(new_rel_name));
 
}

double Statistics::estimateSelectRatio(RelationInfo *relation, const std::string &att_name, bool select_by_equality)
{
    int num_dictincts = relation->data.at(att_name).numDistincts;

    if (select_by_equality)
    {
        return 1.0 / (double)num_dictincts;
    }
    else
    {
        return 1.0 / 3.0;
    }
}

std::vector<double> Statistics::estimateSelectRatio(RelationInfo *relation1, RelationInfo *relation2, struct OrList *or_list)
{
    RelationInfo *relations[2] = {relation1, relation2};  
    RelationInfo *relation;
    int num_dictincts;
    struct ComparisonOp *comp_op;
    std::string att_name, prev_att_name;
    double ratio_num_tuples[2] = {-1, -1};
    std::vector<double> res_ratio;

    int which_rel = -1;
    bool select_by_equality = false;

    while (or_list != NULL)
    {
        comp_op = or_list->left;
        if (comp_op != NULL)
        {
            if (comp_op->left->code == NAME)
            {
                att_name = std::string(comp_op->left->value);
            }
            else if (comp_op->right->code == NAME)
            {
                att_name = std::string(comp_op->right->value);
            }

            if (relation1 != NULL && exists<std::string, AttributeInfo>(relation1->data, att_name))
            {
                relation = relation1;
                which_rel = 0;
            }
            else if (relation2 != NULL && exists<std::string, AttributeInfo>(relation2->data, att_name))
            {
                relation = relation2;
                which_rel = 1;
            }
            else 
            {
                std::string relation1_name, relation2_name, relation_names;  
                if (relation1 != NULL) {
                    relation1_name = relation1->name;
                }
                if (relation2 != NULL) {
                    relation2_name = relation2->name;
                }
                if (relation1_name != "" && relation2_name != "") {
                    relation_names = relation1_name + "' and '" + relation2_name;
                }
                else {
                    relation_names = relation1_name + relation2_name;
                }
                throw std::runtime_error("[Error] In function Statistics::applySelect(): cannot find attribute '" + att_name + "' in relation '" + relation_names + "'");
            }

            if (comp_op->code == EQUALS)
            {
                select_by_equality = true;
            }
            else
            {
                select_by_equality = false;
            }

            if (ratio_num_tuples[which_rel] < 0)
            {
                ratio_num_tuples[which_rel] = 0;
            }

            double cur_ratio = estimateSelectRatio(relation, att_name, select_by_equality);
            if (prev_att_name != att_name)
            {
                ratio_num_tuples[which_rel] = cur_ratio + ratio_num_tuples[which_rel] - (cur_ratio * ratio_num_tuples[which_rel]);
            }
            else
            {
                ratio_num_tuples[which_rel] += cur_ratio;
            }
            prev_att_name = att_name;
            
        }
        or_list = or_list->rightOr;
        
    }

    for (int i = 0; i < 2; i++)
    {
        res_ratio.push_back(ratio_num_tuples[i]);
    }
    
    return res_ratio;

}

void Statistics::applySelect(RelationInfo *relation1, RelationInfo *relation2, 
                             double &est_num_tuples1, double &est_num_tuples2,   
                             struct OrList *or_list)
{
    RelationInfo *relations[2] = {relation1, relation2};
    double *est_num_tuples[2] = {&est_num_tuples1, &est_num_tuples2};
    std::vector<double> ratio = estimateSelectRatio(relation1, relation2, or_list);
    for (int i = 0; i < ratio.size(); i++)
    {
        if (ratio[i] >= 0)
        {
            AddRel((char*)(relations[i]->name.c_str()), relations[i]->numTuples * ratio[i]);
            *(est_num_tuples[i]) = *(est_num_tuples[i]) * ratio[i];
        }
    }
    
}

void Statistics::Apply(struct AndList *parseTree, char *relNames[], int numToJoin)
{
    RelationInfo *tmp1, *tmp2;
    tmp1 = tmp2 = NULL;
    RelationInfo **partition1, **partition2;
    partition1 = &tmp1;
    partition2 = &tmp2;

    bool hasMoreThan2Parts = moreThan2Partitions(partition1, partition2, relNames, numToJoin);
    if (hasMoreThan2Parts)
    {
        throw std::runtime_error("[Error] In function Statistics::Apply(): cannot apply to more than 2 partitions");
    }
    RelationInfo *relation1, *relation2;
    relation1 = *partition1;
    relation2 = *partition2;

    if (relation1 == NULL && relation2 == NULL)
    {
        throw std::runtime_error("[Error] In function Statistics::Apply(): invalid relations");
    }
    
    double est_num_tuples_after_join = -1;
    double est_num_tuples1 = -1, est_num_tuples2 = -1;
    if (relation1 != NULL)
    {
        est_num_tuples1 = relation1->numTuples;
    }
    if (relation2 != NULL)
    {
        est_num_tuples2 = relation2->numTuples;
    }


    struct OrList *or_list = NULL;
    struct ComparisonOp *comp_op = NULL;
    while (parseTree != NULL)
    {
        if (parseTree->left != NULL)
        {
            or_list = parseTree->left;
            while (or_list != NULL)
            {
                comp_op = or_list->left;
                if (comp_op != NULL)
                {
                    if (comp_op->left->code == NAME && comp_op->right->code == NAME)
                    {
                        if (relation1 == NULL || relation2 == NULL)
                        {
                            throw std::runtime_error("[Error] In function Statistics::Apply(): no enough relations to be joined");
                        }
                        std::string attname_left(comp_op->left->value);
                        std::string attname_right(comp_op->right->value);
                        relation1 = applyJoin(est_num_tuples_after_join, 
                                                relation1, relation2, 
                                                attname_left, attname_right, 
                                                est_num_tuples1, est_num_tuples2,
                                                relNames, numToJoin);
                        relation2 = NULL;
                        est_num_tuples1 = est_num_tuples_after_join;
                        est_num_tuples2 = -1;

                        or_list = or_list->rightOr;
                    }
                    else
                    {
                        applySelect(relation1, relation2, 
                                    est_num_tuples1, est_num_tuples2,
                                    or_list);
                        or_list = NULL;
                    }
                }
                else
                {
                    or_list = or_list->rightOr;
                }
            }
        }
        parseTree = parseTree->rightAnd;
    }

    if (est_num_tuples1 >= 0)
    {
        this->final_est_num_tuples = est_num_tuples1;
    }
    else
    {
        this->final_est_num_tuples = est_num_tuples2;
    }
    
}

double Statistics::Estimate(struct AndList *parseTree, char *relNames[], int numToJoin)
{
    Statistics tmp(*this);
    tmp.Apply(parseTree, relNames, numToJoin);
    return tmp.final_est_num_tuples;
}

