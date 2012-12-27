#ifndef SNODE_H
#define SNODE_H

#include <QString>

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

    int getNumParams() const;
    int isValue() const;
    QString asString() const;

    static const char* OpAsString(Op op);

    Op op;
    int param[3];
};

class SNodeStats {
public:
    SNodeStats();
    void reset();

    double bestScoreEver;
    double avgScore;
    double lastAvgScore;
    double bestIndividualScore;
    double bestIndividualScoreEver;
    int generation;
    int hits;
    int runs;
};

#endif // SNODECONSTANTS_H
