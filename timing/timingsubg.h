#ifndef _TIMINGSUBG_H_
#define _TIMINGSUBG_H_

#include "edge.h"
#include "../util/util.h"
#include "../msforest/nodeOP.h"
class timingconf;
class msforest;
class query;
class teNode;
class gstream;
class gtransaction;


struct MatchedPair {
	MatchedPair(dEdge *_e, qEdge *_q): data_event(_e), pattern_event(_q) {}
	dEdge *data_event;
	qEdge *pattern_event;
};

class timingsubg{
public:
	timingsubg(int64_t _winsz, string _config="defaultconfg");
	timingsubg();
	~timingsubg();
	void initial();

	void run(int _mode, gstream* _G, query* Q, timingconf* _tconf);
	bool new_edge(MatchedPair _matched_pair);
	// bool new_edge(dEdge* _e);
	bool expire_edge(MatchedPair _matched_pair);
	bool check_expire_edge(dEdge* _newest);
	bool tuple_expire(dEdge* _e);
	timingconf* getconf();

	string exename;
	string stat_str();
	bool write_stat();

	static void* thread_insert(void* _tran);
	static void* thread_remove(void* _tran);

	vector<qEdge *> pattern_events;
private:

	/* 
	 * path_dataset
	 * path_vertex2id
	 * path_vlabel2id
	 * path_elabel2id
	 * vNum
	 * eNum
	 * Max_Thread_Num 
	 * Max_Query_Size
	 * 
	 */

	gtransaction* next_tran();

	int64_t win_size;
	queue<pair<MatchedPair, bool> > cur_edges;
	// queue<pair<dEdge*, bool> > cur_edges;
	queue<gtransaction*> tran_pool;
	timingconf* tconf;
	msforest* M;
	gstream* G;
	query* Q;



    string run_report();
    void remaining_threadsjoin();
	int seen_eNum;
	int unmat_eNum;
	int del_eNum;
	int gap_log;
	vector<OPlist*> cacheOPlists;
	vector<qEdge*> cacheMatEdge;
	vector<teNode*> cacheTe;
	string fruntime;

	double SUMtime;
	double AVGtime;
	double AVGspace;
	double AVGnoMS;
	long long int TALspace;
	long long int TALnoMS;
	long long int Throughput;
	long long int RUNtimes;

};


#endif 
