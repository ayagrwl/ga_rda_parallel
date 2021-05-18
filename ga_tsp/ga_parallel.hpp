#include <utility>
#include <random>
#include <time.h>
#include <ctime>
#include <vector>
#include <algorithm>
#include <iostream>

using namespace std;

#define rand_frac (double)rand()/(double)RAND_MAX

const bool DEBUG = false;

/* Constants */
const int NUM_CITIES = 25;
const int NUM_CHROMO = 100;
const int ELITE_SIZE = 20;

const int FITNESS_MULT = 1000;

const double MUTATION_RATE = 0.001;
const double GENERATIONS = 100;

struct Chromosome {
    double tour_distance;        // Holds the tour distance
    vector<int> route;           // holds the route
    double fitness;              // simply = 1/tour_distance.

    void calculate_tour();
};

struct City {
	int x, y;
};

City city[NUM_CITIES];
thread_local vector<Chromosome> population;

void loadCities(string fileName){
    FILE *cinBak = stdin;
    freopen(&fileName[0], "r", stdin);

    for(int i=0, x, y;i<NUM_CITIES;i++){
        cin>>city[i].x>>city[i].y;
    }
    fclose(stdin);
    stdin = cinBak;
}

bool chromoFitnessDecComparator(Chromosome &a, Chromosome &b){
    return a.fitness > b.fitness;
}

void randomizeCity(){
    int farthest = 0;
    for(int i=0;i<NUM_CITIES;i++){
        city[i].x = rand() % 200;
        city[i].y = rand() % 200;

        // city[i].x = i;
        // city[i].y = 0;
    }
    // cout<<"Ideal Answer: "<<2*NUM_CITIES<<" units\n";
}

vector<int> createRoute(){
    vector<int> route = vector<int>(NUM_CITIES);
    for(int i=0;i<NUM_CITIES;i++) route[i] = i;
    shuffle(route.begin(), route.end(), default_random_engine(rand()));
    return route;
}

double cityDistance(City &a, City &b){
    return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y));
}


int dummyDataDist(vector<int> route){
    int dist = 0;
    int n = route.size();
    for(int i=1;i<n;i++){
        dist += abs(route[i] - route[i-1]);
    }
    dist += abs(route[0] - route[n-1]);
    return dist;
}

void Chromosome::calculate_tour(){
    double distance = 0;
    for(int i=1, next_city_no, curr_city_no = this->route[0];i<NUM_CITIES;i++){
        next_city_no = this->route[i];
        distance += cityDistance(city[curr_city_no], city[next_city_no]);
        curr_city_no = next_city_no;
    } 
    // To complete a cycle
    distance += cityDistance(city[NUM_CITIES-1], city[0]);

    // distance = dummyDataDist(this->route);
    this->tour_distance = distance;
    this->fitness = (double)FITNESS_MULT * (double(1)/distance);
}

void printVector(vector<int> &vec){
    for(int i=0, n=vec.size();i<n;i++) cout<<vec[i]<<" ";
    cout<<"\n";
}

vector<Chromosome> initialPopulation(){
    vector<Chromosome>  pops;
    for(int i=0;i<NUM_CHROMO;i++){
        Chromosome x;
        x.route = createRoute();
        x.calculate_tour();
        pops.push_back(x);
    }
    return pops;
}

bool pairSecondComparator(pair<int, double> &a, pair<int, double> &b){
    return a.second < b.second;
}

bool pairFirstComparator(pair<int, double> &a, pair<int, double> &b){
    return a.first < b.first;
}

// returns a sorted list of fitness values paired with the index of chromosome.
auto indexedFitness(){
    vector<pair<int, double>> fitness = vector<pair<int, double>>(NUM_CHROMO);
    double cumuFitness = 0;
    for(int i=0;i<NUM_CHROMO;i++){
        cumuFitness += population[i].fitness;
    }
    // cout<<cumuFitness<<"\n";
    for(int i=0;i<NUM_CHROMO;i++){
        fitness[i].first = i;
        fitness[i].second = population[i].fitness / cumuFitness;
        // cout<<i<<" "<<fitness[i].second<<"\n";
    }
    sort(fitness.begin(), fitness.end(), pairSecondComparator);

    if(DEBUG) {
        cout<<"fitness\n";
        for(int i=0;i<NUM_CHROMO;i++){
            cout<<fitness[i].first<<" "<<fitness[i].second<<"  "<<population[fitness[i].first].tour_distance<<"\n";
        }
    }
    return fitness;
}

vector<int> selection(vector<pair<int, double>> &indexedFitness){
    vector<int> selected;

    // Implementing elitism
    for(int i=0; i<ELITE_SIZE; i++){
        int eliteIndex = indexedFitness[NUM_CHROMO - 1 - i].first;
        selected.push_back(eliteIndex);
    }
    if(DEBUG) {
        cout<<"Chosen Most Elite: ";
        cout<<population[selected[0]].tour_distance<<"\n";    
    }
    sort(indexedFitness.begin(), indexedFitness.end(), pairFirstComparator);
    // Now choosing others with roulette wheel algorithm
    for(int i=0;i<NUM_CHROMO - ELITE_SIZE;i++){
        int S = rand_frac;
        int P = 0;
        for(int j=0;j<NUM_CHROMO;j++){
            P += indexedFitness[j].second;
            if(P >= S){
                selected.push_back(j);
                break;
            }
        }
    }
    return selected;
    // indexedFitness.clear();
}

// vector<Chromosome> matingPool(vector<int> selected){
//     vector<Chromosome> pool = vector<Chromosome>(NUM_CHROMO);
//     for(int i=0;i<NUM_CHROMO;i++){
//         pool[i] = population[selected[i]];
//     }
//     return pool;
// }

Chromosome breed(Chromosome par1, Chromosome par2){
    bool childHas[NUM_CITIES];
    for(int i=0;i<NUM_CITIES;i++) childHas[i] = false;

    int par1Start = rand() % NUM_CITIES;
    int par1End = rand() % NUM_CITIES;
    
    if(par1Start > par1End) swap(par1Start, par1End);

    Chromosome child;
    child.route = vector<int>(NUM_CITIES);
    int childi = 0;
    
    for(int i=par1Start;i<par1End;i++){
        int cityNo = par1.route[i];
        child.route[childi++] = cityNo;
        childHas[cityNo] = true;
    }  // Inhereting from parent 1 done.

    for(int i=0;i<NUM_CITIES;i++){
        int cityNo = par2.route[i];

        if(!childHas[cityNo]) {
            child.route[childi++] = cityNo;
            childHas[cityNo] = true;
        }        
    }

    child.calculate_tour();
    return child;
}

vector<Chromosome> breedPopulation(vector<int> selection){
    vector<Chromosome> children;

    int mixedBreeds = NUM_CHROMO - ELITE_SIZE;

    for(int i=0;i<ELITE_SIZE;i++){
        children.push_back(population[selection[i]]);
    }

    shuffle(selection.begin(), selection.end(), default_random_engine(rand()));
    for(int i=0;i<mixedBreeds;i++){
        children.push_back(breed(population[i], population[NUM_CHROMO - 1 - i]));
    }
    return children;
}

Chromosome mutate(Chromosome x){
    bool changed = false;
    for(int i=0;i<NUM_CITIES;i++){
        if(rand_frac < MUTATION_RATE){
            int j = rand() % NUM_CITIES;
            if(i != j){
                changed = true;
                swap(x.route[i], x.route[j]);
            }
        }
    }
    if(changed){
        if(DEBUG) cout<<x.tour_distance<<" Mutated to";
        x.calculate_tour();
        if(DEBUG) cout<<x.tour_distance<<" new tour\n";
    }
    return x;
}

vector<Chromosome> mutatePopulation(vector<Chromosome> generation){
    // cout<<"Elite was "<<generation[0].tour_distance<<" ";
    for(int i=0;i<NUM_CHROMO;i++){
        generation[i] = mutate(generation[i]);
    }
    // cout<<"became "<<generation[0].tour_distance<<"\n";
    return generation;
}

vector<Chromosome> nextGeneration(){
    vector<pair<int, double>> fitness = indexedFitness();
    vector<int> selected = selection(fitness);
    vector<Chromosome> children = breedPopulation(selected);
    vector<Chromosome> nextGen = mutatePopulation(children);
    return nextGen;
}

int bestinGeneration(){
    int best = 0;
    for(int i=1;i<NUM_CHROMO;i++){
        if(population[i].tour_distance < population[best].tour_distance)
            best = i;
    }
    return best;
}

void printChromosome(int chroNo){
    cout<<"Tour Distance: "<<population[chroNo].tour_distance<<"\n";
}

Chromosome breed_2(Chromosome par1, Chromosome par2){
    bool childHas[NUM_CITIES];
    for(int i=0;i<NUM_CITIES;i++) childHas[i] = false;

    int par1Start = rand() % NUM_CITIES;
    int par1End = rand() % NUM_CITIES;
    
    if(par1Start > par1End) swap(par1Start, par1End);

    Chromosome child;
    child.route = vector<int>(NUM_CITIES);
    
    int childi = 0;

    for(int i=par1Start;i<par1End;i++){
        int cityNo = par1.route[i];
        child.route[i] = cityNo;
        childHas[cityNo] = true;
    }  // Inhereting from parent 1 done.

    int par2i = 0;

    while(childi < par1Start){
        int cityNo = par2.route[par2i++];
        if(!childHas[cityNo]){
            childHas[cityNo] = true;
            child.route[childi++] = cityNo;
        }
    }
    childi = par1End;
    
    while(childi < NUM_CITIES){
        int cityNo = par2.route[par2i++];
        if(!childHas[cityNo]){
            childHas[cityNo] = true;
            child.route[childi++] = cityNo;
        }
    }

    // cout<<par1Start<<"  "<<par1End<<"\n";
    // printVector(par1.route);
    // printVector(par2.route);
    // printVector(child.route);
    // cout<<"\n";

    child.calculate_tour();
    return child;
}















