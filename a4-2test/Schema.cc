#include "Schema.h"
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>

int Schema :: Find (char *attName) {

	for (int i = 0; i < numAtts; i++) {
		if (!strcmp (attName, myAtts[i].name)) {
			return i;
		}
	}

	// if we made it here, the attribute was not found
	return -1;
}

Type Schema :: FindType (char *attName) {
	// std::cout << attName << std::endl;
	for (int i = 0; i < numAtts; i++) {
		// std::cout<< i << std::endl;
		// std::cout<< myAtts[i].name << std::endl;
		if (!strcmp (attName, myAtts[i].name)) {
			// std::cout << "FOUND ATT Type!" << std::endl;
			// std::cout << myAtts[i].name << "--" << TypeStr[myAtts[i].myType] << std::endl;
			return myAtts[i].myType;
		}
		// std::cout << "NOT Found ATT Type!" << std::endl;
	}

	// if we made it here, the attribute was not found
	// std::cout << "NOT Found ANY ATT Type!" << std::endl;
	return Int;
}

int Schema :: GetNumAtts () {
	return numAtts;
}

Attribute *Schema :: GetAtts () {
	return myAtts;
}


Schema :: Schema (char *fpath, int num_atts, Attribute *atts) {
	fileName = strdup (fpath);
	numAtts = num_atts;
	myAtts = new Attribute[numAtts];
	for (int i = 0; i < numAtts; i++ ) {
		if (atts[i].myType == Int) {
			myAtts[i].myType = Int;
		}
		else if (atts[i].myType == Double) {
			myAtts[i].myType = Double;
		}
		else if (atts[i].myType == String) {
			myAtts[i].myType = String;
		} 
		else {
			cout << "Bad attribute type for " << atts[i].myType << "\n";
			delete [] myAtts;
			exit (1);
		}
		myAtts[i].name = strdup (atts[i].name);
	}
}

Schema :: Schema (char *fName, char *relName) {

	FILE *foo = fopen (fName, "r");
	
	// this is enough space to hold any tokens
	char space[200];

	fscanf (foo, "%s", space);
	int totscans = 1;

	// see if the file starts with the correct keyword
	if (strcmp (space, "BEGIN")) {
		cout << "Unfortunately, this does not seem to be a schema file.\n";
		exit (1);
	}	
		
	while (1) {

		// check to see if this is the one we want
		fscanf (foo, "%s", space);
		totscans++;
		if (strcmp (space, relName)) {

			// it is not, so suck up everything to past the BEGIN
			while (1) {

				// suck up another token
				if (fscanf (foo, "%s", space) == EOF) {
					cerr << "Could not find the schema for the specified relation.\n";
					exit (1);
				}

				totscans++;
				if (!strcmp (space, "BEGIN")) {
					break;
				}
			}

		// otherwise, got the correct file!!
		} else {
			break;
		}
	}

	// suck in the file name
	fscanf (foo, "%s", space);
	totscans++;
	fileName = strdup (space);

	// count the number of attributes specified
	numAtts = 0;
	while (1) {
		fscanf (foo, "%s", space);
		if (!strcmp (space, "END")) {
			break;		
		} else {
			fscanf (foo, "%s", space);
			numAtts++;
		}
	}

	// now actually load up the schema
	fclose (foo);
	foo = fopen (fName, "r");

	// go past any un-needed info
	for (int i = 0; i < totscans; i++) {
		fscanf (foo, "%s", space);
	}

	// and load up the schema
	myAtts = new Attribute[numAtts];
	for (int i = 0; i < numAtts; i++ ) {

		// read in the attribute name
		fscanf (foo, "%s", space);	
		myAtts[i].name = strdup (space);

		// read in the attribute type
		fscanf (foo, "%s", space);
		if (!strcmp (space, "Int")) {
			myAtts[i].myType = Int;
		} else if (!strcmp (space, "Double")) {
			myAtts[i].myType = Double;
		} else if (!strcmp (space, "String")) {
			myAtts[i].myType = String;
		} else {
			cout << "Bad attribute type for " << myAtts[i].name << "\n";
			exit (1);
		}
	}

	fclose (foo);
}


// deepcopy constructor
Schema::Schema (const Schema& sch) {
	Attribute *atts = new Attribute[sch.numAtts];
	for (int i = 0; i < sch.numAtts; i++) {
		atts[i].name = (char*) malloc (MAX_LEN_ATTNAME * sizeof(char));
		strcpy(atts[i].name, sch.myAtts[i].name);
		//std::cout << atts[i].name << " " << atts[i].name << std::endl;
		atts[i].myType = sch.myAtts[i].myType;
	}
	this->numAtts = sch.numAtts;
	this->myAtts = atts;
	this->fileName = (char*)(std::string(sch.fileName).c_str());
}


Schema :: ~Schema () {
	delete [] myAtts;
	myAtts = 0;
}

std::string Schema::toString() {
	std::string output = "";
	for (int i = 0; i < numAtts; i++) {
		std::string prefix = Util::getPrefix(std::string(myAtts[i].name), '_');
		output += "        Att " + prefix + "." + std::string(myAtts[i].name) + ": " + TypeStr[myAtts[i].myType] + "\n";
	}
	return output;
}