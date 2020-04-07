#ifndef STATISTICS_
#define STATISTICS_

#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <map>
#include <vector>
#include "ParseTree.h"
#include "Util.h"

class AttributeInfo
{
	public: 
		std::string name;
		int numDistincts;
		
		AttributeInfo();
		AttributeInfo(std::string att_name, int num_distincts); 
		std::string toString();
};

class RelationInfo 
{
	public:
		std::string name;
		int numTuples;
		std::map<std::string, AttributeInfo> data;
		
		RelationInfo();
		RelationInfo(std::string rel_name, int num_tuples);
		std::string toString();
};

class Statistics
{
private:
	
public:

	double final_est_num_tuples;
	std::map<std::string, RelationInfo*> joined_relations;
	std::map<std::string, RelationInfo> data;
	
	Statistics();
	Statistics(Statistics &copyMe);
	~Statistics();

	void AddRel(char *relName, int numTuples);
	void AddAtt(char *relName, char *attName, int numDistincts);
	void CopyRel(char *oldName, char *newName);
	
	std::string joinedRelToStr();
	void Read(char *fromWhere);
	void Write(char *fromWhere);

	bool moreThan2Partitions(RelationInfo **partition1, RelationInfo **partition2, char *relNames[], int numToJoin);
	RelationInfo* applyJoin(double &num_tuples_after_join, RelationInfo *relation1, RelationInfo *relation2, const std::string &att_name_left, const std::string &att_name_right, double est_num_tuples1, double est_num_tuples2, char *relNames[], int numToJoin);
	double estimateSelectRatio(RelationInfo *relation, const std::string &att_name, bool select_by_equality);
	std::vector<double> estimateSelectRatio(RelationInfo *relation1, RelationInfo *relation2, struct OrList *or_list);
	void applySelect(RelationInfo *relation1, RelationInfo *relation2, double &est_num_tuples1, double &est_num_tuples2, struct OrList *or_list);

	void  Apply(struct AndList *parseTree, char *relNames[], int numToJoin);
	double Estimate(struct AndList *parseTree, char **relNames, int numToJoin);

	template <typename T1, typename T2>
	bool exists(std::map<T1, T2> container, const T1 &key)
	{
		typename std::map<T1, T2>::iterator it = container.find(key);
		return (it != container.end());
	};

};

#endif
