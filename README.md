#### Parallel Implementation of evolutionary approaches to the Travelling Salesman Problem.
---
TSP is an NP-Hard problem, hence is solved using heuristic approaches. Here I haved parallelized the recent Red Deer Algorithm (RDA), and its older cousin the traditional Genetic Chromosome algorithm in C++, using omp for easy threading.

The RDA refernce is taken from the original paper:
>Fathollahi-Fard, Amir & Hajiaghaei-Keshteli, Mostafa. (2016). Red deer algorithm (RDA); a new optimization algorithm inspired by red deer's mating. 