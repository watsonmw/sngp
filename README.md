sngp
====

Implementation of Single Node Genetic Programming as described by "D. Jackson:
A New, Node-Focused Model for Genetic Programming"

Contains all the sample problems from the paper.

Build
=====

1. Download and install Qt 5.0 for your operating system from https://qt-project.org/downloads
2. Open the sngp.pro file in QtCreator.
3. It should build out of the box if Qt is installed correctly.

Details
=======

The UI allows you to run all the sample problems for the paper.  While running
the number of successful runs are recorded along with other useful stats.

The run can stopped at any time and the individual programs and their scores
examined.

Notes
=====

- There may be floating point or integer exceptions on some operating systems.
  You'll need to turn these off in a platform specific way.  I've only tested
  on OSX 10.8.2.

