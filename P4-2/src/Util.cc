#include "Util.h"

std::string Util::randomStr(const int len) {
        static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

        std::string s;
        for (int i = 0; i < len; ++i) {
            s += alphanum[rand() % (sizeof(alphanum) - 1)];
        }
        return s;
}

bool Util::isEmptyFile(std::ifstream &in_file)
{
    return (in_file.peek() == std::ifstream::traits_type::eof());
}

std::string Util::ParseTreeToString(Operand *operand) {
    return std::string(operand->value);
}

std::string Util::ParseTreeToString(ComparisonOp *comp_op) {
    std::string sign = "";
    switch (comp_op->code) {
        case LESS_THAN:
            sign = " < ";
            break;
        case GREATER_THAN:
            sign = " > ";
            break;
        case EQUALS:
            sign = " = ";
            break;
    }
    return ParseTreeToString(comp_op->left) + sign + ParseTreeToString(comp_op->right);
}

std::string Util::ParseTreeToString(OrList *or_list) {
    ComparisonOp *clause = or_list->left;
    OrList *others = or_list->rightOr;
    std::string output = "( ";
    while (clause != NULL) {
        output += ParseTreeToString(clause); 
        if (others != NULL) {
            clause = others->left;
            others = others->rightOr;
            output += " OR ";
        }
        else {
            output += " )";
            break;
        }
    }
    return output;
}

std::string Util::ParseTreeToString(AndList *and_list) {
    OrList *or_list = and_list->left;
    AndList *others = and_list->rightAnd;
    std::string output = "";
    while (or_list != NULL) {
        output += ParseTreeToString(or_list); 
        if (others != NULL) {
            or_list = others->left;
            others = others->rightAnd;
            output += " AND ";
        }
        else {
            break;
        }
    }
    return output;
}

std::string Util::ParseTreeToString(std::vector<AndList*> and_lists) {
    std::string output = "";
    for (int i = 0; i < and_lists.size(); i++) {
        output += ParseTreeToString(and_lists[i]);
        if (i < and_lists.size() - 1) {
            output += " AND ";
        } 
    }
    return output;
}
    

std::vector<AndList*> Util::splitAndList(AndList *and_list) {
    std::vector<AndList*> res;
    if (and_list == NULL) {
        return res;
    }
    OrList *cur = and_list->left;
    AndList *others = and_list->rightAnd;
    while (cur != NULL) {
        AndList *single = new AndList();
        single->left = cur;
        single->rightAnd = NULL;
        res.push_back(single);
        if (others != NULL) {
            cur = others->left;
            others = others->rightAnd;
        }
        else {
            break;
        }
    }
    return res;
}

AndList* Util::getParseTreeFromVector(std::vector<AndList*> vec) {
    AndList* dummy = new AndList();
    AndList* node = dummy;
    for (int i = 0; i < vec.size(); i++) {
        node->rightAnd = vec[i];
        node = node->rightAnd;
    }
    return dummy->rightAnd;
}

void Util::removeRelNameFromPred(ComparisonOp* predicate) {
    std::string left_name(predicate->left->value);
    std::string right_name(predicate->right->value);
    left_name = getSuffix(left_name, '.');
    right_name = getSuffix(right_name, '.');
    strcpy(predicate->left->value, left_name.c_str());
    strcpy(predicate->right->value, right_name.c_str());
}

void Util::removeRelNameFromPred(OrList* predicate) {
    ComparisonOp *cur = predicate->left;
    OrList *others = predicate->rightOr;
    while (cur != NULL) {
        removeRelNameFromPred(cur);
        if (others != NULL) {
            cur = others->left;
            others = others->rightOr;
        }
        else {
            break;
        }
    }
}

void Util::removeRelNameFromPred(AndList* predicate) {
    OrList *cur = predicate->left;
    AndList *others = predicate->rightAnd;
    while (cur != NULL) {
        removeRelNameFromPred(cur);
        if (others != NULL) {
            cur = others->left;
            others = others->rightAnd;
        }
        else {
            break;
        }
    }
}

void Util::removeRelNameFromPred(NameList* atts) {
    while (atts != NULL) {
        std::string name_str(atts->name);
        name_str = getSuffix(name_str, '.');
        strcpy(atts->name, name_str.c_str());
        atts = atts->next;
    }
}
    
void Util::removeRelNameFromPred(FuncOperator* func) {
    //int trav_dirt = 0;
    //FuncOperator *origin = func;
    //while (trav_dirt < 2) {
        if (func == NULL) {
            return;
        }
        
        if (func->leftOperand != NULL && func->leftOperand->code == NAME) {
            std::string att_name(func->leftOperand->value);
            att_name = getSuffix(att_name, '.');
            strcpy(func->leftOperand->value, att_name.c_str());
        }
            //if (trav_dirt == 0) { // move to left
        removeRelNameFromPred(func->leftOperator);
        removeRelNameFromPred(func->right);
            //}
    //}
}
    
std::string Util::getPrefix(std::string input, char separator) {
    std::stringstream ss(input);
    std::string prefix = "";
    if (ss.good()) {
        getline(ss, prefix, separator);
    }
    return prefix;
}

std::string Util::getSuffix(std::string input, char separator) {
    std::stringstream ss(input);
    std::string suffix = "";
    while (ss.good()) {
        getline(ss, suffix, separator);
    }
    return suffix;
}

std::vector<std::string> Util::splitString(std::string input, char separator) {
    std::vector<std::string> res;
    std::stringstream ss(input);
    std::string substr = "";
    while (ss.good()) {
        getline(ss, substr, separator);
        res.push_back(substr);
    }
    return res;    
}