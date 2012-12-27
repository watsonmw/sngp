#include "sngpworker.h"

#include <map>
#include <QVector>
#include <QTextStream>
#include <QtAlgorithms>

SNGPWorker::SNGPWorker()
  : _bRunning(false),
    _problem(NULL),
    _times(0)
{
}

SNGPWorker::~SNGPWorker()
{
    pause();
    delete _problem;
}

void SNGPWorker::setProblem(Problem *problem)
{
    delete _problem;
    _problem = problem;
    _nodeEval.setNumInputs(_problem->getNumInputs());
    _nodeEval.setAvailableOps(_problem->getOps());
    _nodeEval.setSize(100);
    _testCaseResults.resize(_problem->getNumFitnessCases());
    _nodeEval.init();
    _nodeEval.setNumInputs(_problem->getNumInputs());
    _fitness.resize(_nodeEval.getSize());

    // Initialize the result sets for each test case.
    for (int i = 0; i < _problem->getNumFitnessCases(); ++i) {
        std::vector<int>& results = _testCaseResults[i];
        results.resize(_nodeEval.getSize());
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

void SNGPWorker::reset()
{
    QMutexLocker lock(&_mutex);
    _stats.reset();
    _nodeEval.init();
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

        for (int i = 0; i < _problem->getNumFitnessCases(); ++i) {
            std::vector<int>& results = _testCaseResults[i];
            _nodeEval.evalAll(results);
            for (size_t j = 0; j < _fitness.size(); ++j) {
                _fitness[j] += _problem->getFitness(results[j], i);
            }
        }

        // Calculate total scores
        double totalScore = 0.0;
        double bestScore = _fitness[0];
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
            _nodeEval.restore();
            _stats.avgScore = _stats.lastAvgScore;
        }
        _nodeEval.mutate();

        // Calculate fitness values for all test cases
        for (size_t i = 0; i < _fitness.size(); ++i) {
             _fitness[i] = 0.0;
        }

        for (int i = 0; i < _problem->getNumFitnessCases(); ++i) {
            std::vector<int>& results = _testCaseResults[i];
            _nodeEval.evalChanged(results);
            for (size_t j = 0; j < _fitness.size(); ++j) {
                _fitness[j] += _problem->getFitness(results[j], i);
            }
        }
        _nodeEval.clearChanged();

        // Calculate total scores
        double totalScore = 0.0;
        double bestScore = _fitness[0];
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
    srand(time(NULL));

    while (_bRunning) {
        QMutexLocker lock(&_mutex);
        runGeneration();
        bool doneRun = false;
        if (_problem->hitTargetFitness(_fitness)) {
            _stats.hits++;
            doneRun = true;
        } else if (_stats.generation >= 25000) {
            doneRun = true;
        }
        if (doneRun) {
            _stats.runs++;
            if (_times <= _stats.runs) {
                _bRunning = false;
            } else {
                _nodeEval.init();
                _stats.generation = 0;
            }
        }
    }
}

const std::vector<SNode> &SNGPWorker::getNodes()
{
   if (!_bRunning) {
       _nodesCopy = _nodeEval.getNodes();
   }
   return _nodesCopy;
}

const std::vector<double> &SNGPWorker::getFitness()
{
   if (!_bRunning) {
       _fitnessCopy = _fitness;
   }
   return _fitnessCopy;
}
