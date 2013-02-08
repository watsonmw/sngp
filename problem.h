#ifndef PROBLEM_H
#define PROBLEM_H

#include <vector>

#include "snode.h"
#include "sortedarray.h"

/*
 * Sample GP test cases.
 */
class Problem
{
public:
    Problem();
    virtual ~Problem();

    int getNumInputs() { return _numInputs; }

    /*
     * Get the number of test cases.
     */
    int getNumFitnessCases() { return _testCases.size(); }

    /*
     * Get the inputs for the given test case
     */
    int* getInputs(int fitnessCase);

    std::vector<int>& getOutputs() { return _outputs; }

    /*
     * Get the expected output for the given test case
     */
    int getOutput(int fitnessCase) { return _outputs[fitnessCase]; }

    /*
     * Get the current set of results that were evaluate()d.
     */
    std::vector<int>& getTestCaseResults(int i) { return _testCaseResults[i]; }

    /*
     * Get the operators allowed for solving the
     * problem.
     */
    std::vector<SNode::Op>& getOps() { return _ops; }

    /*
     * Return true if an individual has hit the target
     * fitness (this individual is a solution to the problem).
     */
    virtual bool hitTargetFitness(
        const std::vector<int> &values) = 0;

    /*
     * Optimized inner loop for evaluating all test cases.
     * Don't let that virtual fool you! :)
     */
    virtual void evaluate(const std::vector<SNode> &nodes,
                          std::vector<int> &outFitness) = 0;
    virtual void evaluate(const std::vector<SNode> &nodes,
                          const SortedArray<int> &changedNodes,
                          std::vector<int> &outFitness) = 0;

    void initTestCaseResults(int numNodes);

protected:
    class TestCase {
    public:
        TestCase() : inputs(0) { }

        int* inputs; // each test case has _numInputs
                     // number of inputs.
    };

    typedef int(*EvalNodeFunc)(SNode::Op, int, int, int, const std::vector<int> &);
    typedef int(*CalcFitnessFunc)(int, int);

    template<EvalNodeFunc evalNode, CalcFitnessFunc calcFitness>
    void _evaluateAll(const std::vector<SNode> &nodes,
                      std::vector<int> &outFitness);

    template<EvalNodeFunc evalNode, CalcFitnessFunc calcFitness>
    void _evaluate(const std::vector<SNode>& nodes,
                   const SortedArray<int>& changedNodes,
                   std::vector<int>& outFitness);

    int _numInputs;
    std::vector<TestCase> _testCases;
    std::vector<int> _outputs;
    std::vector<std::vector<int> > _testCaseResults;
    std::vector<SNode::Op> _ops;
};

/*
 * The 6-mux problem.
 * Two address inputs select one of four data inputs.
 * The function set is {AND, OR, NOT, IF}.
 * Fitness evaluation is exhaustive over all inputs, with an
 * individuals fitness equal to the number of matches with
 * the expected results.
 */
class ProblemMultiplexer : public Problem {
public:
    ProblemMultiplexer();

    virtual bool hitTargetFitness(
        const std::vector<int>& values);

    virtual void evaluate(const std::vector<SNode> &nodes,
                          const SortedArray<int> &changedNodes,
                          std::vector<int> &outFitness);

    virtual void evaluate(const std::vector<SNode> &nodes,
                          std::vector<int> &outFitness);

protected:
    void init();
};

/*
 * Even parity problem.
 * For the given number of input select 1 if the number of
 * inputs with values set to 1 is even else select 0.
 * The function set is {AND, OR, NAND, NOR}.
 * Fitness evaluation is exhaustive over all inputs, with an
 * individuals fitness equal to the number of matches with
 * the expected results.
 */
class ProblemEvenParity : public Problem {
public:
    ProblemEvenParity(int inputs);

    virtual bool hitTargetFitness(
        const std::vector<int>& values);

    virtual void evaluate(const std::vector<SNode> &nodes,
                          const SortedArray<int> &changedNodes,
                          std::vector<int> &outFitness);

    virtual void evaluate(const std::vector<SNode> &nodes,
                          std::vector<int> &outFitness);

protected:
    void init();
};

/*
 * Sample symbolic regression problem:
 *      4x^4 – 3x^3 + 2x^2 -x
 *
 * The fitness evaluation is done on integers ranging from
 * -16 to 16, with an individuals fitness equal to
 * difference between it's value and the expected value.
 * Differences are always expressed as negative values.
 *
 * The function set is {ADD, SUB, MULT, DIV},
 */
class ProblemSymbolicRegression : public Problem {
public:
    ProblemSymbolicRegression();

    virtual bool hitTargetFitness(
        const std::vector<int>& values);

    virtual void evaluate(const std::vector<SNode> &nodes,
                          const SortedArray<int> &changedNodes,
                          std::vector<int> &outFitness);

    virtual void evaluate(const std::vector<SNode> &nodes,
                          std::vector<int> &outFitness);

protected:
    void init();
};

#endif // PROBLEM_H
