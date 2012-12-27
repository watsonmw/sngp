#ifndef PROBLEM_H
#define PROBLEM_H

#include <vector>

#include "snode.h"

class Problem
{
public:
    Problem();
    virtual ~Problem();

    virtual int getNumInputs();
    virtual int getNumFitnessCases();
    virtual int* getInputs(int fitnessCase);
    virtual int getOutput(int fitnessCase);
    virtual std::vector<SNode::Op>& getOps() { return _ops; }
    virtual bool hitTargetFitness(
        const std::vector<double> &values) = 0;

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

class ProblemMultiplexer : public Problem {
public:
    ProblemMultiplexer();

    virtual bool hitTargetFitness(
        const std::vector<double>& values);

    virtual double getFitness(int value, int testCase);

protected:
    void init();
};

class ProblemEvenParity : public Problem {
public:
    ProblemEvenParity(int inputs);

    virtual bool hitTargetFitness(
        const std::vector<double>& values);

    virtual double getFitness(int value, int testCase);

protected:
    void init();
};

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
