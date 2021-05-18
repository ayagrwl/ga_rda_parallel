#include "ga_parallel.hpp"
#include <omp.h>

const int SHARE_AFTER = 10;
const int NUM_THREADS = 4;
const double SHARE_PERCENT = 0.1;   // Each thread will share % of its population to the best

vector<Chromosome> bestPool;

int main(){
    srand(time(NULL));
    loadCities("./cities.txt");

    // cout<<"Initially: ";    
    bestPool = initialPopulation();

    int shareCount = NUM_CHROMO * SHARE_PERCENT;
    int bestPoolCount = NUM_THREADS * shareCount;
    // randomizeCity();
    omp_set_num_threads(NUM_THREADS);

    double time_taken = omp_get_wtime();
    #pragma omp parallel
    {
        for(int gen=0;gen<=GENERATIONS;gen += SHARE_AFTER){
            if(gen == 0){
               population = initialPopulation();
            }

            int tid = omp_get_thread_num();
            for(int i=0;i<SHARE_AFTER;i++){
                population = nextGeneration();
            }
            sort(population.begin(), population.end(), chromoFitnessDecComparator);

            // #pragma omp critical
            if(tid == 0)
                cout<<"Thread "<<tid<<" best after "<<gen<<" Generations: Tour: "<<population[0].tour_distance<<"\n";

            for(int i=0;i<shareCount;i++){
                bestPool[shareCount*tid + i] = population[i];
            }
            #pragma omp barrier

            for(int i=0;i<bestPoolCount;i++){
                population[NUM_CHROMO - 1 -i] = bestPool[i];
            }

            // if(deer[0].fitness > bestDeer[0].fitness) bestDeer = deer;
            shuffle(population.begin(), population.end(), default_random_engine(rand()));
        }
    }
    time_taken = omp_get_wtime() - time_taken;
    cout<<"\nTime Taken: "<<(double)time_taken<<"\n";
    int best = 0;
    for(int i=1;i<NUM_CHROMO;i++){
        if(bestPool[i].fitness > bestPool[best].fitness) best = i;
    }
    cout<<"After "<<GENERATIONS<<" Generations: ";
    cout<<"Tour Distance: "<<bestPool[best].tour_distance<<"\n";
}
/*
To run, ensure that openMP (tested on version 4.5) is installed.
g++ ga_parallel -o gap -fopenmp
./gap
*/