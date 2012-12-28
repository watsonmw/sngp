#include "problem.h"

Problem::Problem()
  : _numInputs(0)
{
}

Problem::~Problem()
{
    for (size_t i = 0; i < _testCases.size(); ++i) {
        delete [] _testCases[i].inputs;
    }
}

int Problem::getNumInputs()
{
    return _numInputs;
}

int Problem::getNumFitnessCases()
{
    return _testCases.size();
}

int* Problem::getInputs(int fitnessCase)
{
    return _testCases[fitnessCase].inputs;
}

int Problem::getOutput(int fitnessCase)
{
    return _testCases[fitnessCase].output;
}

ProblemMultiplexer::ProblemMultiplexer()
{
    init();
}

bool ProblemMultiplexer::hitTargetFitness(const std::vector<double> &values)
{
    for (size_t i = 0; i < values.size(); ++i) {
        if (values[i] >= 63.9) {
            return true;
        }
    }
    return false;
}

double ProblemMultiplexer::getFitness(int value, int testCase)
{
    return (value == getOutput(testCase)) ? 1.0 : 0.0;
}

void ProblemMultiplexer::init()
{
    _ops.push_back(SNode::AndOp);
    _ops.push_back(SNode::OrOp);
    _ops.push_back(SNode::NotOp);
    _ops.push_back(SNode::IfOp);
    _numInputs = 6;
    for (int i = 0; i < 64; ++i) {
        TestCase t;
        t.inputs = new int[_numInputs];
        for (int j = 0; j < _numInputs; ++j) {
            t.inputs[j] = ((1 << (_numInputs - 1 - j)) & i) ? 1 : 0;
        }
        t.output = t.inputs[((t.inputs[0] << 1) | t.inputs[1]) + 2];
        _testCases.push_back(t);
    }
}

ProblemEvenParity::ProblemEvenParity(int inputs)
{
    _numInputs = inputs;
    init();
}

void ProblemEvenParity::init()
{
    _ops.push_back(SNode::AndOp);
    _ops.push_back(SNode::OrOp);
    _ops.push_back(SNode::NandOp);
    _ops.push_back(SNode::NorOp);

    for (int i = 0; i < (1 << _numInputs); ++i) {
        TestCase t;
        t.inputs = new int[_numInputs];
        int bitsSet = 0;
        for (int j = 0; j < _numInputs; ++j) {
            t.inputs[j] = ((1 << (_numInputs - 1 - j)) & i) ? 1 : 0;
            if (t.inputs[j]) bitsSet+= 1;
        }
        t.output = bitsSet & 1;
        _testCases.push_back(t);
    }
}

bool ProblemEvenParity::hitTargetFitness(const std::vector<double> &values)
{
    for (size_t i = 0; i < values.size(); ++i) {
         if (values[i] >= ((1 << _numInputs) - 0.1)) {
             return true;
         }
    }
    return false;
}

double ProblemEvenParity::getFitness(int value, int testCase)
{
    return (value == getOutput(testCase)) ? 1.0 : 0.0;
}

ProblemSymbolicRegression::ProblemSymbolicRegression()
{
    init();
}

bool ProblemSymbolicRegression::hitTargetFitness(const std::vector<double> &values)
{
    for (size_t i = 0; i < values.size(); ++i) {
        if (values[i] >= -0.1) {
            return true;
        }
    }
    return false;
}

void ProblemSymbolicRegression::init()
{
    _ops.push_back(SNode::AddOp);
    _ops.push_back(SNode::SubOp);
    _ops.push_back(SNode::MultOp);
    _ops.push_back(SNode::DivOp);

    _numInputs = 1;
    for (int i = -8; i < 8; ++i) {
        TestCase t;
        t.inputs = new int[_numInputs];
        t.inputs[0] = i;
        int x = i;
        int x2 = x * x;
        int x3 = x * x2;
        int x4 = x * x3;
        t.output = (4 * x4) - (3 * x3) + (2 * x2) - x;
        _testCases.push_back(t);
    }
}

double ProblemSymbolicRegression::getFitness(int value, int testCase)
{
    double i = value - getOutput(testCase);
    if (i > 0) {
        i = -i;
    }
    return i;
}

