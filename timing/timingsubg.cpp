#include "timingconf.h"
#include "../util/runtime.h"
#include "../util/util.h"
#include "gtransaction.h"
#include "../msforest/msforest.h"
#include "../gstream/gstream.h"
#include "timingsubg.h"
#include "../msforest/nodeOP.h"
#include "../msforest/teNode.h"
#include <cassert>

timingsubg::timingsubg(int64_t _winsz, string _runtime)
{
	this->fruntime = _runtime;
	this->AVGnoMS = 0;
	this->pattern_events = vector<qEdge *>();
}

timingsubg::timingsubg() {}

timingsubg::~timingsubg() {}

void timingsubg::initial()
{
}

void timingsubg::run(int _mode, gstream *_G, query *_Q, timingconf *_tconf)
{
	util::t2count.clear();
	util::reg_track(-10000);

#ifdef DEBUG_TRACK
	util::track("\nIn run@timingsubg");
	util::track("query:= " + _Q->to_str());
#endif

#ifdef GLOBAL_COMMENT
	cout << "In run@timingsubg";
	cout << ", mode=" << _mode << endl;
#endif
	/* initial */
	this->win_size = _tconf->getwinsz();
#ifdef GLOBAL_COMMENT
	cout << "win_size = " << this->win_size << endl;
#endif
	this->G = _G;
	// this->G->load_edges(this->win_size);
	this->G->initialize(this->win_size);
	this->Q = _Q;
	this->tconf = _tconf;

	this->M = new msforest(this->tconf, this->Q);

#ifdef DEBUG_TRACK
	util::track("\nnew msforest");
#endif

	int threadNum = this->tconf->getmaxthreadNum();

#ifdef DEBUG_TRACK
	{
		stringstream _ss;
		_ss << "\nmax_thread_num = " << threadNum << endl;
		util::track(_ss);
	}
#endif

	if (!this->tran_pool.empty())
	{
		cout << "err tpool is not empty" << endl;
		exit(-1);
	}

	// for (int i = 0; i < threadNum; i++)
	// {
	// 	gtransaction *_tran = new gtransaction();
	// 	this->tran_pool.push(_tran);
	// }

#ifdef DEBUG_TRACK
	util::track("\nreset gstream");
#endif
	// reset iterator that points to cur_edge (or so)
	this->G->reset();
	this->cacheOPlists.clear();
	this->cacheMatEdge.clear();

#ifdef DEBUG_TRACK
	util::track("stream runs...");
#endif

#ifdef GLOBAL_COMMENT
	cout << "stream runs..." << endl;
#endif
	this->seen_eNum = 0;
	this->unmat_eNum = 0;
	this->del_eNum = 0;
	this->gap_log = this->G->size() / 1000;
	if (this->gap_log < 10000)
		this->gap_log = 1;

	/* stream runs */
	// runtime _rtime;
	// runtime _whole;
	// _rtime.initial();
	// _whole.initial();

	bool _new_ret = false;
	long long int _space = 0;
	long long int _total_no_ms = 0;
	long long int _total_space = 0;
	double _avg_space = 0;

	/*  */

	/// Flatten all pattern events for reordering.
	auto sub_patterns = this->Q->TCdecomp;
	for (auto sub_pattern : sub_patterns)
	{
		for (auto pattern_event : sub_pattern)
		{
			this->pattern_events.push_back(pattern_event);
		}
	}

	while (true)
	{	
		vector<dEdge *> concurrent_batch = this->G->next_edges();
		if (concurrent_batch.empty()) break;

	newEdge:
		vector<MatchedPair> reordered_batch;

		/// reorder input events
		for (auto pattern_event : this->pattern_events)
		{
			for (int i = 0; i < concurrent_batch.size(); ++i)
			{
				auto data_event = concurrent_batch[i];
				if (data_event->is_match(pattern_event))
				{
					reordered_batch.push_back(MatchedPair(data_event, pattern_event));
				}
			}
		}

		for (auto _matched : reordered_batch)
		{
			auto _e = _matched.data_event;

#if (defined SPAN)
			this->check_expire_edge(_e);
#endif
			_new_ret = this->new_edge(_matched);
			if (_new_ret)
			{
#ifdef INVALID_READ
				cout << "seenNum=" << this->seen_eNum << "\t";
				cout << "del=" << this->del_eNum << "\t";
				cout << "unmat=" << this->unmat_eNum << endl;
				cout << "\t" << _e->to_str() << endl
					 << endl;
#endif
				this->cur_edges.push(pair<MatchedPair, bool>(_matched, true));
			}
			else
			{
				this->cur_edges.push(pair<MatchedPair, bool>(_matched, false));
			}

#if (defined I_AND_D)
			this->expire_edge(_e);
#if (defined SPAN)
			"err: I_AND_D should not be defined"
#endif
#endif

#if (defined TUPLE)
				this->tuple_expire(_e);
#if (defined I_AND_D) || (defined SPAN)
			"err: TUPLE should not be defined"
#endif

#endif

#ifdef INVALID_READ
				// this->M->to_size();
#endif
			// _rtime.end();

// 			if (this->seen_eNum >= this->win_size || true)
// 			{
				// _space = this->M->to_size();
// 				_total_space += _space;
// #ifdef NO_MS
// 				_total_no_ms += this->M->no_ms_size();
// #ifdef SPACE_LOG
// 				if (_space != this->M->no_ms_size())
// 				{
// 					cerr << "no_ms=" << _total_no_ms << endl;
// 					cerr << "total=" << _total_space << endl;
// 					cerr << "input i to continue....." << endl;
// 					int i;
// 					cin >> i;
// 				}
// 				cout << "no_ms=" << _total_no_ms << endl;
// 				cout << "total=" << _total_space << endl;
// #endif
// #endif
// 			}

#ifdef WHOLE_STR
			cout << this->seen_eNum << "****************incoming: " << _e->to_str() << endl;
			if (this->seen_eNum < this->win_size)
			{
				cout << "to_size=" << this->M->to_size() << endl;
#ifdef NO_MS
				cout << "no_ms_size=" << this->M->no_ms_size() << endl;
#endif
			}
			cout << this->M->whole_str() << "\n==================================" << endl;
#endif
		}
	}

#ifdef CYBER
			util::track(this->run_report());
#endif

#ifdef CYBER
	sleep(10);
	string _ans = this->M->answers_str();
	cout << this->M << '\n';
	cout << _ans << endl;
	cout << "del eNum: " << this->del_eNum << '\n';
	ofstream _fout("cyber.result", ios::out);
	_fout << _ans << endl;
	_fout.close();
#endif

#ifdef MY_GET_NUM_MATCH
	// this->M->num_answer is meaningless without `MY_GET_NUM_MATCH' defined
	// This number represents the number of "non-unique" answers
	cout << "Total number of matches: " << this->M->num_answer << '\n';
#endif

// #ifdef CYBER
	// cout << "start remove edges..." << endl;
// #endif
	while (!this->cur_edges.empty())
	{
		if (this->cur_edges.front().second == true)
		{
			this->expire_edge(this->cur_edges.front().first);
		}

		this->cur_edges.pop();
	}
// #ifdef CYBER
	// cout << "all removed" << endl;
// #endif

	// if (this->seen_eNum > this->win_size)
	// 	_whole.end();
	// double _sum_time = _whole.getsum() / 1000.0;
	// long long int _runtimes = this->seen_eNum - this->win_size;

// #ifdef INVALID_READ
// 	_runtimes = 14;
// #endif
// 	if (_runtimes <= 0)
// 	{
// #if !defined(CYBER) && !defined(MY_GET_NUM_MATCH)
// 		cout << "err runtimes=" << _runtimes << endl;
// 		exit(-1);
// #endif
// 	}
// 	double _avg_time = _sum_time / (_runtimes + 0.0);
// 	_avg_space = _total_space / (_runtimes + 0.0);
// 	double _avg_no_ms = _total_no_ms / (_runtimes + 0.0);

// #ifdef GLOBAL_COMMENT
// 	cout << "noms reduce " << _total_no_ms - _total_space << endl;
// #endif

// 	long long int _throughput = 1000.0 / _avg_time;

// 	this->AVGtime = _avg_time;
// 	this->SUMtime = _sum_time;
// 	this->AVGspace = _avg_space / 1000.0;
// 	this->AVGnoMS = _avg_no_ms / 1000.0;
// 	this->Throughput = _throughput;

// 	this->TALspace = _total_space;
// 	this->TALnoMS = _total_no_ms;

	// this->remaining_threadsjoin();

	delete this->M;

#ifdef GLOBAL_COMMENT
	cout << endl
		 << "OUT stream runs..." << endl
		 << endl;
#endif
}

bool timingsubg::new_edge(MatchedPair _matched_pair)
{
	auto _e = _matched_pair.data_event;
	auto _q = _matched_pair.pattern_event;
#if defined(DEBUG_TRACK) || defined(COMPACT_DEBUG)
	{
		stringstream _ss;
		_ss << "\n====================" << this->seen_eNum << " = " << _e->to_str() << endl;
		util::track(_ss);
#ifdef STD_MAIN
		cout << _ss.str() << endl;
#endif
	}
#endif
	// important
	this->cacheOPlists.clear();
	this->cacheMatEdge.clear();
	this->cacheMatEdge.push_back(_q);

	this->M->getOPlists(_e, this->cacheOPlists, this->cacheMatEdge);
#if defined(DEBUG_TRACK) || defined(COMPACT_TRACK)
	assert(this->cacheMatEdge.size() == 1);
	assert(this->cacheOPlists.size() == 1);
#endif

	if (this->cacheMatEdge.empty())
	{
		this->unmat_eNum++;
		return false;
	}
#ifdef DEBUG_TRACK
	{
		stringstream _ss;
		_ss << "At count=" << this->seen_eNum;
		_ss << ": match " << this->cacheOPlists.size() << " edges" << endl;

		for (int i = 0; i < (int)this->cacheOPlists.size(); i++)
		{
			_ss << i << ": " << this->cacheMatEdge[i]->to_str() << endl;
			this->cacheOPlists[i]->reset();
			while (this->cacheOPlists[i]->hasnext())
			{
				_ss << "\t" << this->cacheOPlists[i]->next()->to_str() << endl;
			}
		}

		util::track(_ss);
#ifdef STD_MAIN
		cout << _ss.str() << endl;
#endif
	}
#endif
	for (int i = 0; i < (int)this->cacheOPlists.size(); i++)
	{

#if defined(DEBUG_TRACK) || defined(COMPACT_DEBUG)
		{
			stringstream _ss;
			_ss << "\n*****start match " << i << endl;
			util::track(_ss);
#ifdef STD_MAIN
			cout << _ss.str() << endl;
#endif
		}
#endif
		// gtransaction *_tran = this->next_tran();

		// _tran->set(_e,
		// 		   this->cacheMatEdge[i],
		// 		   this->cacheOPlists[i],
		// 		   this->G, this->M, this->Q);
		// _tran->count = this->seen_eNum * 100 + i;
#ifdef DEBUG_TRACK
		{
			stringstream _ss;
			_ss << "pthread create [" << _tran->id << "]" << endl;
			util::track(_ss);
		}
#endif

#ifdef NO_THREAD
		// this->M->insert(_tran->e, _tran->qe, _tran->lr_list);
		cacheOPlists[i]->reset();
		this->M->insert(_e, _q, cacheOPlists[i]);
#else
#ifdef PESSIMISTIC_LOCK
		this->M->pessimistic_apply(this->cacheOPlists[i]);
#endif
		// pthread_create(&(_tran->id), NULL, timingsubg::thread_insert, (void *)_tran);
#endif

		{
#ifdef DEBUG_TRACK
			stringstream _ss;
			_ss << "finish [" << _e->to_str() << ",";
			_ss << this->cacheMatEdge[i]->to_str() << "," << _tran->id << "]" << endl;
			util::track(_ss);
#ifdef STD_MAIN
			cout << _ss.str() << endl;
#endif
#endif
		}
	}
	return true;
}

// get some edge "expired" (and remove it)(?)
bool timingsubg::expire_edge(MatchedPair _matched_pair)
{
	auto _e = _matched_pair.data_event;
	auto _q = _matched_pair.pattern_event;
#ifdef INVALID_READ
	cout << "expire: " << _e->to_str() << endl;
#endif

#ifdef ALL_I
	return false;
#endif

#if defined(DEBUG_TRACK) || defined(COMPACT_DEBUG)
	util::track("IN expire_edge");
#endif

	this->cacheTe.clear();
	this->cacheMatEdge.push_back(_q);
	this->M->getTElist(_e, this->cacheTe, this->cacheMatEdge);
#if defined(DEBUG_TRACK)
	{
		stringstream _ss;
		_ss << "There are " << this->cacheTe.size() << " te nodes for";
		_ss << _e->to_str() << endl;
		util::track(_ss);
	}
#endif
	for (int i = 0; i < (int)this->cacheTe.size(); i++)
	{
		/* get next tran */
		// gtransaction *_tran = this->next_tran();
		// /* set the _tran with params */
		// _tran->del_set(_e, this->cacheTe[i], this->G, this->M, this->Q);
#if defined(DEBUG_TRACK) || defined(COMPACT_DEBUG)
		{
			stringstream _ss;
			_ss << "expire[" << i << "]:" << endl;
			_ss << "\t[" << this->cacheMatEdge[i]->to_str() << "]";
			_ss << " -> "
				<< "[" << this->cacheTe[i] << "]" << endl;
			util::track(_ss);
		}
#endif

#ifdef NO_THREAD
		this->M->remove(_e, this->cacheTe[i], NULL);
		// this->M->remove(_tran->e, _tran->tenode, _tran->lr_list);
#else

#ifdef PESSIMISTIC_LOCK
		this->M->pessimistic_apply(_tran->del_tenodes);
#endif

		// pthread_create(&(_tran->id), NULL, timingsubg::thread_remove, (void *)_tran);
#endif

// #ifdef INVALID_READ
#ifdef DEBUG_TRACK
		stringstream _ss;
		_ss << "after exp[" << i << "]:" << endl;
		_ss << "Finish expire test: " << _e->to_str() << endl;
		_ss << this->M->no_ms_size() << endl;
		util::track(_ss);
#endif
	}

#ifdef DEBUG_TRACK
	util::track("OUT expire_edge");
#endif
	return true;
}

bool timingsubg::check_expire_edge(dEdge *_newest)
{
	bool _expired = false;
	while (!this->cur_edges.empty())
	{
		if (this->G->is_expire(this->cur_edges.front().first.data_event, _newest))
		{
#ifdef COMPACT_DEBUG
			if (this->cur_edges.front().first.data_event->id == 9504591) {
				stringstream _ss;
				_ss << "9504591 expires at " << _newest->get_timestamp() << '\n';
			}
#endif
		
			if (this->cur_edges.front().second == true)
				this->expire_edge(this->cur_edges.front().first);

			this->cur_edges.pop();
			_expired = true;
			this->del_eNum++;
		}
		else
		{
			break;
		}
	}

	bool win_limit = false;
#ifdef WIN_LIMIT
	win_limit = ((int)this->cur_edges.size()) > this->win_size * 2;
#endif

	if (!_expired && win_limit)
	{
		if (this->cur_edges.front().second == true)
			this->expire_edge(this->cur_edges.front().first);

		this->cur_edges.pop();
		_expired = true;
		this->del_eNum++;
	}

	return _expired;
}

bool timingsubg::tuple_expire(dEdge *_e)
{
	if (this->win_size < (int)this->cur_edges.size())
	{
		if (this->cur_edges.front().second == false)
		{
			this->cur_edges.pop();
#ifdef DEBUG_TRACK
			{
				stringstream _ss;
				_ss << "expire NULL AT " << this->seen_eNum << endl;
				util::track(_ss);
#ifdef STD_MAIN
				cout << _ss.str() << endl;
#endif
			}
#endif
			return true;
		}
#ifdef DEBUG_TRACK
		{
			stringstream _ss;
			_ss << "AT " << this->seen_eNum << " expire edge: " << this->cur_edges.front().first.data_event->to_str() << endl;
			util::track(_ss);
#ifdef STD_MAIN
			cout << _ss.str() << endl;
#endif
		}
#endif

		this->expire_edge(this->cur_edges.front().first);
		this->cur_edges.pop();
		return _e != NULL;
	}
	return false;
}

timingconf *timingsubg::getconf()
{
	return this->tconf;
}

gtransaction *timingsubg::next_tran()
{
	gtransaction *_ret = NULL;
	_ret = this->tran_pool.front();

	_ret->thread_join();

	this->tran_pool.pop();

	this->tran_pool.push(_ret);

	return _ret;
}

string timingsubg::run_report()
{
	stringstream _ss;
	_ss << "[a" << this->seen_eNum / 1000 << "k, u" << this->unmat_eNum;
	_ss << ", m" << (this->seen_eNum - this->unmat_eNum) << ", ";
	_ss << "d" << (this->del_eNum) << ", w" << this->cur_edges.size();
	_ss << "]  ";
	return _ss.str();
}

void timingsubg::remaining_threadsjoin()
{
#ifdef GLOBAL_COMMENT
	cout << endl
		 << "start join thread..." << endl;
	cout.flush();
#endif
	while (!this->tran_pool.empty())
	{
		gtransaction *_gt = this->tran_pool.front();
#ifndef NO_THREAD
		_gt->thread_join();
#endif
		delete _gt;
		this->tran_pool.pop();
	}
#ifdef GLOBAL_COMMENT
	cout << "finish join thread..." << endl;
	cout.flush();
#endif
}

/// @brief  NOT used
/// @param _tran 
/// @return 
void *timingsubg::thread_insert(void *_tran)
{
	gtransaction *_gt = ((gtransaction *)_tran);
	dEdge *_e = _gt->e;
	qEdge *_qe = _gt->qe;
	msforest *_m = _gt->M;

	util::reg_track(_gt->count);

#ifdef DEBUG_TRACK
	{
		stringstream _ss;
		_ss << "\n\nNEW insert thread:  [" << pthread_self() << "]" << endl;
		_ss << "\t" << _e->to_str() << "\t" << _qe->to_str() << endl;
		util::track(_ss);
	}
#endif

	// _m->insert(_e, _qe, _gt->lr_list);

#ifdef PESSIMISTIC_LOCK
	_m->reset_te2lock(_gt->lr_list);
#endif

#ifdef DEBUG_TRACK
	util::track("INSERTION FINISH\n\n");
#endif
	_gt->flag_exit = true;
	return NULL;
}

void *timingsubg::thread_remove(void *_tran)
{
	gtransaction *_gt = ((gtransaction *)_tran);
	dEdge *_e = _gt->e;
	teNode *_node = _gt->tenode;
	msforest *_m = _gt->M;

	util::reg_track(_gt->count);

#ifdef DEBUG_TRACK
	{
		stringstream _ss;
		_ss << "\n\nNEW remove thread:  [" << pthread_self() << "]" << endl;
		_ss << "\t" << _e->to_str() << "\t" << _node->to_str() << endl;
		util::track(_ss);
	}
#endif

	_m->remove(_e, _node, _gt->lr_list);

#ifdef PESSIMISTIC_LOCK
	List<teNode> &_telist = _gt->del_tenodes;
	_m->reset_te2lock(_telist);
#endif

#ifdef DEBUG_TRACK
	util::track("REMOVE FINISH\n\n");
#endif
	_gt->flag_exit = true;
	return NULL;
}

string timingsubg::stat_str()
{
	stringstream _ss;
	_ss << this->exename;
	_ss << "," << fixed << this->win_size;
	_ss << "," << fixed << this->AVGtime;
	_ss << "," << fixed << this->Throughput;
	_ss << "," << fixed << this->AVGspace;
	_ss << "," << fixed << this->AVGnoMS;
	_ss << "," << fixed << this->SUMtime;
	_ss << "," << fixed << this->tconf->getmaxthreadNum();
	_ss << "," << fixed << this->Q->param_str();
	_ss << "," << fixed << this->seen_eNum;

	_ss << "," << this->TALspace;
	_ss << "," << this->TALnoMS;
	_ss << "," << this->TALspace - this->TALnoMS;

	return _ss.str();
}

bool timingsubg::write_stat()
{
	fstream frun(this->fruntime.c_str(), ios::app | ios::out);
	if (!frun)
	{
		cout << "err: " << this->fruntime << endl;
		exit(-1);
	}

	if ((int)frun.tellg() < 10)
	{
		frun << "exe,WinSize,AVGtime,Throughput,AVGspace,AVGnoms,SUMtime,Thread,Qsize,Qno,timingratio,eNum" << endl;
	}

	frun << this->stat_str() << endl;
	frun.flush();
	frun.close();
	return true;
}
