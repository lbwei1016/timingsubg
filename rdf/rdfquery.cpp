#include "rdfquery.h"
#include "RdfQedge.h"
#include "json/json.hpp"

using json = nlohmann::json;

rdfquery::rdfquery(string _query_path) : query(_query_path)
{
}

rdfquery::rdfquery(string _pattern_path, string _subpattern_path) : query(_pattern_path)
{
	this->subpattern_path = _subpattern_path;
}


rdfquery::~rdfquery()
{
#ifdef GLOBAL_COMMENT
	cout << "IN destruct rdfquery" << endl;
#endif

	for (int i = 0; i < (int)this->left2right_leaf.size(); i++)
	{
		delete this->left2right_leaf[i];
	}

#ifdef GLOBAL_COMMENT
	cout << "OUT destruct rdfquery" << endl;
#endif
}

bool rdfquery::parseQuery()
{
#ifdef GLOBAL_COMMENT
	cout << "IN parseQuery@rdfquery: " << this->query_path << endl;
#endif
	// Parse the behavioral pattern
	ifstream pattern_file(this->query_path.c_str(), ios::in);
	if (!pattern_file)
	{
		cout << "err open pattern" << this->query_path << endl;
		exit(-1);
	}

	json pattern = json::parse(pattern_file);
	string version = pattern["Version"];
	bool useRegex = pattern["UseRegex"];
	auto events = pattern["Events"];

	map<int, qEdge *> id2qedges;
	id2qedges.clear();

	for (auto &[_key, val] : events.items())
	{
		// implicitly convert 'val["Parents"]' to 'vector<int>'
		qEdge *edge = new rdfQedge(val["SubjectID"], val["ObjectID"], val["ID"], val["Signature"], val["Parents"]);
		id2qedges[val["ID"]] = edge;
	}

	// Add preEdges (i.e., parents)
	for (auto &[id, event] : id2qedges) {
		for (auto parent : event->parents) {
			event->add_preedge(id2qedges[parent]);
		}
	}

#ifdef MY_DEBUG
	printf("Now printing pattern edges:\n");
	for (auto &[id, event]: id2qedges) {
		printf("(id, s, t) = (%d, %d, %d)\n", event->id, event->s, event->t);
		printf("preEdges:\n");
		for (auto &pre: event->preEdges) {
			printf("\t(id, s, t) = (%d, %d, %d)\n", pre->id, pre->s, pre->t);
		}
		puts("");
	}
#endif


	// Parse the decomposed subpatterns
	ifstream subpattern_file(this->subpattern_path.c_str(), ios::in);
	if (!subpattern_file)
	{
		cout << "err open subpattern" << this->subpattern_path << endl;
		exit(-1);
	}

	json subpatterns = json::parse(subpattern_file);
    for (auto &[_key, val] : subpatterns.items()) {
		vector<qEdge*> subpattern;
        for (auto event : val["events"]) {
			subpattern.push_back(id2qedges[event["id"]]);
        }
		this->TCdecomp.push_back(subpattern);
    }

#ifdef RUN_COMMENT
	cout << "before parse: " << this->to_str() << endl;
#endif
	/* build left2right_leaf */
	this->left2right_leaf.clear();
	for (int i = 0; i < (int)this->TCdecomp.size(); i++)
	{
		for (int j = 0; j < (int)this->TCdecomp[i].size(); j++)
		{
			this->left2right_leaf.push_back(this->TCdecomp[i][j]);
#ifdef RUN_COMMENT
			cout << "At " << i << "," << j << " ";
			cout << "left2right_leaf push: " << this->TCdecomp[i][j]->to_str() << endl;
#endif
		}
	}

	this->fill_timeorder();


#ifdef RUN_COMMENT
	cout << "sizeof id2qedges: " << id2qedges.size() << endl;
	cout << "sizeof left2right_leaf " << left2right_leaf.size() << endl;
	cout << "left2right leaves: " << endl;
	for (int i = 0; i < (int)left2right_leaf.size(); i++)
	{
		cout << "\t" << this->left2right_leaf[i]->to_str() << endl;
	}

	cout << "after parse: " << this->to_str() << endl;

	stringstream _ss;
	_ss << "{";
	for (int i = 0; i < (int)this->TCdecomp.size(); i++)
	{
		_ss << i << "[" << 0 << "(" << this->TCdecomp[i][0]->to_str() << ")";
		for (int j = 1; j < (int)this->TCdecomp[i].size(); j++)
		{
			_ss << ", " << j << "(" << this->TCdecomp[i][j]->to_str() << ")";
		}
		_ss << "]; ";
	}
	_ss << "}";
	cout << "ori: " << _ss.str() << endl;
#endif

	return true;
}
