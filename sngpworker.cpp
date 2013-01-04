#include "sngpworker.h"

#include <map>
#include <QVector>
#include <QTextStream>
#include <QtAlgorithms>
#include <QDateTime>

SNGPWorker::SNGPWorker()
  : _bRunning(false),
    _problem(NULL),
    _times(0),
    _maxGenerations(25000)
{
}

SNGPWorker::~SNGPWorker()
{
    // Exit the other thread before cleaning up
    pause();
    delete _problem;
}

void SNGPWorker::setProblem(Problem *problem)
{
    // Delete the old problem
    delete _problem;

    // Setup the eval engine according to the problem
    _problem = problem;
    _evalEngine.setNumInputs(_problem->getNumInputs());
    _evalEngine.setAvailableOps(_problem->getOps());
    _evalEngine.setSize(100);
    _testCaseResults.resize(_problem->getNumFitnessCases());
    _evalEngine.setNumInputs(_problem->getNumInputs());
    _fitness.resize(_evalEngine.getSize());

    // Initialize the result sets for each test case.
    for (int i = 0; i < _problem->getNumFitnessCases(); ++i) {
        std::vector<int>& results = _testCaseResults[i];
        results.resize(_evalEngine.getSize());
        int* inputs = _problem->getInputs(i);
        for (int j = 0; j < _problem->getNumInputs(); ++j) {
            results[j] = inputs[j];
        }
        for (size_t j = _problem->getNumInputs(); j < results.size(); ++j) {
            results[j] = 0;
        }
    }
}

void SNGPWorker::setNumTimesToRun(int times)
{
    _times = times;
}

void SNGPWorker::setNumMaxGenerations(int maxGenerations)
{
    _maxGenerations = maxGenerations;
}

void SNGPWorker::reset()
{
    QMutexLocker lock(&_mutex);
    _stats.reset();
    _evalEngine.init();
}

void SNGPWorker::pause()
{
    if (_bRunning) {
        _bRunning = false;
        wait();
    }
}

void SNGPWorker::resume()
{
    if (!_bRunning) {
        _bRunning = true;
        start();
    }
}

bool SNGPWorker::isRunning()
{
    return _bRunning;
}

void SNGPWorker::step()
{
    QMutexLocker lock(&_mutex);
    runGeneration();
}

void SNGPWorker::runGeneration()
{
    // Apply a new mutation or revert the previous
    if (_stats.generation == 0) {
        // Calculate fitness values for all test cases
        for (size_t i = 0; i < _fitness.size(); ++i) {
             _fitness[i] = 0.0;
        }

        _evalEngine.init();

        for (int i = 0; i < _problem->getNumFitnessCases(); ++i) {
            std::vector<int>& results = _testCaseResults[i];
            _evalEngine.evalAll(results);
            for (size_t j = 0; j < _fitness.size(); ++j) {
                _fitness[j] += _problem->getFitness(results[j], i);
            }
        }

        // Calculate total scores
        int totalScore = 0;
        int bestScore = _fitness[0];
        for (size_t i = 0; i < _fitness.size(); ++i) {
            totalScore += _fitness[i];
            if (_fitness[i] > bestScore) {
                bestScore = _fitness[i];
            }
        }

        // First time just record the stats, this handles the case
        // where the test is retuning negative values.
        _stats.lastAvgScore = totalScore;
        _stats.avgScore = totalScore;
        _stats.bestScoreEver = totalScore;
        _stats.bestIndividualScore = bestScore;
        _stats.bestIndividualScoreEver = bestScore;
    } else {
        if (_stats.avgScore < _stats.lastAvgScore) {
            _evalEngine.restore();
            _stats.avgScore = _stats.lastAvgScore;
        }
        _evalEngine.mutate();

        // Calculate fitness values for all test cases
        std::set<int>& changedNodes = _evalEngine.getChangedNodes();
        for (std::set<int>::iterator it = changedNodes.begin();
                        it != changedNodes.end(); ++it) {
             _fitness[*it] = 0;
        }

        for (int i = 0; i < _problem->getNumFitnessCases(); ++i) {
            std::vector<int>& results = _testCaseResults[i];
            int expectedOutput = _problem->getOutput(i);
            for (std::set<int>::iterator it = changedNodes.begin();
                        it != changedNodes.end(); ++it) {
                int j = *it;
                results[j] = _evalEngine.evalNode(j, results);
                _fitness[j] += _problem->getFitness(results[j], expectedOutput);
            }
        }
        _evalEngine.clearChanged();

        // Calculate total scores
        int64_t totalScore = 0;
        int bestScore = _fitness[0];
        for (size_t i = 0; i < _fitness.size(); ++i) {
            totalScore += _fitness[i];
            if (_fitness[i] > bestScore) {
                bestScore = _fitness[i];
            }
        }

        _stats.lastAvgScore = _stats.avgScore;
        _stats.avgScore = totalScore;
        if (_stats.bestScoreEver < totalScore) {
            _stats.bestScoreEver = totalScore;
        }
        _stats.bestIndividualScore = bestScore;
        if (_stats.bestIndividualScoreEver < bestScore) {
            _stats.bestIndividualScoreEver = bestScore;
        }
    }
    _stats.generation++;
}

QString SNGPWorker::getProgramAsText(int i)
{
    QString text;
    QTextStream stream(&text);
    QVector<int> indices;
    QVector<bool> toCheck(i+1, false);
    toCheck[i] = true;
    for (int j = i; j >= 0; -- j) {
        if (toCheck[j]) {
            SNode& node = _nodesCopy[j];
            indices.push_back(j);
            if (node.op != SNode::ValOp &&
                node.op != SNode::InputOp) {
                for (int k = 0; k < node.getNumParams(); ++k) {
                    toCheck[node.param[k]] = true;
                }
            }
        }
    }

    std::reverse(indices.begin(), indices.end());

    std::map<int, int> remapping;
    for (int j = 0; j < indices.size(); ++j) {
        remapping[indices[j]] = j;
    }

    for (int j = 0; j < indices.size(); ++j) {
        SNode& node = _nodesCopy[indices[j]];
        stream << j << " (" << indices[j] << "): ";
        stream << SNode::OpAsString(node.op);
        if (node.op == SNode::ValOp) {
            stream << " " << node.param[0];
        } else if (node.op == SNode::InputOp) {
            stream << " " << indices[j];
        } else {
            stream << " (";
            for (int k = 0; k < node.getNumParams(); ++k) {
                stream << remapping[node.param[k]];
                if (k < node.getNumParams() - 1) {
                    stream << ", ";
                }
            }
            stream << ")";
        }
        stream << endl;
    }
    return text;
}

void SNGPWorker::run()
{
//    qsrand(1); Sometimes I set to 1 for testing.
    qsrand((uint)QDateTime::currentMSecsSinceEpoch());

    _stats.startTimeMilliseconds = QDateTime::currentMSecsSinceEpoch();

    while (_bRunning) {
        QMutexLocker lock(&_mutex);
        runGeneration();
        bool doneRun = false;
        if (_problem->hitTargetFitness(_fitness)) {
            _stats.hits++;
            doneRun = true;
        } else if (_stats.generation >= _maxGenerations) {
            doneRun = true;
        }
        if (doneRun) {
            _stats.runs++;
            if (_times <= _stats.runs) {
                _bRunning = false;
            } else {
                _evalEngine.init();
                _stats.generation = 0;
            }
        }
    }

    _stats.timeTakenMilliseconds +=
        QDateTime::currentMSecsSinceEpoch() - _stats.startTimeMilliseconds;
}

const std::vector<SNode> &SNGPWorker::getNodes()
{
   if (!_bRunning) {
       _nodesCopy = _evalEngine.getNodes();
   }
   return _nodesCopy;
}

const std::vector<int> &SNGPWorker::getFitness()
{
   if (!_bRunning) {
       _fitnessCopy = _fitness;
   }
   return _fitnessCopy;
}
