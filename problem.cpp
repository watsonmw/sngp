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

int* Problem::getInputs(int fitnessCase)
{
    return _testCases[fitnessCase].inputs;
}

void Problem::initTestCaseResults(int numNodes)
{
    // Initialize the result sets for each test case.
    _testCaseResults.resize(getNumFitnessCases());
    for (int i = 0; i < getNumFitnessCases(); ++i) {
        std::vector<int>& results = getTestCaseResults(i);
        results.resize(numNodes);
        int* inputs = getInputs(i);
        for (int j = 0; j < getNumInputs(); ++j) {
            results[j] = inputs[j];
        }
        for (size_t j = getNumInputs(); j < results.size(); ++j) {
            results[j] = 0;
        }
    }
}

template<Problem::EvalNodeFunc evalNode,
         Problem::CalcFitnessFunc calcFitness>
void Problem::_evaluateAll(const std::vector<SNode>& nodes,
                           std::vector<int>& outFitness)
{
    int numTestCases = _testCases.size();
    int result = 0;

    for (size_t i = _numInputs; i < nodes.size(); ++i) {
        int fitness = 0;
        const SNode& node = nodes[i];
        SNode::Op op = node.op;
        int p1 = node.param[0];
        int p2 = node.param[1];
        int p3 = node.param[2];
        for (int j = 0; j < numTestCases; ++j) {
            std::vector<int>& results = _testCaseResults[j];
            int expectedOutput = getOutput(j);
            result = evalNode(op, p1, p2, p3, results);
            fitness += calcFitness(result, expectedOutput);
            results[i] = result;
        }
        outFitness[i] = fitness;
    }
}

int ProblemCalcFitness(int value, int expectedOutput)
{
    if (value == expectedOutput) {
        return 1;
    }
    return false;
}

template<Problem::EvalNodeFunc evalNode,
         Problem::CalcFitnessFunc calcFitness>
void Problem::_evaluate(const std::vector<SNode>& nodes,
                        const SortedArray<int>& changedNodes,
                        std::vector<int>& outFitness)
{
    int numTestCases = _testCases.size();
    int result = 0;

    // Calculate fitness values for all test cases
    int* nodeIndices = changedNodes.data();
    for (int k = 0; k < changedNodes.size(); ++k) {
        int j = nodeIndices[k];
        int fitness = 0;
        const SNode& node = nodes[j];
        SNode::Op op = node.op;
        int p1 = node.param[0];
        int p2 = node.param[1];
        int p3 = node.param[2];
        for (int i = 0; i < numTestCases; ++i) {
            std::vector<int>& results = _testCaseResults[i];
            int expectedOutput = getOutput(i);
            result = evalNode(op, p1, p2, p3, results);
            fitness += calcFitness(result, expectedOutput);
            results[j] = result;
        }
        outFitness[j] = fitness;
    }
}

ProblemMultiplexer::ProblemMultiplexer()
{
    init();
}

bool ProblemMultiplexer::hitTargetFitness(const std::vector<int> &values)
{
    for (size_t i = _numInputs; i < values.size(); ++i) {
        if (values[i] >= (1 << _numInputs)) {
            return true;
        }
    }
    return false;
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
        _outputs.push_back(t.inputs[((t.inputs[0] << 1) | t.inputs[1]) + 2]);
        _testCases.push_back(t);
    }
    _testCaseResults.resize(_testCases.size());
}

int ProblemMultiplexerEvalNode(SNode::Op op,
                               int p1, int p2, int p3,
                               const std::vector<int> &values)
{
    int val0 = values[p1];
    int val1 = values[p2];
    int val2 = values[p3];
    switch (op) {
    case SNode::NotOp:
        if (val0) {
            return 1;
        } else {
            return 0;
        }
    case SNode::OrOp:
        if (val0 || val1) {
            return 1;
        } else {
            return 0;
        }
    case SNode::AndOp:
        if (val0 && val1) {
            return 1;
        } else {
            return 0;
        }
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

void ProblemMultiplexer::evaluate(const std::vector<SNode>& nodes,
                                  std::vector<int>& outFitness)
{
    _evaluateAll<ProblemMultiplexerEvalNode,
                 ProblemCalcFitness>(nodes, outFitness);
}

void ProblemMultiplexer::evaluate(const std::vector<SNode>& nodes,
                                  const SortedArray<int>& changedNodes,
                                  std::vector<int>& outFitness)
{
    _evaluate<ProblemMultiplexerEvalNode,
              ProblemCalcFitness>(nodes, changedNodes, outFitness);
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

        _outputs.push_back(bitsSet & 1);
        _testCases.push_back(t);
    }
    _testCaseResults.resize(_testCases.size());
}

bool ProblemEvenParity::hitTargetFitness(const std::vector<int> &values)
{
    for (size_t i = _numInputs; i < values.size(); ++i) {
         if (values[i] >= (1 << _numInputs)) {
             return true;
         }
    }
    return false;
}

int ProblemEvenParityEvalNode(SNode::Op op,
                              int p1, int p2, int,
                              const std::vector<int> &values)
{
    int val0 = values[p1];
    int val1 = values[p2];
    switch (op) {
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
    default:
        // do nothing
        break;
    }
    return 0;
}

void ProblemEvenParity::evaluate(const std::vector<SNode>& nodes,
                                 std::vector<int>& outFitness)
{
    _evaluateAll<ProblemEvenParityEvalNode,
                 ProblemCalcFitness>(nodes, outFitness);
}

void ProblemEvenParity::evaluate(const std::vector<SNode>& nodes,
                                 const SortedArray<int>& changedNodes,
                                 std::vector<int>& outFitness)
{
    _evaluate<ProblemEvenParityEvalNode,
              ProblemCalcFitness>(nodes, changedNodes, outFitness);
}

ProblemSymbolicRegression::ProblemSymbolicRegression()
{
    init();
}

bool ProblemSymbolicRegression::hitTargetFitness(const std::vector<int> &values)
{
    for (size_t i = _numInputs; i < values.size(); ++i) {
        if (values[i] >= 0) {
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
    for (int i = 0; i < 10; ++i) {
        TestCase t;
        t.inputs = new int[_numInputs];
        t.inputs[0] = i;
        int x = i;
        int x2 = x * x;
        int x3 = x * x2;
        int x4 = x * x3;
        int r = (4 * x4) - (3 * x3) + (2 * x2) - x;
        _outputs.push_back(r);
        _testCases.push_back(t);
    }
    _testCaseResults.resize(_testCases.size());
}

int ProblemSymbolicRegressionGetFitness(int value, int expectedOutput)
{
    int i = value - expectedOutput;
    if (i > 0) {
        i = -i;
    }
    if (i < -1000000) {
        i = -1000000;
    }
    return i;
}

int ProblemSymbolicRegressionEvalNode(SNode::Op op,
                                      int p1, int p2, int,
                                      const std::vector<int> &values)
{
    int val0 = values[p1];
    int val1 = values[p2];
    switch (op) {
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
        break;
    default:
        // do nothing
        break;
    }
    return 0;
}

void ProblemSymbolicRegression::evaluate(const std::vector<SNode>& nodes,
                                 std::vector<int>& outFitness)
{
    _evaluateAll<ProblemSymbolicRegressionEvalNode,
                 ProblemSymbolicRegressionGetFitness> (nodes, outFitness);
}

void ProblemSymbolicRegression::evaluate(const std::vector<SNode>& nodes,
                                 const SortedArray<int>& changedNodes,
                                 std::vector<int>& outFitness)
{
    _evaluate<ProblemSymbolicRegressionEvalNode,
              ProblemSymbolicRegressionGetFitness>
                  (nodes, changedNodes, outFitness);
}
