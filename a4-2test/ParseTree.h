#ifndef PARSE_TREE_H
#define PARSE_TREE_H
// these are the different types of operators that can appear
// in a CNF expression
#define LESS_THAN 1
#define GREATER_THAN 2
#define EQUALS 3

// these are the types of operands that can appear in a CNF expression
#define DOUBLE 1
#define INT 2
#define STRING 3
#define NAME 4


struct Operand {

	// this tells us the type of the operand: FLOAT, INT, STRING...
	// When the operand is an attribute name, this code is NAME (4).
	int code;

	// this is the actual operand
	// When the operand is an attribute name, this value would be string of the name
	// For example, if we have a ComparisonOp: "ps_suppkey > 10", here value of left operand is string "ps_suppkey", value of right operand is "10"
	// if we have a ComparisonOp: "s_suppkey = ps_suppkey", then value of left operand is "s_suppkey" while value of right operand is "ps_suppkey".
	char *value;

	// name of the relation where this operand belongs to
	// if code == NAME, relName is not NULL
	// otherwise, relName is NULL
	//char *relName = NULL;

	//char* toString();
};

struct ComparisonOp {

	// this corresponds to one of the codes describing what type
	// of literal value we have in this nodes: LESS_THAN, EQUALS...
	int code;

	// these are the operands on the left and on the right
	struct Operand *left;
	struct Operand *right;	

	//char* toString();
};

struct OrList {

	// this is the comparison to the left of the OR
	struct ComparisonOp *left;

	// this is the OrList to the right of the OR; again,
	// this might be NULL if the right is a simple comparison
	struct OrList *rightOr;

	//char* toString();
};

struct AndList {

	// this is the disjunction to the left of the AND
	struct OrList *left;

	// this is the AndList to the right of the AND
	// note that this can be NULL if the right is a disjunction
	struct AndList *rightAnd;

	//char* toString();
};

struct FuncOperand {

	// this tells us the type of the operand: FLOAT, INT, STRING...
	int code;

	// this is the actual operand
	char *value;
};

struct FuncOperator {

	// this tells us which operator to use: '+', '-', ...
	int code;

	// these are the operators on the left and on the right
	struct FuncOperator *leftOperator;
	struct FuncOperand *leftOperand;
	struct FuncOperator *right;	

};

struct TableList {

	// this is the original table name
	char *tableName;

	// this is the value it is aliased to
	char *aliasAs;

	// and this the next alias
	struct TableList *next;
};

struct NameList {

	// this is the name
	char *name;

	// and this is the next name in the list
	struct NameList *next;
};


#endif

