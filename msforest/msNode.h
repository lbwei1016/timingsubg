#ifndef _MSNODE_H
#define _MSNODE_H

#include "../timing/match.h"
#include "../util/list_T.h"

#include <memory>

class msNode;
class query;
class JoinResult{
public:
	shared_ptr<msNode> first;
	List<match>* second;
	
	~JoinResult();
};

class msNode{
public:
	msNode(shared_ptr<msNode> _f, shared_ptr<msNode> _c, match* _mat);
	msNode(msNode *_m);
	~msNode();

	long long int to_size();
	long long int no_ms_size();
	long long int level_mat_size();
	long long int whole_mat_size();

	shared_ptr<msNode> father;
	shared_ptr<msNode> child_first;
	// shared_ptr<msNode *> child_first;
	shared_ptr<msNode> next;
	shared_ptr<msNode> prev;
	match* mat;
	match* path_match;
	
#ifdef MARK_DEL
	bool mark_del;
#endif

	/* join owned match with _matches into jrList */
	bool joinwith(List<match>* _matches, List<JoinResult>* jrList, query* _q);
	/* create new children over _matches and add into childrenlist */
	bool addBranches(List<match>* _matches, bool& _is_first_child);
	/* add new_list into children list */
	bool addChildren(shared_ptr<msNode> _new_child_list, bool& _is_first_child);

	shared_ptr<msNode> be_removed();
	bool is_dedge(dEdge* _e);
	
	static shared_ptr<msNode> build_mslist(List<match>* _matlist, shared_ptr<msNode> _father);

	/* get whole match */
	match* get_whole_match();
	string whole_match_str();
};

#endif
