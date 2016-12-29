#ifndef _DECISION_TREE_HPP
#define _DECISION_TREE_HPP

#include "General.hpp"
#include "TaskData.hpp"
#include "TreeIterator.hpp"
#include "Predicate.hpp"

//class Predicate;
//class PredicateStat;

double GetSeriesValue(const vector< pair<double, DecisionTree> >& series, const Point& point);

enum NodeType
{
    NodeType_Internal = 0,
    NodeType_Leaf = 1
};

class DecisionTreeNode
{
public:
    DecisionTreeNode() {}

    bool IsPreLeaf() 
    {
        bool leftIsLeaf = (left != NULL) && (left->nodeType == NodeType_Leaf);
        bool rightIsLeaf = (right != NULL) && (right->nodeType == NodeType_Leaf);

        return leftIsLeaf && rightIsLeaf;
    }

    PredicateStat stat;
    Predicate predicate;
    DecisionTreeNode *left;
    DecisionTreeNode *right;
    NodeType nodeType;
    double value;
    double error;
    int index;
};

class DecisionTree
{
public:
	DecisionTree();
	DecisionTree(DecisionTreeNode *root) : _root(root) {}
	DecisionTree(const DecisionTree& tree);
    double GetValue(const Point& point) const;
    DecisionTreeNode* GetLeaf(const Point& point) const;
    DecisionTreeNode* GetRoot() const;
	vector<DecisionTreeNode*> GetLowestInternalNodes() const;
    vector<DecisionTreeNode*> GetLowestNodes() const;
	TreeIterator GetIterator() const;
    int GetNodesCount() const;
    int GetTerminalNodesCount() const;
	int GetLeafsCount() const;
	double GetError() const;
	void Collapse(DecisionTreeNode *node);
	bool EqualStructure(const DecisionTree& tree) const;
    string ToString() const;
    int GetHeight() const;
    void SetLeafsIndexes();

    void operator=(const DecisionTree& tree);
    ~DecisionTree();

private:
    int GetHeight(DecisionTreeNode* node) const;

    DecisionTreeNode *_root;
};


double GetPredicateError(const Predicate& predicate, const vector<Point*>& points, double *avg, double *avg_left, double *avg_right);
void SplitPoints(Predicate predicate, vector<Point*> points, vector<Point*> *leftPoints, vector<Point*> *rightPoints);
DecisionTree PruneTree(const DecisionTree& tree, double alpha);

Predicate FindBestPredicate(const vector<Point*>& points, const TaskData& taskData, const vector<bool>& allowedFeatures, PredicateStat *stat);
Predicate FindBestPredicateFast(const vector<Point*>& points, const TaskData& taskData, const vector<bool>& allowedFeatures, PredicateStat *stat);
Predicate FindBestPredicateFull(const vector<Point*>& points, const TaskData& taskData, const vector<bool>& allowedFeatures, PredicateStat *stat);

Predicate FindBestPredicate(const vector< vector<Point*> >& points, const TaskData& taskData, const vector<bool>& allowedFeatures, PredicateStat *bestStat);

#endif /* _DECISION_TREE_HPP */
