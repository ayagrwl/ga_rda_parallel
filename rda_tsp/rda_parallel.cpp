#include "rdatsp.hpp"
#include <omp.h>

const int SHARE_AFTER = 10;
const int NUM_THREADS = 4;
const double SHARE_PERCENT = 0.1;   // Each thread will share % of its population to the best

int main(){
    srand(time(NULL));
    loadCities("./cities.txt");

    // cout<<"Initially: ";    
    bestDeer = initialPopulation();

    int shareCount = NUM_DEER * SHARE_PERCENT;
    int bestPoolCount = NUM_THREADS * shareCount;
    // randomizeCity();
    omp_set_num_threads(NUM_THREADS);

    double time_taken = omp_get_wtime();
    #pragma omp parallel
    {
        for(int gen=0;gen<=NUM_GENERATIONS;gen += SHARE_AFTER){
            if(gen == 0){
                deer = initialPopulation();
            }

            int tid = omp_get_thread_num();
            for(int i=0;i<SHARE_AFTER;i++){
                deer = nextGeneration();
            }
            sort(deer.begin(), deer.end(), deerFitnessDecComparator);

            // #pragma omp critical
            if(tid == 0)
                cout<<"Thread "<<tid<<" best after "<<gen<<" Generations: Tour: "<<deer[0].tour_distance<<"\n";

            for(int i=0;i<shareCount;i++){
                bestDeer[shareCount*tid + i] = deer[i];
            }
            #pragma omp barrier

            for(int i=0;i<bestPoolCount;i++){
                deer[NUM_DEER -1 -i] = bestDeer[i];
            }

            // if(deer[0].fitness > bestDeer[0].fitness) bestDeer = deer;
            shuffle(deer.begin(), deer.end(), default_random_engine(rand()));
        }
    }
    time_taken = omp_get_wtime() - time_taken;
    cout<<"\nTime Taken: "<<(double)time_taken<<"\n";
    int best = 0;
    for(int i=1;i<NUM_DEER;i++){
        if(bestDeer[i].fitness > bestDeer[best].fitness) best = i;
    }
    cout<<"After "<<NUM_GENERATIONS<<" Generations: ";
    cout<<"Tour Distance: "<<bestDeer[best].tour_distance<<"\n";
}
/*
To run, ensure that openMP (tested on version 4.5) is installed.
g++ rda_parallel -o rdap -fopenmp
./rdap
*/