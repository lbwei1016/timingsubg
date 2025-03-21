#include "rdfstream.h"
#include "../util/util.h"
#include "rdfquery.h"
#include "../timing/query.h"
#include "../timing/timingconf.h"
#include "../timing/timingsubg.h"

#include <sys/resource.h>
#include <iomanip>

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
int main(int argc, char *argv[])
{
#ifdef DEBUG_TRACK
#endif

#ifdef THREAD_LOG
#endif
	struct rusage ru;
	struct timeval utime;
	struct timeval stime;

	if (argc < 7)
	{
		cout << "err argc" << endl;
		for (int i = 0; i < argc; i++)
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
		for (int i = 3; i < argc; i++)
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
	cout << "query Q: \n"
		 << _Q.to_str() << endl;
	cout << "pre Q: \n"
		 << _Q.timingorder_str() << endl;
#endif

#if defined(DEBUG_TRACK) || defined(CYBER) || defined(COMPACT_DEBUG)
	util::init_track("./track");
#elif defined(MY_GET_NUM_MATCH)
	util::init_track("./answers");
#endif

	timingsubg tsubg(0, _frtime);
	tsubg.exename = string(argv[0]);
	tsubg.run(0, &_NS, &_Q, &_tconf);
#ifdef GLOBAL_COMMENT
	cout << "finish timingsubg run" << endl;
#endif

	tsubg.write_stat();

#if defined(DEBUG_TRACK) || defined(CYBER) || defined(MY_GET_NUM_MATCH)
	util::finalize();
#endif
	
	getrusage(RUSAGE_SELF, &ru);

	utime = ru.ru_utime;
	stime = ru.ru_stime;
	double utime_used = utime.tv_sec + (double)utime.tv_usec / 1000000.0;
	double stime_used = stime.tv_sec + (double)stime.tv_usec / 1000000.0;

	cout << "CPU time elapsed: " << fixed << setprecision(4) << utime_used + stime_used << " secs\n";
	cout << "Peak memory usage: " << ru.ru_maxrss << " kB\n";
}