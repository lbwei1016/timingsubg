#ifndef _NETSTREAM_H_
#define _NETSTREAM_H_

#include "../gstream/gstream.h"
#include "RdfDedge.h"
#include "RdfQedge.h"

#include <queue>
#include <climits>

class rdfstream : public gstream
{
public:
	rdfstream(string _dat_set);
	~rdfstream();

	bool load_edges(int64_t _avg_win_tuple_num);
	void initialize(int64_t window_size);
	bool is_expire(dEdge *_e_old, dEdge *_e_new);
	bool nothing_to_send();
	rdfDedge *next_valid_record();
	vector<dEdge *> get_batch();
	vector<dEdge *> next_edges();
	// int64_t get_timestamp(dEdge *e);

	struct rdfDedgeComparator
	{
		bool operator()(rdfDedge *_a, rdfDedge *_b)
		{
			return _a->t_sec > _b->t_sec;
		}
	};

	priority_queue<rdfDedge *, vector<rdfDedge *>, rdfDedgeComparator> buffer;
	ifstream *fin;
	int64_t boundary_time;

private:
	int64_t avg_span_t;
	int64_t window_size;
};

#endif
