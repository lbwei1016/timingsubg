#ifndef _NETQEDGE_H_
#define _NETQEDGE_H_

#include "../timing/edge.h"
#include <vector>

/// @todo: Regex? Something else?
class rdfQedge : public qEdge{
public:
	rdfQedge(int _s, int _t);
	rdfQedge(string _str_e);
	rdfQedge(int _s, int _t, string _stype, string _otype, string _pre, string _literal);
  	rdfQedge(int _s, int _t, int _id, string _signature, vector<int> _parents);
	~rdfQedge();

	bool is_before(qEdge* _q);
	bool is_after(qEdge* _q);
	bool is_match(dEdge* _d);
	string to_str();
	string label();


	string stype;
	string otype;
	string pre;
	string literal;
};


#endif
