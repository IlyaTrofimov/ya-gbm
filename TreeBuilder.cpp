#include <map>
#include <algorithm>

#include "General.hpp"
#include "DecisionTree.hpp"
#include "Predicate.hpp"

using std::map;

struct Split
{
    double errorDelta;
    DecisionTreeNode *Node;
    Predicate Predicate;
    PredicateStat PredicateStat;
    vector<Point*> LeftPoints;
    vector<Point*> RightPoints;
};

void SetLeaf(DecisionTreeNode *node, PredicateStat stat)
{
    node->stat = stat;
    node->value = stat.avg;
    node->error = stat.error;
    node->nodeType = NodeType_Leaf;
    node->left = NULL;
    node->right = NULL;
}

void ApplySplit(const Split& split)
{
    DecisionTreeNode *node = split.Node;
    PredicateStat stat = split.PredicateStat;
    Predicate predicate = split.Predicate;

    node->nodeType = NodeType_Internal;
    node->stat = stat;
    node->value = stat.avg;
    node->error = stat.error;
    node->predicate = predicate;

    node->left = new DecisionTreeNode();
    node->left->nodeType = NodeType_Leaf;
    node->left->value = stat.avg_left;
    node->left->error = stat.left_error;
    node->left->left = NULL;
    node->left->right = NULL;

    node->right = new DecisionTreeNode();
    node->right->nodeType = NodeType_Leaf;
    node->right->value = stat.avg_right;
    node->right->error = stat.right_error;
    node->right->left = NULL;
    node->right->right = NULL;
}

void FindBestSplit(DecisionTreeNode *node, vector<Point*> points, const TaskData& taskData, const vector<bool>& allowedFeatures, Split *split)
{
    PredicateStat stat;
    Predicate predicate = FindBestPredicate(points, taskData, allowedFeatures, &stat);

    split->errorDelta = stat.error - stat.left_error - stat.right_error;
    split->Node = node;
    split->Predicate = predicate;
    split->PredicateStat = stat;

    vector<Point*> leftPoints, rightPoints;
    SplitPoints(predicate, points, &(split->LeftPoints), &(split->RightPoints));
}

bool IsGoodSplit(const Split& split, int leafRestriction)
{
    if(split.PredicateStat.left_count < leafRestriction || split.PredicateStat.right_count < leafRestriction)
        return false;
    else
        return true;
}

DecisionTreeNode* BuildFirstBestTree(const TaskData& taskData, const vector<Point*>& points, int leafsMax, vector<bool>& allowedFeatures, int leafRestriction)
{
    vector<Split> splits(1);
    map<double, int> splitCandidates;
    DecisionTree *tree = new DecisionTree(new DecisionTreeNode());

    int fisrtSplitIdx = 0;
    FindBestSplit(tree->GetRoot(), points, taskData, allowedFeatures, &splits[fisrtSplitIdx]);

    if(IsGoodSplit(splits[fisrtSplitIdx], leafRestriction))
        splitCandidates.insert(make_pair(splits[fisrtSplitIdx].errorDelta, fisrtSplitIdx));

    SetLeaf(tree->GetRoot(), splits[fisrtSplitIdx].PredicateStat);

    while(tree->GetLeafsCount() < leafsMax && splitCandidates.size() > 0)
    {
        map<double, int>::iterator it = --splitCandidates.end();
        int splitIdx = it->second;
        splitCandidates.erase(it);

        ApplySplit(splits[splitIdx]); // node - внутр., дети - листы

        DecisionTreeNode *node = splits[splitIdx].Node;  // node - лист

        splits.resize(splits.size() + 2);
        int leftSplitIdx = splits.size() - 1;
        int rightSplitIdx = splits.size() - 2;

        FindBestSplit(node->left, splits[splitIdx].LeftPoints, taskData, allowedFeatures, &splits[leftSplitIdx]);
        FindBestSplit(node->right, splits[splitIdx].RightPoints, taskData, allowedFeatures, &splits[rightSplitIdx]);

        if(IsGoodSplit(splits[leftSplitIdx], leafRestriction))
            splitCandidates.insert(make_pair(splits[leftSplitIdx].errorDelta, leftSplitIdx));

        if(IsGoodSplit(splits[rightSplitIdx], leafRestriction))
            splitCandidates.insert(make_pair(splits[rightSplitIdx].errorDelta, rightSplitIdx));
    }

    return tree->GetRoot();
}

DecisionTreeNode* BuildNormalTree(const TaskData& taskData, const vector<Point*>& points, int levels, vector<bool>& allowedFeatures, int leafRestriction)
{
    DecisionTreeNode *node = new DecisionTreeNode();

    PredicateStat stat;
    //GetRandomSubspace(0.5, &allowedFeatures);
    node->predicate = FindBestPredicateFast(points, taskData, allowedFeatures, &stat);

    vector<Point*> leftPoints;
    vector<Point*> rightPoints;
    SplitPoints(node->predicate, points, &leftPoints, &rightPoints);

    node->stat = stat;
    node->value = stat.avg;
    node->error = stat.error;

    if(levels == 0 || (int)leftPoints.size() < leafRestriction || (int)rightPoints.size() < leafRestriction)
    {
        node->nodeType = NodeType_Leaf;
        node->left = NULL;
        node->right = NULL;
    }
    else if(levels == 1)
    {
        node->nodeType = NodeType_Internal;

        node->left = new DecisionTreeNode();
        node->left->nodeType = NodeType_Leaf;
        node->left->value = stat.avg_left;
        node->left->error = stat.left_error;
        node->left->left = NULL;
        node->left->right = NULL;

        node->right = new DecisionTreeNode();
        node->right->nodeType = NodeType_Leaf;
        node->right->value = stat.avg_right;
        node->right->error = stat.right_error;
        node->right->left = NULL;
        node->right->right = NULL;
    }
    else
    {
        node->nodeType = NodeType_Internal;

        node->left = BuildNormalTree(taskData, leftPoints, levels - 1, allowedFeatures, leafRestriction);
        node->right = BuildNormalTree(taskData, rightPoints, levels - 1, allowedFeatures, leafRestriction);
    }

    return node;
}

DecisionTreeNode* CreateInternalNode(Predicate pred)
{
    DecisionTreeNode* node = new DecisionTreeNode();

    node->nodeType = NodeType_Internal;
    node->predicate = pred;
    node->left = NULL;
    node->right = NULL;

    return node;
}

DecisionTreeNode* CreateLeafNode(double value)
{
    DecisionTreeNode* node = new DecisionTreeNode();

    node->nodeType = NodeType_Leaf;
    node->value = value;
    node->left = NULL;
    node->right = NULL;

    return node;
}

double GetTreeError(const DecisionTree& tree)
{
    double error = 0;

    for(TreeIterator it = tree.GetIterator(); !it.IsDone(); it.MoveNext())
    {
        if(it.GetCurrent()->IsPreLeaf())
        {
            PredicateStat *stat = &(it.GetCurrent()->stat);
            error += stat->left_error + stat->right_error;
        }
    }

    return error;
}

 void AddInternalNodes(DecisionTree *tree, Predicate pred)
 {
    vector<DecisionTreeNode*> lowestNodes = tree->GetLowestNodes();

    for(int i = 0; i < (int)lowestNodes.size(); ++i)
    {
        lowestNodes[i]->left = CreateInternalNode(pred);
        lowestNodes[i]->right = CreateInternalNode(pred);
    }   
 }

DecisionTreeNode* BuildMatrixTree(const TaskData& taskData, const vector<Point*>& points, int levels, vector<bool>& allowedFeatures)
{     
    vector< vector<Point*> > split(1, points);
    vector< vector<Point*> > newSplit;

    int treeHeight = 0;
    DecisionTreeNode *root;
    DecisionTree *tree;

    while(treeHeight < levels)
    {
        PredicateStat stat;
        Predicate pred = FindBestPredicate(split, taskData, allowedFeatures, &stat);

        if(treeHeight == 0)
        {
            root = CreateInternalNode(pred);
            tree = new DecisionTree(root);
        }
        else
            AddInternalNodes(tree, pred);

        newSplit.clear();
        newSplit.resize(split.size() * 2);

        for(int i = 0; i < (int)split.size(); ++i)
        {
            SplitPoints(pred, split[i], &newSplit[i * 2], &newSplit[i * 2 + 1]);
        }

        split = newSplit;

        ++treeHeight;
    };

    vector<DecisionTreeNode*> nodes = tree->GetLowestNodes();

    for(int i = 0; i < (int)nodes.size(); ++i)
    {
        double value;

        value = GetAvg(newSplit[i * 2]); 
        nodes[i]->left = CreateLeafNode(value);

        value = GetAvg(newSplit[i * 2 + 1]); 
        nodes[i]->right = CreateLeafNode(value);
    }

    return root;
}

void GetRandomSubset(vector<Serp>::iterator itBegin, vector<Serp>::iterator itEnd, double probability, vector<Point*> *subset)
{
    subset->clear();

    if(probability > 0)
    {
        while(subset->size() == 0)
        {
            for(SerpIterator it = itBegin; it != itEnd; ++it)
                for(int i = 0; i < (int)it->size(); ++i)
                {
                    if(!it->at(i).disabled && !it->at(i).totalDisabled)
                        if(((rand() % 1024) / (double)1024) < probability)
                            subset->push_back(&it->at(i));
                }
        }
    }
}

void GetRandomSubspace(double part, vector<bool> *allowedFeatures)
{
    std::fill(allowedFeatures->begin(), allowedFeatures->end(), false);

    if(part > 0)
    {
        while(std::count(allowedFeatures->begin(), allowedFeatures->end(), true) == 0)
        {
            for(int i = 0; i < (int)allowedFeatures->size(); ++i)
                    if((rand() % 1024) < part * 1024)
                        allowedFeatures->at(i) = true;
        }
    }
}