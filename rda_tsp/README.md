Travelling Salesman Problem is an NP-Hard problem, hence is solved using heuristic approaches. Here I haved use genetic the recent Red Deer Algorithm (RDA) to solve this, and used omp to parallelise it using threads.

RDA Summary: 
    • Similar to other evolutionary algorithms, the RDA starts with a population of random solutions called Red Deer (RD). 
    • This set of solutions has been divided into two types: “male RDs” as the best solutions and “hinds” as the rest of solutions. Generally, roaring, fighting, and mating are the three main behaviors of RDs during the breeding season. First of all, male RDs roar strongly to show their power to other males and attract hinds. 
    • After Roaring a number of successful males are selected as the commanders based on their power (fitness of solutions). 
    • Another main part of algorithm is the fighting process between the commanders and the rest of males called stags. During this competition, the better solutions as the winners have been chosen again as the commanders to form the harems, which are a group of hinds. 
    • Based on the power of each commander, a number of hinds has been adopted to be in the harem. The greater the power of the commander, the more hinds in the harem. 
    • After generating the harems, an amazing mating behavior occurs. First of all, the commanders should mate with a number of hinds in the harem and a few in another harem to extend his territory. 
    • The stags can mate with the nearest hind without the limitation of harems. 
    • Regarding the evolutionary concepts in the RDA, a set of better solutions will be chosen as the next generation of algorithm by roulette wheel selection or tournament selection mechanisms. 
    • At the end, the stop condition of this algorithm based on the maximum number of iterations should be satisfied.

To use RDA on TSP, we need an encoding scheme that can represent our route list in the form of a deer. For this, 2-stage process called Random Key (RK) has been used. In the first step, random numbers between zero and one from a uniform distribution. In the second step, the uniform distribution is sorted after pairing it with city-indexes to generate a route. This route is used for tour and fitness calculations. The RK of this deer will be used for all the mathematical computations.


Main steps of Algo:
    • Initialize the city List. I have used the same city list as generated for GA for easy result and optimization comparison.
    • Generate a set of “Population” randomly. These consists of Rks.
    • Distribute Population into Males and Hinds according to fitness values.
    • Choose Top males as Commanders, and fight them with males to produce even better commanders.
    • Form Harems for each commanders.
    • Perform 3 kinds of Mating: 
        ◦ Commanders in their own harem
        ◦ Commanders in other harem.
        ◦ Rest of the stags with other hinds.
    • By Elistism, the males are passed down the generation unharmed.
    • The rest of the population is filled from the hinds and new Children. I have also added a mutation stage here to introduce more randomness, as the RK tend to saturate over generations. The “best-pick” method is then used to fill the population.
    • The Process is repeated to get next generation.


To parallelise it:
    • I initially create a thread pool, and assign a random population to everyone.
    • These threads run independently for a few generations.
    • Then they share their best solutions, and a new “Best Pool” is formed, that consists of best solutions from all threads. 
    • Each threads replaces its worst solutions with this Best Pool. 
    • Then they repeat the process.


The Parameters for comparison are the time vs final tour distance (lower is better)

Conclusion:
    • We can see that using a single thread, there is very less randomization, so RDA gets stuck on   3023-2800 values.
    • When using 2 threads, the result is not much better, but still the execution time is much reduced.
    • With 4 threads -- It not only runs fast, but gives better result (2603 in 100 gen vs 3023 in 500 gens in serial).
    • This was possible since threads were pooling threads best results together.