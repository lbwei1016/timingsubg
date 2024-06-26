#include "msNode.h"
#include "../timing/query.h"
#include "../util/util.h"
#include "../util/list_T.h"

JoinResult::~JoinResult()
{
	delete second;
}

msNode::msNode(shared_ptr<msNode> _f, shared_ptr<msNode> _c, match *_mat)
{
	this->father = _f;
	this->child_first = _c;
	this->mat = _mat;
	this->next = NULL;
	this->prev = NULL;
	this->path_match = NULL;
#ifdef MARK_DEL
	this->mark_del = false;
#endif

#ifdef NO_MS
	this->get_whole_match();
#endif
}

// msNode::msNode(msNode *_m) {
// 	this->father = _m->father;
// 	this->child_first = _m->child_first;
// 	this->mat = _m->mat;
// 	this->next = _m->next;
// 	this->prev = _m->prev;
// 	this->path_match = _m->path_match;
// }

msNode::~msNode()
{	
#ifdef MY_DEBUG
	cout << "msNode destructor called!\n";
#endif
	delete this->path_match;
}

long long int msNode::to_size()
{
	long long int _sz = 0;

	_sz += sizeof(msNode);

	_sz += this->level_mat_size();

	return _sz;
}

long long int msNode::no_ms_size()
{
	long long int _sz = this->to_size();
	shared_ptr<msNode> _cur = this->father;
#ifdef SPACE_LOG
	cout << "no_ms@msNode[" << this << "]" << endl;
	cout << "father=" << this->father << endl;
	if (_cur != NULL)
	{
		cout << "cur as father not NULL" << this->whole_match_str() << endl;
	}
#endif
	while (_cur != NULL)
	{
		_sz += _cur->to_size();
		_cur = _cur->father;
	}
	return _sz;
}

int64_t msNode::get_list_length() {
	// int64_t length = 0;
	set<int64_t> vis;
	shared_ptr<msNode> cur = shared_from_this();
	while (cur != nullptr) {
		if (vis.count((int64_t)cur.get()) != 0) {
			cout << "LOOP!\n aborting...";
			abort();
		}
		vis.insert((int64_t)cur.get());
		cur = cur->next;
	}
	return vis.size();
}

long long int msNode::level_mat_size()
{
	return (this->mat->size()) * (sizeof(dEdge *) + sizeof(qEdge *));
}

long long int msNode::whole_mat_size()
{
	long long int _sz = 0;
	if (this->father != NULL)
	{
		_sz += this->father->whole_mat_size();
	}
	_sz += this->level_mat_size();
	return _sz;
}

// bool check_entity_uniqueness(set<int64_t> &used)
// {

// }

/* join owned match with _matches into jrList */
// jr: Join Result?
bool msNode::joinwith(List<match> *_matches, List<JoinResult> *jrList, query *_q)
{
	match *_mat = this->get_whole_match();
#ifdef NO_THREAD
#ifdef DEBUG_TRACK
	{
		stringstream _ss;
		_ss << "IN join with: matches size: " << _matches->size() << endl;
		_ss << "\t" << _mat->to_str() << endl;
		util::track(_ss);
	}
#endif
#endif

	/* many filter can be conducted here */
	_matches->reset();
	JoinResult *_jr = NULL;
	set<int64_t> used;
	while (_matches->hasnext())
	{
		match *_m2 = _matches->next();
		if (_q->compatible(_mat, _m2))
		{
			if (_jr == NULL)
			{
				_jr = new JoinResult;
				// _jr->first = this;
				_jr->first = shared_from_this();
				_jr->second = new List<match>(false);
			}

			_jr->second->add(_m2);

#ifdef NO_THREAD
#if defined(DEBUG_TRACK) || defined(COMPACT_DEBUG)
			{
				stringstream _ss;
				_ss << "\tcompatible : " << endl;
				_ss << "\t\t" << _mat->to_str() << " VS " << _m2->to_str() << endl;
				util::track(_ss);
			}
#endif
#endif
		}
#ifdef NO_THREAD
#ifdef DEBUG_TRACK
		else
		{
			stringstream _ss;
			_ss << "\tuncompatible : " << endl;
			_ss << "\t\t" << _mat->to_str() << " VS " << _m2->to_str() << endl;
			util::track(_ss);
		}
#endif
#endif
	}

	if (_jr != NULL)
	{
		jrList->add(_jr);
	}

	return true;
}

/* create new children over _matches and add into childrenlist
 *
 * */
// bool msNode::addBranches(List<match> *_matches, bool &_is_first_child)
// {
// 	shared_ptr<msNode> _list = msNode::build_mslist(_matches, this);
// 	this->addChildren(_list, _is_first_child);
// 	return true;
// }

/* add new_list into children list
 * is_first_child: true if previous child is NULL, false otherwise
 * */
bool msNode::addChildren(shared_ptr<msNode> _new_child_list, bool &_is_first_child)
{
#ifdef DEBUG_TRACK
	{
		stringstream _ss;
		_ss << "\t\tfirst_child=" << this->child_first << endl;
		if (this->child_first != NULL)
		{
			_ss << "\t\tfirst_prev=" << this->child_first->prev << endl;
			shared_ptr<msNode> _mstmp = this->child_first->next;
			while (_mstmp != NULL)
			{
				_ss << "\t\t" << _mstmp << " f=" << _mstmp->father << endl;
				_mstmp = _mstmp->next;
			}
		}
		util::track(_ss);
	}
#endif

	if (_new_child_list == NULL)
		return false;
	if (this->child_first == NULL)
	{ /* note: remember to linked with mslist in teNode */
		this->child_first = _new_child_list;
		_is_first_child = true;
		return true;
	}
	_is_first_child = false;

	shared_ptr<msNode> _tail = _new_child_list;
	while (_tail->next != NULL)
	{

#ifdef DEBUG_TRACK
		{
			stringstream _ss;
			_ss << "\t\t" << _tail->mat->to_str() << endl;
			if (_tail->next->next == NULL)
			{
				_ss << "\t\t" << _tail->next->mat->to_str() << endl;
			}
			util::track(_ss);
		}
#endif
		_tail = _tail->next;
	}
	/* bound with left */
	_new_child_list->prev = this->child_first->prev;
	this->child_first->prev->next = _new_child_list;
	/* bound with right */
	_tail->next = this->child_first;
	this->child_first->prev = _tail;

	this->child_first = _new_child_list;
#ifdef DEBUG_TRACK
	{
		stringstream _ss;
		_ss << "\t\tfirst_child=" << this->child_first << endl;
		if (this->child_first != NULL)
		{
			_ss << "\t\tfirst_prev=" << this->child_first->prev << endl;
			shared_ptr<msNode> _mstmp = this->child_first->next;
			while (_mstmp != NULL)
			{
				_ss << "\t\t" << _mstmp << " f=" << _mstmp->father << endl;
				_mstmp = _mstmp->next;
			}
		}
		util::track(_ss);
	}
#endif

	return true;
}

match *msNode::get_whole_match()
{
	if (this->path_match != NULL)
		return this->path_match;
	this->path_match = new match();
	this->path_match->add(this->mat);
	if (this->father != NULL)
	{
		this->path_match->add(this->father->get_whole_match());
#ifdef SPACE_LOG
		cout << "father not NULL " << this->whole_match_str() << endl;
#endif
	}

	return this->path_match;
}

string msNode::whole_match_str()
{
	match *_mat = this->get_whole_match();

	return _mat->to_str();
}

/*remove children: child_first,...,tail from the level list*/
shared_ptr<msNode> msNode::be_removed()
{
#ifdef DEBUG_TRACK
	{
		stringstream _ss;
		_ss << "in be_remove@" << endl;
		util::track(_ss);
	}
#endif
	if (this->child_first == NULL)
		return NULL;
	shared_ptr<msNode> _tail = this->child_first;
#ifdef MARK_DEL
	_tail->mark_del = true;
#endif
	while (_tail->next != NULL)
	{
		if (_tail->next->father.get() != this)
			break;
		_tail = _tail->next;
#ifdef MARK_DEL
		_tail->mark_del = true;
#endif
	}


	/// @bug This condition is added temporarily for debugging
	if (this->child_first->prev == NULL)
	{
		return NULL;
	}

	this->child_first->prev->next = _tail->next;
	if (_tail->next != NULL)
	{
		_tail->next->prev = this->child_first->prev;
	}


	this->child_first->prev = NULL;
	_tail->next = NULL;

	// auto to_be_removed = this->child_first;
	// this->child_first = NULL;
	// return to_be_removed;

	return this->child_first;

}

bool msNode::is_dedge(dEdge *_e)
{
	return this->mat->is_edge(_e);
}

shared_ptr<msNode> msNode::build_mslist(List<match> *_matlist, shared_ptr<msNode> _father)
{
	_matlist->reset();

	// shared_ptr<msNode> _mlist = new msNode(_father, NULL, _matlist->next());
	shared_ptr<msNode> _mlist = make_shared<msNode>(_father, nullptr, _matlist->next());
	shared_ptr<msNode> p_tmp = _mlist;
	while (_matlist->hasnext())
	{
		// p_tmp->next = new msNode(_father, NULL, _matlist->next());
		p_tmp->next = make_shared<msNode>(_father, nullptr, _matlist->next());
		p_tmp->next->prev = p_tmp;
		p_tmp = p_tmp->next;
	}
	return _mlist;
}
