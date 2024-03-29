#include "RdfDedge.h"
#include "RdfQedge.h"

#include <regex>

rdfDedge::rdfDedge(int _s, int _t) : dEdge(_s, _t)
{
	
}

rdfDedge::~rdfDedge(){

}

// Construct a single data edge (For IPMES)
rdfDedge::rdfDedge(string _str_e) : dEdge(-1, -1)
{
	stringstream _ss(_str_e);
	string token;

	int cnt = 0;
	// int start_time, end_time;
	while(std::getline(_ss, token, ',')) {
		switch (cnt)
		{
		case 0:
			// start_time = int64_t(stod(token) * 1000);
			this->t_sec = int64_t(stod(token) * 1000);
			break;
		case 1:
			// (IPMES) not used currently
			// maybe split this into two edges?
			this->end_time = int64_t(stod(token) * 1000);
			break;
		case 2:
			this->signature = token;
			break;
		case 3:
			this->id = stoi(token);
			break;
		case 4: 
			this->s = stoi(token);
			break;
		case 5:
			this->t = stoi(token);
			break;
		default:
			break;
		}

		++cnt;

		// #ifdef MY_DEBUG
		// 	std::cout << "Now parsing: " << token << '\n';
		// #endif
	}
}

// Split into two events
rdfDedge* rdfDedge::split() {
	if (this->t_sec == this->end_time) 
	{
		cout << "NO need to split!\n";
		return NULL;
	}

	cout << this->t_sec << ' ' << this->end_time << '\n';

	rdfDedge *_end_d = new rdfDedge(this->s, this->t);
	_end_d->id = this->id;
	_end_d->signature = this->signature;
	_end_d->t_sec = this->end_time;
	return _end_d;
}

bool rdfDedge::is_same(dEdge* _d)
{
	return false;
	rdfDedge* _nd = (rdfDedge*)_d;
	if(this == _nd) return true;
	
	// if(this->s != _nd->s) return false;
	// if(this->t != _nd->t) return false;
	// if(this->pre != _nd->pre) return false;
	// if(this->t_sec != _nd->t_sec) return false;
	// if(this->stype != _nd->stype ) return false;
	// if(this->otype != _nd->otype ) return false;
	// if(this->literal != _nd->literal) return false;

	if (this->signature != _nd->signature) return false;
	// if (this->id != _nd->id) return false;
	
	return true;
}

bool rdfDedge::is_before(dEdge* _d)
{
	rdfDedge* _nd = (rdfDedge*)_d;
	if(this->t_sec <= _nd->t_sec) return true;

	return false;
}

bool rdfDedge::is_after(dEdge* _d)
{
	return ((rdfDedge*)_d)->is_before( (dEdge*)this );
}

bool rdfDedge::is_match(qEdge* _q)
{
	rdfQedge* _rq = (rdfQedge*)_q;

	// if (this->id != _rq->id) return false;
	// no Regex is considered yet

// #ifdef MY_DEBUG
// 	printf("Now matching event %d and %d:\n", this->id, _rq->id);
// #endif

	regex reg(_rq->signature);
	if (!regex_match(this->signature, reg)) return false;

	// if (this->signature != _rq->signature) return false;

// #ifdef MY_DEBUG
	// printf("Match success!\n-----------------\n");
// #endif

#ifdef ENABLE_LITERAL
	if(this->is_literal() && _rq->literal != "NULL")
	{
		if(this->literal != _rq->literal)
			return false;
	}
#endif

	return true;
}
	
bool rdfDedge::is_literal()
{
	return this->otype == "literal";
}

string rdfDedge::to_str()
{
	stringstream _ss;
	// _ss << this->stype << this->s << " " << this->pre << " " << this->otype << this->t << " ";
	// _ss << this->literal;
	// _ss << " " << t_sec;

	/// Only print ids
	_ss << this->id;

	//_ss << " " << size;
	return _ss.str();
}
