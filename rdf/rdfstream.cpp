#include "rdfstream.h"

rdfstream::rdfstream(string _dat_set) : gstream(_dat_set)
{
}

rdfstream::~rdfstream()
{
	delete this->fin;
#ifdef GLOBAL_COMMENT
	cout << "rdfstream destruction..." << endl;
#endif
}

/// @brief Initialize RdfStream
/// @param window_size 
void rdfstream::initialize(int64_t window_size)
{
	this->boundary_time = 0;
	this->fin = new ifstream(this->data_path.c_str(), ios::in);
	if (!(*fin))
	{
		cout << "err: failed open " << this->data_path << endl;
		delete this->fin;
		exit(-1);
	}
	this->avg_span_t = window_size;

#if defined(MY_DEBUG) || defined(DEBUG_TRACK) || defined(MY_GET_NUM_MATCH)
	cout << "effective window size: " << this->avg_span_t << '\n';
#endif
}

/// @brief Check whether there are events ready to be sent (in the buffer)
/// @return `true` for yes
bool rdfstream::nothing_to_send()
{
	if (this->buffer.empty())
		return true;
	return this->buffer.top()->get_timestamp() >= this->boundary_time;
}

/// @brief Get the next input event
/// @return A pointer to the event read (if any)
rdfDedge *rdfstream::next_valid_record()
{
	char _buf[500];
	while (!this->fin->eof())
	{
		this->fin->getline(_buf, 499, '\n');
		// a strange last line
		if (strlen(_buf) < 5)
			break;

		rdfDedge *e = new rdfDedge(_buf);
		return e;
	}
	return nullptr;
}

/// @brief Get a batch of events having the same timestamp
/// @return A batch of events having the same timestamp
vector<dEdge *> rdfstream::get_batch()
{
	vector<dEdge *> batch;
	while (this->buffer.size())
	{
		if (this->buffer.top()->get_timestamp() < this->boundary_time)
		{
			batch.push_back(this->buffer.top());
			this->buffer.pop();
		}
		else 
			break;
	}
	return batch;
}


/// @brief Streamingly read the input events, until there are no events left
/// @return A batch of events having the same timestamp
vector<dEdge *> rdfstream::next_edges()
{
	while (this->nothing_to_send())
	{
		rdfDedge *e = this->next_valid_record();
		if (e == nullptr)
		{
			this->boundary_time = LONG_LONG_MAX;
			break;
		}

		this->boundary_time = e->get_timestamp();
		this->buffer.push(e);
		rdfDedge *e2 = e->split();
		if (e2)
		{
			this->buffer.push(e2);
		}
	}
	return this->get_batch();
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
	priority_queue<rdfDedge *, vector<rdfDedge *>, rdfDedgeComparator> edge_buf;

	while (!fin.eof())
	{
		fin.getline(_buf, 4999, '\n');
		// a strange last line
		if (strlen(_buf) < 5)
			break;
		_rd = new rdfDedge(_buf);

		while (edge_buf.size() && edge_buf.top()->t_sec <= _rd->t_sec)
		{
			auto *e = edge_buf.top();
			edge_buf.pop();
			this->alledges.push_back((dEdge *)e);
		}

		this->alledges.push_back((dEdge *)_rd);

		rdfDedge *_end_rd = _rd->split();
		if (_end_rd)
		{
			edge_buf.push(_end_rd);
		}
	}
	fin.close();

	while (edge_buf.size())
	{
		auto *e = edge_buf.top();
		edge_buf.pop();
		this->alledges.push_back((dEdge *)e);
	}

#ifdef MY_DEBUG
	cout << "Now outputing all data edges...\n";
	for (auto &_edge : this->alledges)
	{
		auto edge = (rdfDedge *)_edge;
		printf("(time, s, t, eid) = (%ld, %d, %d, %d)\n", edge->t_sec, edge->s, edge->t, edge->id);
	}
	cout << "Loading data graph finished.\n";
#endif

	/* calculate avg time span */
	int64_t all_enum = this->alledges.size();
	rdfDedge *_last = (rdfDedge *)(this->alledges[all_enum - 1]);
	rdfDedge *_first = (rdfDedge *)(this->alledges[0]);

	int64_t _tspan = _last->t_sec - _first->t_sec;
	int64_t _win_times = 1 + this->alledges.size() / _avg_win_tuple_num;
	int64_t _avg_span = 1 + _tspan / _win_times;
	// this->avg_span_t = _avg_span;
	/// Directly use the user-provided window size
	this->avg_span_t = _avg_win_tuple_num;
	// this->window_size = _avg_win_tuple_num;

#if defined(MY_DEBUG) || defined(DEBUG_TRACK) || defined(MY_GET_NUM_MATCH)
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
	if (_re2->t_sec - _re1->t_sec < this->avg_span_t)
		return false;

	return true;
}
