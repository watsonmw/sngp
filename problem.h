#ifndef PROBLEM_H
#define PROBLEM_H

#include <vector>

#include "snode.h"

/*
 * Sample GP test cases.
 */
class Problem
{
public:
    Problem();
    virtual ~Problem();

    virtual int getNumInputs();

    /*
     * Get the number of test cases.
     */
    virtual int getNumFitnessCases();

    /*
     * Get the inputs for the given test case
     */
    virtual int* getInputs(int fitnessCase);

    /*
     * Get the expected output for the given test case
     */
    virtual int getOutput(int fitnessCase);

    /*
     * Get the operators allowed for solving the
     * problem.
     */
    virtual std::vector<SNode::Op>& getOps() { return _ops; }

    /*
     * Return true if an individual has hit the target
     * fitness (this individual is a solution to the problem).
     */
    virtual bool hitTargetFitness(
        const std::vector<double> &values) = 0;

    /*
     * Get the fitness value for the given output and testcase.
     */
    virtual double getFitness(int value, int testCase) = 0;


protected:
    class TestCase {
    public:
        TestCase() : inputs(0), output(0) { }

        int* inputs; // each test case has _numInputs
                     // number of inputs.
        int output;
    };

    int _numInputs;
    std::vector<TestCase> _testCases;
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
        const std::vector<double>& values);

    virtual double getFitness(int value, int testCase);

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
        const std::vector<double>& values);

    virtual double getFitness(int value, int testCase);

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
        const std::vector<double>& values);

    virtual double getFitness(int value, int testCase);

protected:
    void init();
};

#endif // PROBLEM_H
