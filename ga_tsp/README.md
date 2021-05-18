Travelling Salesman Problem is an NP-Hard problem, hence is solved using heuristic approaches. Here I haved use genetic algorithm approach to solve this, and used omp to parallelise it using threads.

The main steps involved were:
    • Initialize the city List. I have generated a 100-city list, and used the same input everywhere for easy result and optimization comparison.
    • Generate a set of “Population” randomly. These consists of random routes that are possible solutions.
    • Select pool for mating – The parents that will be crossed to form new possible solution.
    • Use elitism to constantly export best into the next generation. 
    • The rest of the population is filled using new children obtained from mating.
    • Mutation occurs in the new generation to increase randomness. 
    • This Mutated population is the next gen, and the process is repeated.


To parallelise it:
    • I initially create a thread pool, and assign a random population to everyone.
    • These threads run independently for a few generations.
    • Then they share their best solutions, and a new “Best Pool” is formed, that consists of best solutions from all threads. 
    • Each threads replaces its worst solutions with this Best Pool. 
    • Then they repeat the process.


The Parameters for comparison are the time vs final tour distance (lower is better)


Conclusion:
    • The threaded approach is a clear winner.
    • When using 2 threads, the result it not much better, but still the execution time is much reduced.
    • With 4 threads -- It not only runs fast, but gives better result.
    • This was possible since threads were pooling threads best results together.