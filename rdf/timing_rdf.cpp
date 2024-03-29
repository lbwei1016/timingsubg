#include "rdfstream.h"
#include "../util/util.h"
#include "rdfquery.h"
#include "../timing/query.h"
#include "../timing/timingconf.h"
#include "../timing/timingsubg.h"

/* 
 * argv0 : exe
 * argv1 : dataset
 * argv2 : query
 * argv3 : winsz
 * argv4 : Max_Thread_Num
 * argv5 : File for recording the runtime analysis
 * argv6 : subpattern (for IPEMS)
 *
 * */
int main(int argc, char* argv[])
{
#ifdef DEBUG_TRACK
#endif

#ifdef THREAD_LOG
#endif
	
	if(argc < 7){
		cout << "err argc" << endl;
		for(int i = 0; i < argc; i ++)
		{
			cout << argv[i] << "\t\t";
		}
		cout << endl;
		exit(0);
	}
#ifdef GLOBAL_COMMENT
	cout << "num of argc: " << argc << endl;
#endif

	string _dataset, _query, _frtime, _subpattern;
	_dataset = string(argv[1]);
	_query = string(argv[2]);
	_frtime = string(argv[5]);
	_subpattern = string(argv[6]);

	double _window;
	int max_thread_num;
	{
		stringstream _ss;
		for(int i = 3; i < argc; i ++) 
			_ss << argv[i] << " ";
		_ss >> _window >> max_thread_num;
	}

#ifdef GLOBAL_COMMENT
	cout << "runtime=" << _frtime << endl;
#endif
	// All events are recorded in microsecond
	_window = (int64_t)(1000 * _window);

	timingconf _tconf(_window, max_thread_num, _dataset, _query);
	rdfstream _NS(_dataset);
	rdfquery _Q(_query, _subpattern);

	_Q.parseQuery();
#ifdef RUN_COMMENT
	cout << "query Q: \n" << _Q.to_str() << endl;
	cout << "pre Q: \n" << _Q.timingorder_str() << endl;
#endif

	util::init_track("./track");
	cout << "I am init_track!!!!!\n";

	timingsubg tsubg(0, _frtime);
	tsubg.exename = string(argv[0]);
	tsubg.run(0, &_NS, &_Q, &_tconf);
#ifdef GLOBAL_COMMENT
	cout << "finish timingsubg run" << endl;
#endif
	
	tsubg.write_stat();

	util::finalize();
}

