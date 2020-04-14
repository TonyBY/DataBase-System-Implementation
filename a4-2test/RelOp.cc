#include "RelOp.h"

void RunFunc::SelectFile (void* raw_args) {
	struct args_SelectFile* args = (struct args_SelectFile*) raw_args;
	Record rec;
	int cnt = 0;
	while (args->inFile->GetNext(rec, *(args->selOp), *(args->literal))) {
		args->outPipe->Insert(&rec);
		cnt ++;
	}
	std::cout << "The number of records from file: " << cnt << std::endl;
	args->outPipe->ShutDown();
}

void SelectFile::Run (DBFile &inFile, Pipe &outPipe, CNF &selOp, Record &literal) {
	struct RunFunc::args_SelectFile* args = (struct RunFunc::args_SelectFile*)malloc(sizeof(struct RunFunc::args_SelectFile));
	args->inFile = &inFile;
	args->outPipe = &outPipe;
	args->selOp = &selOp;
	args->literal = &literal;
	//pthread_t worker;
	pthread_create(&worker, NULL, (THREADFUNCPTR) RunFunc::SelectFile, (void*)args); 
}

//void SelectFile::WaitUntilDone () {
//	std::cout << &worker << std::endl;
//	pthread_join (worker, NULL);
//	//std::cout << "SelectFile Done" << std::endl;
//}

//void SelectFile::Use_n_Pages (int runlen) {
//	return;
//}

void RunFunc::SelectPipe (void* raw_args) {
	struct args_SelectPipe* args = (struct args_SelectPipe*) raw_args;
	Record rec;
	ComparisonEngine comp;
	while (args->inPipe->Remove(&rec)) {
		if (comp.Compare(&rec, args->literal, args->selOp) == 1) { // matched
			args->outPipe->Insert(&rec);
		}
	}
	args->outPipe->ShutDown();
}

void SelectPipe::Run (Pipe &inPipe, Pipe &outPipe, CNF &selOp, Record &literal) {
	struct RunFunc::args_SelectPipe* args = (struct RunFunc::args_SelectPipe*)malloc(sizeof(struct RunFunc::args_SelectPipe));
	args->inPipe = &inPipe;
	args->outPipe = &outPipe;
	args->selOp = &selOp;
	args->literal = &literal;
	//pthread_t worker;
	pthread_create(&worker, NULL, (THREADFUNCPTR) RunFunc::SelectPipe, (void*)args); 
}

//void SelectPipe::WaitUntilDone () { 
//	//std::cout << &worker << std::endl;
//	pthread_join (worker, NULL);
//}

//void SelectPipe::Use_n_Pages (int n) { return; }

void RunFunc::Project (void* raw_args) {
	struct args_Project* args = (struct args_Project*) raw_args;
	Record rec;
	while (args->inPipe->Remove(&rec)) {
		rec.Project(args->keepMe, args->numAttsOutput, args->numAttsInput);
		args->outPipe->Insert(&rec);
	}
	args->outPipe->ShutDown();
}

void Project::Run (Pipe &inPipe, Pipe &outPipe, int *keepMe, int numAttsInput, int numAttsOutput) {
	struct RunFunc::args_Project* args = (struct RunFunc::args_Project*)malloc(sizeof(struct RunFunc::args_Project));
	args->inPipe = &inPipe;
	args->outPipe = &outPipe;
	args->keepMe = keepMe;
	args->numAttsInput = numAttsInput;
	args->numAttsOutput = numAttsOutput;
	pthread_create(&worker, NULL, (THREADFUNCPTR) RunFunc::Project, (void*)args); 
}

//void Project::WaitUntilDone () { 
//	std::cout << &worker << std::endl;
//	pthread_join (worker, NULL);
//}

//void Project::Use_n_Pages (int n) { return; }

void RunFunc::Join (void* raw_args) {
	std::cout << "kk" << std::endl;
	struct args_Join* args = (struct args_Join*) raw_args;
	int runLength = 200;
	int buffersz = 100;
	Record recL, recR, recMerge;
	Pipe bigq_outL(buffersz); Pipe bigq_outR(buffersz);
	Pipe *join_inL, *join_inR;
	OrderMaker orderL, orderR;

	Attribute sAtt[7] = {
							{"s_suppkey", Int}, 
							{"s_name", String},
					 		{"s_address", String},
							{"s_nationkey", Int},
							{"s_phone", String},
							{"s_acctbal", Double},
							{"s_comment", String}
	};
	
	Attribute psAtt[5] = {
							{"ps_partkey", Int}, 
							{"ps_suppkey", Int},
					 		{"ps_availqty", Int},
							{"ps_supplycost", Double},
							{"ps_comment", String}
	};
	
	Schema s_sch ("supplier_sch", 7, sAtt);
	Schema ps_sch ("ps_sch", 5, psAtt);

	if (!args->selOp->GetSortOrders(orderL, orderR)) {
		// Generating OrderMakers failed
		std::cout << 6666 << std::endl;
		join_inL = args->inPipeL;
		join_inR = args->inPipeR;
	}
	else {
		std::cout << 111111 << std::endl;
		
		//orderL.Print();
		//orderR.Print();
		
		//int y;
		//std::cin >> y;

		BigQ bqL(*(args->inPipeL), bigq_outL, orderL, runLength);
		/*
		for (int i = 0; i< 5; i++) {
			Record tmpL, tmpR;
			bigq_outL.Remove(&tmpL);
			//bigq_outR.Remove(&tmpR);
			
			tmpL.Print(&s_sch);
			//tmpR.Print(&ps_sch);

			std::cout << std::endl;
			int x;
			std::cin >> x;
		}
		*/
	
		// If the two BigQ start too close, they will affect each other, then the sorting results will be wrong.
		// We do not know why. So just make the second BigQ start 2 seconds later.
		sleep(2); 
	
		BigQ bqR(*(args->inPipeR), bigq_outR, orderR, runLength);

		//sleep(2);
		/*
		for (int i = 0; i< 5; i++) {
			Record tmpL, tmpR;
			bigq_outL.Remove(&tmpL);
			bigq_outR.Remove(&tmpR);
			
			tmpL.Print(&s_sch);
			tmpR.Print(&ps_sch);

			std::cout << std::endl;
			int x;
			std::cin >> x;
		}
		*/

		std::cout << 555555 << std::endl;
		//join_inL = &bigq_outL;
		//join_inR = &bigq_outR;
		MergeJoin(args, &bigq_outL, &bigq_outR, &orderL, &orderR);
	}
	std::cout << "llk" << std::endl;
	/*
	int getLeft = join_inL->Remove(&recL);
	int getRight = join_inR->Remove(&recR);
	int cnt = 1;
	while ( getLeft || getRight ) {
		//std::cout << 'here' << std::endl;
		if (!getLeft) {
			args->numAttsLeft = 0;
		}
		else if (!getRight){
			args->numAttsRight = 0;
		}
		
		std::cout << "here" << std::endl;
		recMerge.MergeRecords(&recL, &recR, args->numAttsLeft, args->numAttsRight, args->attsToKeep, args->numAttsToKeep, args->startOfRight);
		std::cout << "there" << std::endl;
		//std::cout << 'there' << std::endl;
		args->outPipe->Insert(&recMerge);
		
		std::cout << "merge No." << cnt << "record" << std::endl;
		cnt++;

		

		getLeft = join_inL->Remove(&recL);
		getRight = join_inR->Remove(&recR);
		
	}
	*/
	args->outPipe->ShutDown();
	
}

void RunFunc::MergeJoin(struct args_Join* args, Pipe* bigq_outL, Pipe* bigq_outR, OrderMaker* orderL, OrderMaker* orderR) {
	std::cout << "MergeJoin" << std::endl;
	int runLength = 200;
	int buffsz = 65535;
	Record recL, recR, recMerge, *prevL = NULL, *prevR = NULL;
	Pipe* join_inL = bigq_outL;
	Pipe* join_inR = bigq_outR;

	
	int cnt = 1;
	/*
	Attribute sAtt[7] = {
							{"s_suppkey", Int}, 
							{"s_name", String},
					 		{"s_address", String},
							{"s_nationkey", Int},
							{"s_phone", String},
							{"s_acctbal", Double},
							{"s_comment", String}
	};
	
	Attribute psAtt[5] = {
							{"ps_partkey", Int}, 
							{"ps_suppkey", Int},
					 		{"ps_availqty", Int},
							{"ps_supplycost", Double},
							{"ps_comment", String}
	};
	
	Schema s_sch ("supplier_sch", 7, sAtt);
	Schema ps_sch ("ps_sch", 5, psAtt);
	*/
	/*
	for (int i = 0; i< 5; i++) {
		Record tmpL, tmpR;
		join_inL->Remove(&tmpL);
		join_inR->Remove(&tmpR);
		tmpL.Print(&s_sch);
		tmpR.Print(&ps_sch);
		std::cout << std::endl;
		int x;
		std::cin >> x;
	}
	*/

	int getLeft = join_inL->Remove(&recL);
	int getRight = join_inR->Remove(&recR);
	ComparisonEngine comp;
	

	while (getLeft && getRight) {
		int left_vs_right = comp.Compare(&recL, orderL, &recR, orderR);
    	if (left_vs_right < 0) {
			getLeft = join_inL->Remove(&recL);
		}
		else if (left_vs_right > 0) {
			getRight = join_inR->Remove(&recR);
		}
		else {
			//recL.Print(&s_sch);
			//recR.Print(&ps_sch);
			//std::cout << std::endl;

			vector<Record*> buff;
			while (getLeft && (prevL == NULL || comp.Compare(prevL, &recL, orderL) == 0)) {
				//if (prevL == NULL) {
				prevL = new Record();
					//prev->Consume(&recL);
				//}
				prevL->Consume(&recL);
				buff.push_back(prevL);
				getLeft = join_inL->Remove(&recL);
			}
			while (getRight && comp.Compare(prevL, orderL, &recR, orderR) == 0) {
				Record copyR;
				copyR.Copy(&recR);
				int tmp_cnt = 1;
				for (int i = 0; i < buff.size(); i++) {
					recMerge.MergeRecords(buff[i], &recR, args->numAttsLeft, args->numAttsRight, args->attsToKeep, args->numAttsToKeep, args->startOfRight);
					recR.Copy(&copyR);
					args->outPipe->Insert(&recMerge);
					//std::cout << "merge No." << cnt << "record, left " << buff.size() << " records, right No." << tmp_cnt << " records"<< std::endl;
					//recR.Print(&ps_sch);
					cnt++;
					tmp_cnt++;
				}
				getRight = join_inR->Remove(&recR);
			}

			for (int i = 0; i < buff.size(); i++) { // delete all pointers in buff
				delete buff[i];
				buff[i] = NULL;
			}
			prevL = NULL;
		}
	}
	/*
	while ( getLeft || getRight ) {
		//std::cout << 'here' << std::endl;
		if (!getLeft) {
			args->numAttsLeft = 0;
		}
		else if (!getRight){
			args->numAttsRight = 0;
		}
		
		//std::cout << "here" << std::endl;
		recMerge.MergeRecords(&recL, &recR, args->numAttsLeft, args->numAttsRight, args->attsToKeep, args->numAttsToKeep, args->startOfRight);
		//std::cout << "there" << std::endl;
		//std::cout << 'there' << std::endl;
		args->outPipe->Insert(&recMerge);
		
		
		

		getLeft = join_inL->Remove(&recL);
		getRight = join_inR->Remove(&recR);
		
	}
	*/
	//args->outPipe->ShutDown();
}

void Join::Run (Pipe &inPipeL, Pipe &inPipeR, Pipe &outPipe, CNF &selOp, Record &literal, 
			int numAttsLeft,
			int numAttsRight,
			int *attsToKeep,
			int numAttsToKeep,
			int startOfRight) {
	struct RunFunc::args_Join* args = (struct RunFunc::args_Join*)malloc(sizeof(struct RunFunc::args_Join));
	args->inPipeL = &inPipeL;
	args->inPipeR = &inPipeR;
	args->outPipe = &outPipe;
	args->selOp = &selOp;
	args->literal = &literal;
	args->attsToKeep = attsToKeep;
	args->numAttsToKeep = numAttsToKeep;
	args->numAttsLeft = numAttsLeft;
	args->numAttsRight = numAttsRight;
	args->startOfRight = startOfRight;
	pthread_create(&worker, NULL, (THREADFUNCPTR) RunFunc::Join, (void*)args); 
}

void RunFunc::DuplicateRemoval(void* raw_args) {
	struct args_DupRemoval* args = (struct args_DupRemoval*) raw_args;
	OrderMaker sort_order(args->mySchema);
	int pipesz = 100, runLength = 100;
	Pipe bigq_out(pipesz);
	BigQ bigq(*(args->inPipe), bigq_out, sort_order, runLength);
	Record cur, *prev = NULL;
	ComparisonEngine comp;
	while (bigq_out.Remove(&cur)) {
		if (prev == NULL || comp.Compare(&cur, prev, &sort_order) != 0) {
			if (prev == NULL) {
				prev = new Record();
			}
			prev->Copy(&cur);
			args->outPipe->Insert(&cur);
		}
	}
	if (prev != NULL) {
		delete prev;
		prev = NULL;
	}

	args->outPipe->ShutDown();
}

void DuplicateRemoval::Run(Pipe &inPipe, Pipe &outPipe, Schema &mySchema) {
	struct RunFunc::args_DupRemoval* args = (struct RunFunc::args_DupRemoval*)malloc(sizeof(struct RunFunc::args_DupRemoval));
	args->inPipe = &inPipe;
	args->outPipe = &outPipe;
	args->mySchema = &mySchema;
	pthread_create(&worker, NULL, (THREADFUNCPTR) RunFunc::DuplicateRemoval, (void*)args); 
}

void RunFunc::Sum (void* raw_args) {
	struct args_Sum* args = (struct args_Sum*) raw_args;
	Record rec;
	int int_res = 0, int_sum = 0;
	double double_res = 0, double_sum = 0;
	Type res_type;
	int cnt = 0;
	while (args->inPipe->Remove(&rec)) {
		res_type = args->func->Apply(rec, int_res, double_res);
		switch (res_type) {
			case Int:
				int_sum += int_res;
				int_res = 0;
				break;
			case Double:
				double_sum += double_res;
				double_res = 0;
				break;
			default:
				std:cerr << "[Error] In RunFunc::Sum(): invalid sum result type" << std::endl;
		}
		cnt++;
	}
	
	std::cout << "Calculate sum on " << cnt << " records" << std::endl;
	
	Record sum;
	Attribute sum_att = {"sum", res_type};
	Schema sum_sch ("sum_sch", 1, &sum_att);
	const char* sum_str;
	
	if (res_type == Int) {
		sum_str = (Util::toString<int>(int_sum) + "|").c_str();
	}
	else if (res_type == Double) {
		sum_str = (Util::toString<double>(double_sum) + "|").c_str();
	}
	
	sum.ComposeRecord(&sum_sch, sum_str);

	args->outPipe->Insert(&sum);
	args->outPipe->ShutDown();
}

void Sum::Run (Pipe &inPipe, Pipe &outPipe, Function &computeMe) { 
	struct RunFunc::args_Sum* args = (struct RunFunc::args_Sum*)malloc(sizeof(struct RunFunc::args_Sum));
	args->inPipe = &inPipe;
	args->outPipe = &outPipe;
	args->func = &computeMe;
	pthread_create(&worker, NULL, (THREADFUNCPTR) RunFunc::Sum, (void*)args); 
}
	
void RunFunc::GroupBy(void* raw_args) {

Attribute jAtt[12] = {
							{"s_suppkey", Int}, 
							{"s_name", String},
					 		{"s_address", String},
							{"s_nationkey", Int},
							{"s_phone", String},
							{"s_acctbal", Double},
							{"s_comment", String},

							{"ps_partkey", Int}, 
							{"ps_suppkey", Int},
					 		{"ps_availqty", Int},
							{"ps_supplycost", Double},
							{"ps_comment", String}
	};
	
	//Schema s_sch ("supplier_sch", 7, sAtt);
	//Schema ps_sch ("ps_sch", 5, psAtt);
	Schema j_sch ("j", 12, jAtt);

	struct args_GroupBy* args = (struct args_GroupBy*) raw_args;

	args->groupAtts->Print();

	Record cur, *prev = NULL;
	int int_res = 0, int_sum = 0;
	double double_res = 0, double_sum = 0;
	Type res_type;
	int cnt = 0;

	int runLength = 100;
	int buffsz = 200;
	
	Pipe* bigq_out = new Pipe(buffsz);
	//Pipe bigq_out(buffsz);
	
	sleep(10);
	BigQ bigq(*(args->inPipe), *bigq_out, *(args->groupAtts), runLength);
	//sleep(2);

	ComparisonEngine comp;

	bool remainingRec = false;
	int haveReadCur = 0;

	while (remainingRec || (haveReadCur = bigq_out->Remove(&cur)) || prev != NULL) {
		//cur.Print(&j_sch);
		//int x;
		//std::cin >> x;
		if ((remainingRec || haveReadCur) && (prev == NULL || comp.Compare(prev, &cur, args->groupAtts) == 0)) {
			// In the same group
			if (prev == NULL) {
				prev = new Record();
				prev->Copy(&cur);
				remainingRec = false;
			}
			res_type = args->func->Apply(cur, int_res, double_res);
			switch (res_type) {
				case Int:
					int_sum += int_res;
					int_res = 0;
					break;
				case Double:
					double_sum += double_res;
					double_res = 0;
					break;
				default:
					std:cerr << "[Error] In RunFunc::Sum(): invalid sum result type" << std::endl;
			}	
		}
		else {
			// A new group starts
			//std::cout << "Calculate sum on " << cnt << " records" << std::endl;
			cnt = 0;
			Record group_sum;
			
			Attribute sum_att = {"sum", res_type};
			Attribute sum_atts[1] = {sum_att};
			Attribute group_atts[MAX_NUM_ATTS];
			
			//Attribute* out_atts = NULL;
			Attribute out_atts[MAX_NUM_ATTS + 1];
			int group_att_indices[MAX_NUM_ATTS + 1];
			int numAtts = args->groupAtts->GetAtts(group_atts, group_att_indices);	
			int numOutAtts = Util::catArrays<Attribute>(out_atts, sum_atts, group_atts, 1, numAtts);
			
			Schema group_sch ("group_sch", numOutAtts, out_atts);
			const char* group_str;
			std::string tmp;
			if (res_type == Int) {
				tmp = Util::toString<int>(int_sum) + "|";
			}
			else {
				tmp = Util::toString<double>(double_sum) + "|";
			}
			for (int i = 0; i < numAtts; i++) {
				switch (group_atts[i].myType) {
					case Int:{
						int att_val_int = *(prev->GetAttVal<int>(group_att_indices[i]));
						tmp += Util::toString<int>(att_val_int) + "|";
						break;
					}
					case Double:{
						double att_val_double = *(prev->GetAttVal<double>(group_att_indices[i]));
						tmp += Util::toString<double>(att_val_double) + "|";
						break;
					}
					case String:{
						std::string att_val_str(prev->GetAttVal<char>(group_att_indices[i]));
						tmp += att_val_str + "|";
						break;
					}
				}
			}
			group_str = tmp.c_str();
			group_sum.ComposeRecord(&group_sch, group_str);
			
			//group_sum.Print(&group_sch);
			sleep(1);

			args->outPipe->Insert(&group_sum);
			
			delete prev;
			prev = NULL;
			if (haveReadCur){
				remainingRec = true;
			}
		}
		cnt++;
	}
	args->outPipe->ShutDown();
}

void GroupBy::Run (Pipe &inPipe, Pipe &outPipe, OrderMaker &groupAtts, Function &computeMe) {
	struct RunFunc::args_GroupBy* args = (struct RunFunc::args_GroupBy*)malloc(sizeof(struct RunFunc::args_GroupBy));
	args->inPipe = &inPipe;
	args->outPipe = &outPipe;
	args->groupAtts = &groupAtts;
	args->func = &computeMe;
	pthread_create(&worker, NULL, (THREADFUNCPTR) RunFunc::GroupBy, (void*)args); 	
}

void RunFunc::WriteOut(void* raw_args) {
	struct args_WriteOut* args = (struct args_WriteOut*) raw_args;
	Record rec;
	while (args->inPipe->Remove(&rec)) {
		WriteRecord(&rec, args->outFile, args->mySchema);
	}
}

void RunFunc::WriteRecord(Record* rec, FILE* outFile, Schema* mySchema) {
	int numAtts = mySchema->GetNumAtts();
	Attribute* atts = mySchema->GetAtts();
	for (int i = 0; i < numAtts; i++) {
		int attLoc = ((int*)(rec->bits))[i+1];
		switch (atts[i].myType) {
			case Int:
				fprintf( outFile, "%d", *((int*)(rec->bits + attLoc)) );
				break;
			case Double:
				fprintf( outFile, "%lf", *((double*)(rec->bits + attLoc)) );
				break;
			case String:
				fprintf( outFile, "%s", *((char*)(rec->bits + attLoc)) );
				break;			
		}
		fprintf( outFile, "|" );
	}
	fprintf( outFile, "\n" );
}

void WriteOut::Run (Pipe &inPipe, FILE *outFile, Schema &mySchema) { 
	struct RunFunc::args_WriteOut* args = (struct RunFunc::args_WriteOut*)malloc(sizeof(struct RunFunc::args_WriteOut));
	args->inPipe = &inPipe;
	args->outFile = outFile;
	args->mySchema = &mySchema;
	pthread_create(&worker, NULL, (THREADFUNCPTR) RunFunc::WriteOut, (void*)args); 
}
