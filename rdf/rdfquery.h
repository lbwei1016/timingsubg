#ifndef _NETQUERY_H_
#define _NETQUERY_H_

#include "../timing/query.h"

class rdfquery: public query{
public:
	rdfquery(string _query_path);
	rdfquery(string _pattern_path, string _subpattern_path);
	~rdfquery();

	bool parseQuery();
};

#endif
