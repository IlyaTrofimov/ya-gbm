#ifndef _TREE_ITERATOR_HPP
#define _TREE_ITERATOR_HPP

#include <deque>

class DecisionTreeNode;
using std::deque;

class TreeIterator
{
public:
	TreeIterator(DecisionTreeNode *root);
	DecisionTreeNode* GetCurrent();
	void MoveNext();
	bool IsDone();

private:
	DecisionTreeNode* _root;
	deque<DecisionTreeNode*> _nodes;
};

#endif /* TreeIterator.hpp */