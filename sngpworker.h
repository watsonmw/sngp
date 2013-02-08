#ifndef SNGPWORKER_H
#define SNGPWORKER_H

#include <QThread>
#include <QMutex>

#include <vector>

#include "snode.h"
#include "sevalengine.h"
#include "problem.h"

/*
 * Worker thread and interface to the Single Node GP engine.
 */
class SNGPWorker : public QThread
{
    Q_OBJECT
public:
    SNGPWorker();
    virtual ~SNGPWorker();

    /*
     * Set the problem.  Ownership is passed to this class.
     * Object is deleted next time setProblem is called.
     * Also called when SNGPWorker is destroyed.
     */
    void setProblem(Problem* problem);

    /*
     * Set the number of times to run the GP engine to
     * the termination condition.
     * Current run stats and solutions can be found in
     * getStats().
     */
    void setNumTimesToRun(int times);

    /*
     * Set the max number of generations to run the
     * GP engine.
     */
    void setNumMaxGenerations(int maxGenerations);

    /*
     * Reset the current stats.
     */
    void reset();

    /*
     * Pause the current run.
     */
    void pause();

    /*
     * Resume the current run, or start a new one (when
     * resume is first called or after a reset()).
     */
    void resume();

    /*
     * Return true if the engine is still running.
     */
    bool isRunning();

    /*
     * Run one generation.
     */
    void step();

    /*
     * Get the stats for the current gp run.
     */
    const SNodeStats& getStats() { return _stats; }

    /*
     * Get the list of nodes, only updated when stopped.
     */
    const std::vector<SNode>& getNodes();

    /*
     * Get the fitness of all individual nodes, only
     * update when stopped.
     */
    const std::vector<int> &getFitness();

    /*
     * Get a somewhat readable output of the
     * program for the given node.
     */
    QString getProgramAsText(int i);


private:
    /*
     * QThread::run override.
     */
    virtual void run();

    void runGeneration();

    void resetFitness();

    // True when running, false when stopped.
    // Note: that the thread may still be running, but it won't
    // muck with private member variables.
    bool _bRunning;

    // The GP evaluation engine
    SEvalEngine _evalEngine;

    // Copy of the nodes, updates only when stopped.
    std::vector<SNode> _nodesCopy;

    // Current fitness values
    std::vector<int> _fitness;

    // Copy of the fitness values, updates only when
    // stopped
    std::vector<int> _fitnessCopy;

    // Memory barrier and lock for communication between
    // UI and worker thread.
    QMutex _mutex;

    // Current stats, updated during execution.
    SNodeStats _stats;

    // The current problem set
    Problem* _problem;

    // Number of times to run to completion
    int _times;

    // The max number of generations to allow before
    // terminating the current run.
    int _maxGenerations;
};

#endif // SNGPWORKER_H
