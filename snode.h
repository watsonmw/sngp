#ifndef SNODE_H
#define SNODE_H

#include <QString>

/*
 * Lightweight class to represent a GP node.
 */
class SNode
{
public:
    SNode();

    enum Op {
        NoOp,
        InputOp,
        ValOp,
        AddOp,
        SubOp,
        MultOp,
        DivOp,
        OrOp,
        NorOp,
        AndOp,
        NandOp,
        YesOp,
        NotOp,
        GreaterOp,
        LessOp,
        EqualOp,
        IfOp,
        NumOps
    };

    /*
     * Get the number of used params by the current operation
     */
    int getNumParams() const;

    /*
     * Return true if this node is a value type (InputOp/ValueOp)
     */
    bool isValue() const;

    /*
     * Get the node as a human readable string
     */
    QString asString() const;

    /*
     * Get the given Op as a human readable string
     */
    static const char* OpAsString(Op op);

    Op op;
    int param[3];
};

/*
 * Single node GP stats
 */
class SNodeStats {
public:
    SNodeStats();

    /*
     * Reset values (to zero).
     */
    void reset();

    // Best score during the run.
    int64_t bestScoreEver;

    // Average score of all individuals for the current generation
    int64_t avgScore;

    // Average score of all individuals for the previous generation
    int64_t lastAvgScore;

    // Best score for an individual for the current generation
    int64_t bestIndividualScore;

    // Best score ever for an individual during the run
    int64_t bestIndividualScoreEver;

    // Current generation
    int generation;

    // Start time in milliseconds since the epoch
    int64_t startTimeMilliseconds;

    // Time take in milliseconds once the run has completed, if zero the
    // run has not completed yet.
    int64_t timeTakenMilliseconds;

    // Number of runs that found a solutions
    int hits;

    // Number of runs
    int runs;
};

#endif // SNODECONSTANTS_H
