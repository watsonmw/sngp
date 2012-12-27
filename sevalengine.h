#ifndef SEVALENGINE_H
#define SEVALENGINE_H

#include "snode.h"
#include <vector>
#include <set>

/*
 * Stores and evaluates a graph of SNodes.
 *
 * Initialisation and GP mutation functions are also available.
 */
class SEvalEngine
{
public:
    SEvalEngine();
    virtual ~SEvalEngine();

    /*
     * Set the number of nodes.
     */
    void setSize(int size) { _size = size; }

    /*
     * Get the number of nodes.
     */
    int getSize() { return _size; }

    /*
     * Generates new set of nodes and resets any internal state
     */
    void init();

    /*
     * Set the number of nodes to act as inputs
     */
    void setNumInputs(int inputs) { _numInputs = inputs; }

    /*
     * Set the list of available ops used to generate the random
     * list of nodes.
     */
    void setAvailableOps(const std::vector<SNode::Op>& ops);

    const std::vector<SNode>& getNodes() { return _nodes; }

    /*
     * Evaluates all nodes.
     *
     * The param 'values' is the stored list of results from
     * the last time evalAll() or evalChanged() was called.
     * Must be the same number of values as there are nodes
     * (including inputs).
     */
    void evalAll(std::vector<int> &values);

    /*
     * Mutate a random node.
     */
    void mutate();

    /*
     * Restore the previous mutation.
     */
    void restore();

    /*
     * Clear list of changed nodes, should only call after
     * evalAll() or evalChanged().
     */
    void clearChanged();

    /*
     * Eval the nodes that have changed by mutate() and
     * restore() since the last time clearChanged() was called.
     * Also evaluates any nodes that depend on the values
     * returned by the nodes that changed.
     * The param 'values' is the stored list of results from
     * the last time evalAll() or evalChanged() was called.
     * Must be the same number of values as there are nodes
     * (including inputs).
     */
    void evalChanged(std::vector<int> &values);


private:
    /*
     * Useful during debugging.
     * Verify that all links in the link list are real.
     * Fast.
     */
    bool verifyLinksExist();

    /*
     * Useful during debugging.
     * Verify all links exist that should exist exist
     * and that there are no extra links.
     * Slow.
     */
    bool verifyAllLinks();

    /*
     * Generate a random node at 'i'.
     */
    void randomise(int i);

    /*
     * Mutate one of the links for the given node at
     * index 'i'.  The node operation stays the same.
     */
    void smut(int i);

    /*
     * Eval a single node.
     */
    int evalNode(int i, std::vector<int> &values);

    /*
     * Switch a link in the link list for the given node.
     */
    void switchLink(int i, int oldLink, int newLink);

    /*
     * Mark that a node has to be reevaluated.  Any nodes that
     * depend on that node also have to be reevaluated.
     */
    void markChanged(int index);

    /*
     * Generate all links
     */
    void generateLinks();

    int _numInputs;
    int _size;
    std::vector<SNode> _nodes;

    // Available operators
    std::vector<SNode::Op> _ops;

    // For each node, the set of nodes that refer back to that node
    typedef std::multiset<int> NodeLinks;
    typedef NodeLinks::iterator NodeLinksIterator;
    std::vector<NodeLinks> _nodeLinks;

    // Ordered list of nodes that were changed by smut() and/or restore()

    std::set<int> _changedNodes;

    // The last node that was changed by smut()
    SNode _oldNode;
    int _oldNodeIndex;
};

#endif // SEVALENGINE_H