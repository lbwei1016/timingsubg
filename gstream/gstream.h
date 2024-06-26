#ifndef _GSTREAM_H_
#define _GSTREAM_H_

#include "../util/util.h"

class dEdge;
class gstream{
public:
	gstream(string& _data_path);
	~gstream();
	/*
	 * check empty of alledges
	 * read all edges into alledges
	 * call reset();
	 */
	virtual bool load_edges(int64_t _avg_win_tuple_num) = 0;
	virtual bool is_expire(dEdge* _e_old, dEdge* _e_new) = 0;
	virtual void initialize(int64_t window_size) = 0;
	virtual vector<dEdge *> next_edges() = 0;
	bool reset();
	bool hasnext();
	dEdge * next();
	int size();
	dEdge *peek();

protected:
	vector<dEdge*>::iterator cur_itr;
	string data_path;
	vector<dEdge*> alledges;
};


#endif
