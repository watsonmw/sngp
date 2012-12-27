#include "snode.h"

#include <QTextStream>

SNode::SNode()
  : op(NoOp)
{
    param[0] = 0;
    param[1] = 0;
    param[2] = 0;
}

int SNode::getNumParams() const
{
    switch(op) {
    case NumOps:
    case NoOp:
    case InputOp:
        return 0;
    case ValOp:
    case NotOp:
    case YesOp:
        return 1;
    case AddOp:
    case SubOp:
    case MultOp:
    case DivOp:
    case OrOp:
    case NorOp:
    case AndOp:
    case NandOp:
    case GreaterOp:
    case LessOp:
    case EqualOp:
        return 2;
    case IfOp:
        return 3;
    default:
        return 0;
    }
}

int SNode::isValue() const
{
    return (op == InputOp || op == ValOp);
}

QString SNode::asString() const
{
    QString str;

    QTextStream stream(&str);
    stream << OpAsString(op);
    int numParams = getNumParams();
    if (numParams) {
        stream << " (";
        for (int i = 0; i < getNumParams(); ++i) {
            stream << param[i];
            if (i < getNumParams() - 1) {
                stream << ", ";
            }
        }
        stream << ")";
    }
    return str;
}

const char* SNode::OpAsString(Op op) {
    switch(op) {
    case NoOp:
        return "Nop";
    case InputOp:
        return "Input";
    case ValOp:
        return "Value";
    case AddOp:
        return "Add";
    case SubOp:
        return "Sub";
    case MultOp:
        return "Mult";
    case DivOp:
        return "Div";
    case OrOp:
        return "Or";
    case NorOp:
        return "Nor";
    case AndOp:
        return "And";
    case NandOp:
        return "Nand";
    case YesOp:
        return "Yes";
    case NotOp:
        return "Not";
    case GreaterOp:
        return "Greater";
    case LessOp:
        return "Less";
    case EqualOp:
        return "Equal";
    case IfOp:
        return "If";
    case NumOps:
    default:
        return "MaxOps";
    }
}

SNodeStats::SNodeStats()
{
    reset();
}

void SNodeStats::reset()
{
    bestScoreEver = 0;
    avgScore = 0;
    lastAvgScore = 0;
    bestIndividualScore = 0;
    bestIndividualScoreEver = 0;
    generation = 0;
    hits = 0;
    runs = 0;
}

