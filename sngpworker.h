#ifndef SNGPWORKER_H
#define SNGPWORKER_H

#include <QThread>
#include <QMutex>

#include <vector>

#include "snode.h"
#include "snodeeval.h"
#include "problem.h"

class SNGPWorker : public QThread
{
    Q_OBJECT
public:
    SNGPWorker();
    virtual ~SNGPWorker();

    void setProblem(Problem* problem);
    void setNumTimesToRun(int times);
    void reset();
    void pause();
    void resume();
    bool isRunning();
    void step();
    const SNodeStats& getStats() { return _stats; }
    const std::vector<SNode>& getNodes();
    const std::vector<double>& getFitness();

    void runGeneration();

    QString getProgramAsText(int i);


private:
    void run();

    int getFitness(const std::vector<int> &values, int target);

    bool _bRunning;
    SNodeEval _nodeEval;
    std::vector<SNode> _nodesCopy;
    std::vector<double> _fitness;
    std::vector<double> _fitnessCopy;
    std::vector<std::vector<int> > _testCaseResults;
    QMutex _mutex;
    SNodeStats _stats;
    Problem* _problem;
    int _times;
};

#endif // SNGPWORKER_H
