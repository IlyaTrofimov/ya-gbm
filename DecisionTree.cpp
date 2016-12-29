#include "DecisionTree.hpp"
#include "TreeIterator.hpp"
#include "General.hpp"
#include "TaskData.hpp"

#include <algorithm>
#include <limits>
#include <vector>
#include <cassert>
#include <cmath>

using std::vector;

DecisionTree:: DecisionTree()
{
    _root = NULL;
}

DecisionTreeNode* DecisionTree::GetRoot() const
{
    return _root;
}

DecisionTreeNode* CopyTree(DecisionTreeNode *node)
{
    DecisionTreeNode* copyNode = new DecisionTreeNode();
    *copyNode = *node;

	if(node->left != NULL)
	{
		copyNode->left = CopyTree(node->left);
	}
	if(node->right != NULL)
	{
		copyNode->right = CopyTree(node->right);
	}

    return copyNode;
}

DecisionTree::DecisionTree(const DecisionTree& tree)
{
	_root = CopyTree(tree.GetRoot());
}

void DecisionTree::operator=(const DecisionTree& tree)
{
    _root = CopyTree(tree.GetRoot());
}

double EvalTree(const Point& point, const DecisionTreeNode* node)
{
    /*for (int i = 0; i < 20; ++i)
    {
         cout << i << " " << point.present[i] << endl;
    }*/

    if (node->nodeType == NodeType_Leaf)
    {
        return node->value;
    }
    else if (point.present[node->predicate.GetFeatureId()])
    {
        if(node->predicate.GetValue(point))
            return EvalTree(point, node->left);
        else
            return EvalTree(point, node->right);
    }
    else
    {
        return (node->stat.left_count * EvalTree(point, node->left) + node->stat.right_count * EvalTree(point, node->right)) / node->stat.count; 
    }
}

double DecisionTree::GetValue(const Point& point) const
{
    return EvalTree(point, _root);

    //DecisionTreeNode *node = this->GetLeaf(point);    
    //return node->value;
}

DecisionTreeNode* DecisionTree::GetLeaf(const Point& point) const
{
    DecisionTreeNode *node = _root;

    while(node->nodeType == NodeType_Internal)
    {       
        if(node->predicate.GetValue(point))
            node = node->left;
        else
            node = node->right;
    }

    return node;
}

vector<DecisionTreeNode*> DecisionTree::GetLowestInternalNodes() const
{
	vector<DecisionTreeNode*> lowestInternalNodes;

	for(TreeIterator it = this->GetIterator(); !it.IsDone(); it.MoveNext())
		if(it.GetCurrent()->IsPreLeaf())
			lowestInternalNodes.push_back(it.GetCurrent());

	return lowestInternalNodes;
}

vector<DecisionTreeNode*> DecisionTree::GetLowestNodes() const
{
	vector<DecisionTreeNode*> lowestNodes;	

	for(TreeIterator it = this->GetIterator(); !it.IsDone(); it.MoveNext())
		if(it.GetCurrent()->left == NULL && it.GetCurrent()->right == NULL)
			lowestNodes.push_back(it.GetCurrent());	

	return lowestNodes;
}

bool DecisionTree::EqualStructure(const DecisionTree& tree) const
{
	bool isEqual = true;
	TreeIterator it1 = this->GetIterator();
	TreeIterator it2 = tree.GetIterator();

	while(!(it1.IsDone() || it2.IsDone()))
	{
		isEqual = (it1.GetCurrent()->predicate == it2.GetCurrent()->predicate);

		if(!isEqual)
			break;

		it1.MoveNext();
		it2.MoveNext();
	}

	isEqual = isEqual && (it1.IsDone() == it2.IsDone());

	return isEqual;
}

void DecisionTree::SetLeafsIndexes()
{
    int index = 0;

    for(TreeIterator it = this->GetIterator(); !it.IsDone(); it.MoveNext())
        if(it.GetCurrent()->nodeType == NodeType_Leaf)
        {
           it.GetCurrent()->index = index;
           ++index;
        }
}

TreeIterator DecisionTree::GetIterator() const
{ 
	return TreeIterator(_root);
}

int DecisionTree::GetTerminalNodesCount() const
{
	int terminalNodesCount = 0;

	for(TreeIterator it = this->GetIterator(); !it.IsDone(); it.MoveNext())
        if(it.GetCurrent()->nodeType == NodeType_Leaf)
        {
		    ++terminalNodesCount;
        }

	return terminalNodesCount;
}

int DecisionTree::GetNodesCount() const
{
	TreeIterator it = this->GetIterator();
	int nodesCount = 0;

	while(!it.IsDone())
	{
		++nodesCount;
		it.MoveNext();
	}

	return nodesCount;
}

int DecisionTree::GetLeafsCount() const
{
	TreeIterator it = this->GetIterator();
	int leafsCount = 0;

	while(!it.IsDone())
	{
		if(it.GetCurrent()->nodeType == NodeType_Leaf)
			++leafsCount;

		it.MoveNext();
	}

	return leafsCount;
}

double DecisionTree::GetError() const
{
	TreeIterator it = this->GetIterator();
	double error = 0;

	while(!it.IsDone())
	{
		if(it.GetCurrent()->nodeType == NodeType_Leaf)
			error += it.GetCurrent()->error;

		it.MoveNext();
	}

	return error;
}

void DecisionTree::Collapse(DecisionTreeNode *node)
{
	assert(node->nodeType != NodeType_Leaf);
    assert(node->left->nodeType == NodeType_Leaf);
    assert(node->right->nodeType == NodeType_Leaf);

	node->nodeType = NodeType_Leaf;	
	node->value = node->stat.avg;
	node->error = node->stat.error;
	node->left = NULL;
	node->right = NULL;
}

#include <queue>
#include <algorithm>
using std::queue;
using std::max;

int DecisionTree::GetHeight(DecisionTreeNode *node) const
{
    if(node->nodeType == NodeType_Leaf)
        return 1;
    else 
        return max(GetHeight(node->left), GetHeight(node->right)) + 1;
}

int DecisionTree::GetHeight() const
{
    return GetHeight(this->_root);
}

void PrintRow(DecisionTreeNode *node, int pad, string *image)
{
    image->append(pad, ' ');
    //image->append(1, '(');
    if(node->nodeType == NodeType_Internal) {
        image->append(node->predicate.ToString());

        char buffer[256];
        sprintf(buffer, ", %d)", node->stat.count);
        image->append(buffer);
    }
    else
    {
        char buffer[256];
        sprintf(buffer, "(%10.2f, %d)", node->value, node->stat.count);
        image->append(buffer);
    }
    //image->append(1, ')');
    //image->append(pad, '@');
}

string DecisionTree::ToString() const
{
    queue<DecisionTreeNode*> nodes;

    string image;
    int height = this->GetHeight();
    int level = 1;
    nodes.push(_root);

    while(level <= height)
    {
        int item_size = 1;
        int pad = item_size * (pow(2.0, height - 1) - pow(2.0, level - 1)) / (pow(2.0, level - 1) + 1);
        queue<DecisionTreeNode*> nextNodes;

        char buffer[256];
        sprintf(buffer, "%2d   ", level);
        image.append(buffer);

        while(nodes.size() > 0)
        {
            DecisionTreeNode *node = nodes.front();
            nodes.pop();

            if(node != NULL)
            {
                PrintRow(node, pad, &image);
                nextNodes.push(node->left);
                nextNodes.push(node->right);
            }
            else
            {
                image.append(pad, ' ');
                image.append(item_size, '-');
                nextNodes.push(NULL);
                nextNodes.push(NULL);
            }
        }

        nodes = nextNodes;
        image.append(1, '\n');
        ++level;
    }

    return image;
}

DecisionTree::~DecisionTree()
{
    TreeIterator it = this->GetIterator();

    while(!it.IsDone())
    {
        DecisionTreeNode *node = it.GetCurrent();
        it.MoveNext();
        delete node;
    }
}

void SplitPoints(Predicate predicate, vector<Point*> points, vector<Point*> *leftPoints, vector<Point*> *rightPoints)
{
    leftPoints->clear();
    rightPoints->clear();

    for(int i = 0; i < (int)points.size(); ++i)
    {
        if(predicate.GetValue(*points[i]))
            leftPoints->push_back(points[i]);
        else
            rightPoints->push_back(points[i]);
    }
}

DecisionTree PruneOneNode(const DecisionTree& tree)
{	
	vector<double> errors;
    int preLeafsCount = tree.GetLowestInternalNodes().size();
    vector<DecisionTreeNode*> nodes = tree.GetLowestInternalNodes();

	for(int i = 0; i < preLeafsCount; ++i)
	{
		errors.push_back(nodes[i]->stat.error - nodes[i]->stat.left_error - nodes[i]->stat.right_error);		
	}

	vector<double>::const_iterator it = std::min_element(errors.begin(), errors.end());
	int index = std::distance<vector<double>::const_iterator>(errors.begin(), it);

    DecisionTree prunedTree(tree);
    vector<DecisionTreeNode*> nodes2 = prunedTree.GetLowestInternalNodes();
    prunedTree.Collapse(nodes2[index]);
	
    return prunedTree;
}

double GetComplexity(const DecisionTree& tree, double alpha)
{
	//double alpha = 0.1;
    double mse_sum = 0;

    TreeIterator it = tree.GetIterator();

    while(!it.IsDone())
    {
        if(it.GetCurrent()->nodeType == NodeType_Leaf)
        {
            mse_sum += it.GetCurrent()->error;
        }

        it.MoveNext();
    }

	return mse_sum + alpha * tree.GetLeafsCount();
}

DecisionTree PruneTree(const DecisionTree& tree, double alpha)
{
	vector<DecisionTree> trees;
	trees.push_back(tree);

	DecisionTree prunedTree(tree);

	while(prunedTree.GetNodesCount() > 1)
	{
		int nodesCount = prunedTree.GetNodesCount();
		prunedTree = PruneOneNode(prunedTree);
		trees.push_back(prunedTree);
	}

	vector<double> complexities;

	for(int i = 0; i < (int)trees.size(); ++i)
	{
		double complexity = GetComplexity(trees[i], alpha);
		complexities.push_back(complexity);
	}

	vector<double>::iterator it = std::min_element(complexities.begin(), complexities.end());
	int index = std::distance(complexities.begin(), it);

	return trees[index];
}

double GetSeriesValue(const TreeSeries& series, const Point& point)
{
	double value = 0;

	for(int i = 0; i < (int)series.size(); ++i)
		value += series[i].first * series[i].second.GetValue(point);

	return value;
}
