#include "TreeIterator.hpp"
#include "DecisionTree.hpp"

TreeIterator::TreeIterator(DecisionTreeNode *root)
{
	_root = root;

	if(root != NULL)
		_nodes.push_back(root);
}

DecisionTreeNode* TreeIterator::GetCurrent()
{
	return _nodes.front();
}

void TreeIterator::MoveNext()
{
	DecisionTreeNode *node = _nodes.front();
	_nodes.pop_front();

	if(node->nodeType == NodeType_Internal)
	{
		if(node->left != NULL)
			_nodes.push_back(node->left);
		if(node->right != NULL)
			_nodes.push_back(node->right);
	}
}

bool TreeIterator::IsDone()
{
	return _nodes.empty();
}