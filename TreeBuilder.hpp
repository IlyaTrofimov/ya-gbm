#ifndef _TREE_BUILDER_HPP_
#define _TREE_BUILDER_HPP_

class TaksData;
struct Point;
class DecisionTreeNode;

enum LeafRestrics
{
    LeafRestrics_None,
    LeafRestrics_Sqrt
};

DecisionTreeNode* BuildNormalTree(const TaskData& taskData, const vector<Point*>& points, int levels, vector<bool>& allowedFeatures, int leafRestriction);
DecisionTreeNode* BuildMatrixTree(const TaskData& taskData, const vector<Point*>& points, int levels, vector<bool>& allowedFeatures);
DecisionTreeNode* BuildFirstBestTree(const TaskData& taskData, const vector<Point*>& points, int leafsMax, vector<bool>& allowedFeatures, int leafRestriction);

void GetRandomSubset(vector<Serp>::iterator itBegin, vector<Serp>::iterator itEnd, double probability, vector<Point*> *subset);
void GetRandomSubspace(double part, vector<bool> *allowedFeatures);

double GetTreeError(const DecisionTree& tree);

#endif  /* TreeBuilder.hpp */
