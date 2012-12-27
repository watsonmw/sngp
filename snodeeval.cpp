#include "snodeeval.h"

#include <stdlib.h>

SNodeEval::SNodeEval()
  : _numInputs(0),
    _size(0),
    _oldNodeIndex(0)
{
}

SNodeEval::~SNodeEval()
{
}

void SNodeEval::evalAll(std::vector<int> &values)
{
    for (size_t i = _numInputs; i < _nodes.size(); ++i) {
        values[i] = evalNode(i, values);
    }
}

void SNodeEval::evalChanged(std::vector<int>& values)
{
    for (std::set<int>::iterator it = _changedNodes.begin();
         it != _changedNodes.end(); ++it)
    {
        int i = *it;
        values[i] = evalNode(i, values);
    }
}

void SNodeEval::clearChanged()
{
    _changedNodes.clear();
}

int SNodeEval::evalNode(int i, std::vector<int>& values)
{
    SNode& node = _nodes[i];

    switch (node.op) {
    case SNode::NoOp:
        return 0;
    case SNode::InputOp:
        return values[i];
    case SNode::ValOp:
        return node.param[0];
    default:
        // do nothing
        break;
    }

    // One ref operations
    int val0 = values[node.param[0]];
    switch (node.op) {
    case SNode::YesOp:
        if (val0) {
            return 1;
        } else {
            return 0;
        }
    case SNode::NotOp:
        if (val0) {
            return 1;
        } else {
            return 0;
        }
    default:
        // do nothing
        break;
    }

    // Two ref operations
    int val1 = values[node.param[1]];
    switch (node.op) {
    case SNode::AddOp:
        return val0 + val0;
    case SNode::SubOp:
        return val1 - val0;
    case SNode::MultOp:
        return val1 * val0;
    case SNode::DivOp:
        if (val0) {
            return val1/val0;
        } else {
            return 0;
        }
    case SNode::OrOp:
        if (val0 || val1) {
            return 1;
        } else {
            return 0;
        }
    case SNode::NorOp:
        if (val0 || val1) {
            return 0;
        } else {
            return 1;
        }
    case SNode::AndOp:
        if (val0 && val1) {
            return 1;
        } else {
            return 0;
        }
    case SNode::NandOp:
        if (val0 && val1) {
            return 0;
        } else {
            return 1;
        }
    case SNode::GreaterOp:
        if (val0 > val1) {
            return 1;
        } else {
            return 0;
        }
    case SNode::LessOp:
        if (val0 < val1) {
            return 1;
        } else {
            return 0;
        }
    case SNode::EqualOp:
        if (val0 == val1) {
            return 1;
        } else {
            return 0;
        }
    default:
        // do nothing
        break;
    }

    // Three ref operations
    int val2 = values[node.param[2]];
    switch(node.op) {
    case SNode::IfOp:
        if (val0) {
            return val1;
        } else {
            return val2;
        }
    default:
        // do nothing
        break;
    }

    return 0;
}

void SNodeEval::mutate()
{
    int nodeIndex = _numInputs + (rand() % (_size - _numInputs));
    _oldNode = _nodes[nodeIndex];
    _oldNodeIndex = nodeIndex;
    smut(nodeIndex);
}

void SNodeEval::restore()
{
    SNode& currentNode = _nodes[_oldNodeIndex];
    for(int i = 0; i < _oldNode.getNumParams(); ++i) {
        int newLink = _oldNode.param[i];
        int oldLink = currentNode.param[i];
        if (oldLink != newLink) {
            _nodes[_oldNodeIndex] = _oldNode;
            switchLink(_oldNodeIndex, oldLink, newLink);
            markChanged(_oldNodeIndex);
            return;
        }
    }
}

void SNodeEval::smut(int i)
{
    SNode& node = _nodes[i];

    if (i > 1) {
        if (node.op == SNode::ValOp) {
            node.param[0] = rand() % 1001;
        } else {
            if (node.getNumParams()) {
                int j = rand() % node.getNumParams();
                int oldLink = node.param[j];
                int newLink = rand() % i;
                node.param[j] = newLink;
                switchLink(i, oldLink, newLink);
                markChanged(i);
            }
        }
    }
}

void SNodeEval::switchLink(int i, int oldLink, int newLink)
{
    if (oldLink == newLink) {
        return;
    }

    if (oldLink >= _numInputs) {
        std::multiset<int>& link = _nodeLinks[oldLink];
        std::multiset<int>::iterator it = link.find(i);
        if (it  != link.end()) {
            link.erase(it);
        }
    }

    if (newLink >= _numInputs) {
        std::multiset<int>& link = _nodeLinks[newLink];
        link.insert(i);
    }
}

void SNodeEval::markChanged(int index)
{
    std::pair<std::set<int>::iterator,bool> ret = _changedNodes.insert(index);
    if (ret.second) {
        std::multiset<int>& links = _nodeLinks[index];
        for (std::multiset<int>::iterator it = links.begin(); it != links.end(); ++it) {
            markChanged(*it);
        }
    }
}

void SNodeEval::generateLinks()
{
    for (int i = _numInputs; i < _size; ++i) {
        std::multiset<int>& links = _nodeLinks[i];
        links.clear();
    }
    for (int i = _numInputs; i < _size; ++i) {
        SNode& node = _nodes[i];
        for (int j = 0; j < node.getNumParams(); ++j) {
            int k = node.param[j];
            std::multiset<int>& links = _nodeLinks[k];
            if (i >= _numInputs) {
                links.insert(i);
            }
        }
    }
}

bool SNodeEval::verifyLinksExist()
{
    // Used for debugging purposes only
    for (int i = _numInputs; i < _size; ++i) {
        std::multiset<int>& links = _nodeLinks[i];
        for (std::multiset<int>::iterator it = links.begin();
             it != links.end(); ++it)
        {
            bool bFound = false;
            int j = *it;
            SNode& node  = _nodes[j];
            for (int k = 0; k < node.getNumParams(); ++k) {
                if (node.param[k] == i) {
                    bFound = true;
                    break;
                }
            }
            if (!bFound) {
                return false;
            }
        }
    }
    return true;
}

bool SNodeEval::verifyAllLinks()
{
    // Used for debugging purposes only
    std::vector<std::multiset<int> > nodeLinks(_size);
    for (int i = _numInputs; i < _size; ++i) {
        SNode& node = _nodes[i];
        for (int j = 0; j < node.getNumParams(); ++j) {
            int k = node.param[j];
            std::multiset<int>& links = nodeLinks[k];
            if (i >= _numInputs) {
                links.insert(i);
            }
        }
    }

    bool bNotEqual = false;
    for (int i = _numInputs; i < _size; ++i) {
        std::multiset<int>& linksReal = nodeLinks[i];
        std::multiset<int>& linksCached = _nodeLinks[i];
        for (std::multiset<int>::iterator it = linksCached.begin();
             it != linksCached.end(); ++it)
        {
            int j = *it;
            std::multiset<int>::iterator j_it = linksReal.find(j);
            if (j_it  == linksReal.end()) {
                bNotEqual = true;
            } else {
                linksReal.erase(j_it);
            }
        }
        if (linksReal.size() > 0) {
            bNotEqual = true;
        }
    }

    return !bNotEqual;
}

void SNodeEval::randomize(int i)
{
    SNode& node = _nodes[i];

    int val = rand() % _ops.size();
    node.op = _ops[val];

    if (i > 1) {
        if (node.op == SNode::ValOp) {
            node.param[0] = rand() % 1001;
            node.param[1] = 0;
            node.param[2] = 0;
        } else {
            for (int j = 0; j < node.getNumParams(); ++j) {
                node.param[j] = rand() % i;
            }
        }
    } else {
        node.param[0] = 0;
        node.param[1] = 0;
        node.param[2] = 0;
    }
}

void SNodeEval::init()
{
    _nodes.resize(_size);
    _nodeLinks.resize(_size);

    for (int i = 0; i < _numInputs; ++i) {
        _nodes[i].op = SNode::InputOp;
    }

    for (int i = _numInputs; i < _size; ++i) {
        randomize(i);
    }

    generateLinks();
}

void SNodeEval::setAvailableOps(const std::vector<SNode::Op> &ops)
{
    _ops = ops;
}

