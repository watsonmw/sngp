#ifndef SNODEEVAL_H
#define SNODEEVAL_H

#include "snode.h"
#include <vector>
#include <set>

class SNodeEval
{
public:
    SNodeEval();
    ~SNodeEval();

    void setSize(int size) { _size = size; }

    int getSize() { return _size; }

    void setNumInputs(int inputs) { _numInputs = inputs; }

    void setAvailableOps(const std::vector<SNode::Op>& ops);

    void init();

    const std::vector<SNode>& getNodes() { return _nodes; }

    void evalAll(std::vector<int> &values);

    void mutate();

    void restore();

    void clearChanged();

    void evalChanged(std::vector<int> &values);


private:
    bool verifyLinksExist();

    bool verifyAllLinks();

    void randomize(int i);

    void smut(int i);

    int evalNode(int i, std::vector<int> &values);

    void switchLink(int i, int oldLink, int newLink);

    void markChanged(int index);

    void generateLinks();

    int _numInputs;
    int _size;
    std::vector<SNode> _nodes;

    // Available operators
    std::vector<SNode::Op> _ops;

    // For each node, the set of nodes that refer to that node
    std::vector<std::multiset<int> > _nodeLinks;

    // Ordered list of nodes that were changed by smut() and/or restore()
    std::set<int> _changedNodes;

    // The last node that was changed by smut()
    SNode _oldNode;
    int _oldNodeIndex;
};

#endif // SNODEEVAL_H
