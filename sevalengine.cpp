#include "sevalengine.h"

#include <stdlib.h>

static int Rand(int range)
{
    // Modding the value returned by rand() is not strictly
    // correct as it affects the distribution, by oversampling
    // the values returned by RAND_MAX % range.  The effect is
    // slight here as we only request small ranges from rand()
    // but worth noting in case you want a larger range.
#ifdef BETTER_RAND_DISTRIBUTION
    double f = (1.0 / (RAND_MAX + 1.0));
    return int(qrand() * f * range);
#else
    return qrand() % range;
#endif
}

SEvalEngine::SEvalEngine()
  : _numInputs(0),
    _size(0),
    _changedNodes(100),
    _oldNodeIndex(0)
{
}

SEvalEngine::~SEvalEngine()
{
}

void SEvalEngine::evalAll(std::vector<int> &values)
{
    for (size_t i = _numInputs; i < _nodes.size(); ++i) {
        values[i] = evalNode(i, values);
    }
}

void SEvalEngine::clearChanged()
{
    _changedNodes.clear();
}

int SEvalEngine::evalNode(int i, const std::vector<int> &values)
{
    SNode& node = _nodes[i];
    return evalNode(node, values);
}

int SEvalEngine::evalNode(const SNode &node, const std::vector<int> &values)
{
    switch (node.op) {
    case SNode::NoOp:
        return 0;
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

void SEvalEngine::mutate()
{
    int nodeIndex = _numInputs + Rand(_size - _numInputs);
    _oldNode = _nodes[nodeIndex];
    _oldNodeIndex = nodeIndex;
    smut(nodeIndex);
}

void SEvalEngine::restore()
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

void SEvalEngine::smut(int i)
{
    SNode& node = _nodes[i];

    if (i > 1) {
        if (node.op == SNode::ValOp) {
            node.param[0] = Rand(1001);
        } else {
            if (node.getNumParams()) {
                int it = i - 1;
                int j = Rand(node.getNumParams() * it);
                int jdiv = j / it;
                int jrem = j % it;
                int oldLink = node.param[jdiv];
                if (jrem >= oldLink) {
                    jrem++;
                }
                int newLink = jrem;
                node.param[jdiv] = newLink;
                switchLink(i, oldLink, newLink);
                markChanged(i);
            }
        }
    }
}

void SEvalEngine::switchLink(int i, int oldLink, int newLink)
{
    if (oldLink == newLink) {
        return;
    }

    if (oldLink >= _numInputs) {
        NodeLinks& link = _nodeLinks[oldLink];
        for (size_t j = 0; j < link.size(); ++j) {
            if (link[j] == i) {
                link.erase(link.begin() + j);
                break;
            }
        }
    }

    if (newLink >= _numInputs) {
        NodeLinks& link = _nodeLinks[newLink];
        link.push_back(i);
    }
}

void SEvalEngine::markChanged(int index)
{
    if (_changedNodes.add(index)) {
        NodeLinks& links = _nodeLinks[index];
        for (size_t i = 0; i < links.size(); ++i) {
            markChanged(links[i]);
        }
    }
}

void SEvalEngine::generateLinks()
{
    for (int i = _numInputs; i < _size; ++i) {
        NodeLinks& links = _nodeLinks[i];
        links.clear();
    }
    for (int i = _numInputs; i < _size; ++i) {
        SNode& node = _nodes[i];
        for (int j = 0; j < node.getNumParams(); ++j) {
            int k = node.param[j];
            NodeLinks& links = _nodeLinks[k];
            if (i >= _numInputs) {
                links.push_back(i);
            }
        }
    }
}

bool SEvalEngine::verifyLinksExist()
{
    // Used for debugging purposes only
    for (int i = _numInputs; i < _size; ++i) {
        NodeLinks& links = _nodeLinks[i];
        for (NodeLinksIterator it = links.begin();
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

bool SEvalEngine::verifyAllLinks()
{
    // Used for debugging purposes only
    std::vector<NodeLinks > nodeLinks(_size);
    for (int i = _numInputs; i < _size; ++i) {
        SNode& node = _nodes[i];
        for (int j = 0; j < node.getNumParams(); ++j) {
            int k = node.param[j];
            NodeLinks& links = nodeLinks[k];
            if (i >= _numInputs) {
                links.push_back(i);
            }
        }
    }

    bool bNotEqual = false;
    for (int i = _numInputs; i < _size; ++i) {
        NodeLinks& linksReal = nodeLinks[i];
        NodeLinks& linksCached = _nodeLinks[i];
        for (size_t j = 0; j < linksCached.size(); ++j) {
            int val = linksCached[j];
            size_t k = 0;
            for (; k < linksReal.size(); ++k) {
                if (linksReal[k] == val) {
                    linksReal.erase(linksReal.begin() + k);
                    break;
                }
            }
            if (k == linksReal.size()) {
                bNotEqual = true;
            }
        }
        if (linksReal.size() > 0) {
            bNotEqual = true;
        }
    }

    return !bNotEqual;
}

void SEvalEngine::randomise(int i)
{
    SNode& node = _nodes[i];

    int val = Rand(_ops.size());
    node.op = _ops[val];

    if (i > 1) {
        if (node.op == SNode::ValOp) {
            node.param[0] = Rand(1001);
            node.param[1] = 0;
            node.param[2] = 0;
        } else {
            for (int j = 0; j < node.getNumParams(); ++j) {
                node.param[j] = Rand(i);
            }
        }
    } else {
        node.param[0] = 0;
        node.param[1] = 0;
        node.param[2] = 0;
    }
}

void SEvalEngine::init()
{
    _nodes.resize(_size);
    _nodeLinks.resize(_size);

    for (int i = 0; i < _numInputs; ++i) {
        _nodes[i].op = SNode::InputOp;
    }

    for (int i = _numInputs; i < _size; ++i) {
        randomise(i);
    }

    generateLinks();
}

void SEvalEngine::setAvailableOps(const std::vector<SNode::Op> &ops)
{
    _ops = ops;
}

