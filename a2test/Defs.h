#ifndef DEFS_H
#define DEFS_H


#define MAX_ANDS 20
#define MAX_ORS 20

#define PAGE_SIZE 131072

enum Unit {
    Byte = 1,
    KB = 1000 * Byte,
    MB = 1000 * KB,
    GB = 1000 * MB
};

const long long MaxMemorySize = 1 * GB;

enum Target {Left, Right, Literal};
enum CompOperator {LessThan, GreaterThan, Equals};
enum Type {Int, Double, String};


unsigned int Random_Generate();


#endif

