#include "rdfstream.h"

rdfstream::rdfstream(string _dat_set) : gstream(_dat_set)
{
}

rdfstream::~rdfstream()
{
#ifdef GLOBAL_COMMENT
	cout << "rdfstream destruction..." << endl;
#endif
}

/*
 * sid pid oid stype otype literal(otype=="literal") timestamp
 * read edges is implemented in RdfDedge
 *
 * */
bool rdfstream::load_edges(int64_t _avg_win_tuple_num)
{
#ifdef GLOBAL_COMMENT
	cout << "IN load_edges from [" << this->data_path << "]" << endl;
#endif
#ifdef DEBUG_TRACK
	util::track("IN load_edges from " + this->data_path);
#endif
	ifstream fin(this->data_path.c_str(), ios::in);
	if (!fin)
	{
		cout << "err: failed open " << this->data_path << endl;
		exit(-1);
	}
	/* load edges */

#ifdef MY_DEBUG
	cout << "Loading data graph...\n";
#endif

	char _buf[5000];
	rdfDedge *_rd = NULL;
	deque<rdfDedge *> edge_buf;

	while (!fin.eof())
	{
		fin.getline(_buf, 4999, '\n');
		// a strange last line
		if (strlen(_buf) < 5)
			break;
		_rd = new rdfDedge(_buf);

		while (edge_buf.size() && edge_buf.front()->t_sec < _rd->t_sec) {
			auto *e = edge_buf.front();
			edge_buf.pop_front();
			this->alledges.push_back((dEdge *)e);
		}

		this->alledges.push_back((dEdge *)_rd);

		rdfDedge *_end_rd = _rd->split();
		if (_end_rd)
		{
			// this->alledges.push_back((dEdge *)_end_rd);
			edge_buf.push_back(_end_rd);
		}
		// else cout << "NOT splitted!\n";
	}
	fin.close();

	// #ifdef MY_DEBUG
	// 	cout << "Now outputing data edges...\n";
	// 	for (auto &_edge: alledges) {
	// 		auto edge = (rdfDedge*)_edge;
	// 		printf("(time, s, t, eid) = (%ld, %d, %d, %d)\n", edge->t_sec, edge->s, edge->t, edge->id);
	// 	}
	// 	cout << "Loading data graph finished.\n";
	// #endif

	/* calculate avg time span */
	int64_t all_enum = this->alledges.size();
	rdfDedge *_last = (rdfDedge *)(this->alledges[all_enum - 1]);
	rdfDedge *_first = (rdfDedge *)(this->alledges[0]);

	int64_t _tspan = _last->t_sec - _first->t_sec;
	int64_t _win_times = 1 + this->alledges.size() / _avg_win_tuple_num;
	int64_t _avg_span = 1 + _tspan / _win_times;
	this->avg_span_t = _avg_span;
	// this->avg_span_t = _avg_win_tuple_num;
	// this->window_size = _avg_win_tuple_num;

#ifdef MY_DEBUG
	// cout << "avg_span_t: " << this->avg_span_t << '\n';
	// cout << "effective window size: " << this->window_size << '\n';
	cout << "effective window size: " << this->avg_span_t << '\n';
#endif

#ifdef GLOBAL_COMMENT
	cout << "tspan[" << _tspan << "],"
		 << "all[" << this->alledges.size() << "], ";
	cout << "avg_tuple=" << _avg_win_tuple_num << ", avg_span=" << _avg_span << endl;

	cout << "OUT load_edges: " << this->alledges.size() << endl;

	for (int i = 0; i < this->alledges.size(); ++i)
	{
		cout << "(" << this->alledges[i]->s << ", " << this->alledges[i]->t << ")\n";
	}
#endif
	return true;
}

bool rdfstream::is_expire(dEdge *_e_old, dEdge *_e_new)
{
	rdfDedge *_re1 = (rdfDedge *)_e_old;
	rdfDedge *_re2 = (rdfDedge *)_e_new;
	// printf("_re2->t_sec: %ld, _re1->t_sec: %ld, diff = %ld\n", _re2->t_sec, _re1->t_sec, _re2->t_sec - _re1->t_sec);
	if (_re2->t_sec - _re1->t_sec < this->avg_span_t)
		return false;
	// if(_re2->t_sec - _re1->t_sec < this->window_size) return false;

	return true;
}

// int64_t get_timestamp(dEdge *e) {
// 	rdfDedge *re = (rdfDedge*)e;
// 	return re->t_sec;
// }
