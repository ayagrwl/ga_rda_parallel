#include <utility>
#include <random>
#include <time.h>
#include <ctime>
#include <vector>
#include <algorithm>
#include <iostream>

using namespace std;

#define rand_frac (double)rand()/(double)RAND_MAX

/* Constants */
const bool DEBUG = false;

const int NUM_CITIES = 25;
const int NUM_DEER = 100;
const int NUM_MALE = 25;       // A upper bound on GAMMA * NUM_DEER

const double ALPHA = 0.8;          // Comm mates with alpha% of hinds in his harem
const double BETA = 0.6;           // Comm mates with beta% of hinds in other harem
const double GAMMA = 0.2;          // gamma percent of best males become commanders

const int NUM_GENERATIONS = 100;
const double MUTATION_RATE = 0.1;
const int FITNESS_MULT = 1000;
// const int UB = 5;               // When male roars, he can move in this space
// const int LB = 0;               // Only. 


struct RDA_Deer {
    double tour_distance;       // Holds the tour distance for this deer.
    vector<double> rkd;     // Holds the random key order for this deer.
    double fitness;

    void calculate_tour();
    void normalize_rkd();
};

struct City {
	int x, y;
};


City city[NUM_CITIES];
vector<RDA_Deer> bestDeer;
thread_local vector<RDA_Deer> deer;
thread_local vector<pair<int, double>> indexedFitness;
thread_local vector<int> males;
thread_local vector<int> hinds;

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

void loadCities(string fileName){
    FILE *cinBak = stdin;
    freopen(&fileName[0], "r", stdin);

    for(int i=0, x, y;i<NUM_CITIES;i++){
        cin>>city[i].x>>city[i].y;
    }
    fclose(stdin);
    stdin = cinBak;
}

vector<double> createRKD(){
    vector<double> route = vector<double>(NUM_CITIES);
    for(int i=0;i<NUM_CITIES;i++) route[i] = rand_frac;
    return route;
}

double cityDistance(City &a, City &b){
    return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y));
}

bool pairSecondComparator(pair<int, double> &a, pair<int, double> &b){
    return a.second < b.second;
}

bool pairFirstComparator(pair<int, double> &a, pair<int, double> &b){
    return a.first < b.first;
}

bool deerFitnessDecComparator(RDA_Deer &a, RDA_Deer &b){
    return a.fitness > b.fitness;
}

void printRoute(RDA_Deer &deer){
    pair<int, double> rkd_numbered[NUM_CITIES];
    for(int i=0;i<NUM_CITIES;i++){
        rkd_numbered[i].first = i;
        rkd_numbered[i].second = deer.rkd[i];
    }
    sort(rkd_numbered, rkd_numbered+NUM_CITIES, pairSecondComparator);
    
    for(int i=0;i<NUM_CITIES;i++) cout<<rkd_numbered[i].first<<" ";
    cout<<"\n";
}

void RDA_Deer::calculate_tour(){
    // First get the order in which to visit vertices
    pair<int, double> rkd_numbered[NUM_CITIES];
    for(int i=0;i<NUM_CITIES;i++){
        rkd_numbered[i].first = i;
        rkd_numbered[i].second = this->rkd[i];
    }
    sort(rkd_numbered, rkd_numbered+NUM_CITIES, pairSecondComparator);

    double distance = 0;
    int curr_city = rkd_numbered[0].first;
    int next_city;
    for(int i=1;i<NUM_CITIES;i++){
        next_city = rkd_numbered[i].first;
        distance += cityDistance(city[curr_city], city[next_city]);
        curr_city = next_city;
    }
    // To complete a cycle
    distance += cityDistance(city[NUM_CITIES-1], city[0]);

    // cout<<distance<<"\n";
    this->tour_distance = distance;
    this->fitness = (double)FITNESS_MULT  * (double)1/distance;
}

void RDA_Deer::normalize_rkd(){
    double totRkd = 0;
    for(int i=0;i<NUM_CITIES;i++) totRkd += this->rkd[i];

    for(int i=0;i<NUM_CITIES;i++) this->rkd[i] /= totRkd;    
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

void printVector(vector<double> &vec){
    for(int i=0, n=vec.size();i<n;i++) cout<<vec[i]<<" ";
    cout<<"\n";
}

vector<RDA_Deer> initialPopulation(){
    vector<RDA_Deer>  pops;
    for(int i=0;i<NUM_DEER;i++){
        RDA_Deer x;
        x.rkd = createRKD();
        // x.normalize_rkd();
        x.calculate_tour();
        pops.push_back(x);
    }
    return pops;
}

// returns a sorted list of fitness values paired with the index of chromosome.
vector<pair<int, double>> RDA_indexedFitness(){
    if(DEBUG) cout<<"Calculating Indexed Fitness\n";
    vector<pair<int, double>> fitness = vector<pair<int, double>>(NUM_DEER);

    double cumuFitness = 0;
    for(int i=0;i<NUM_DEER;i++){
        cumuFitness += deer[i].fitness;
    }
    // cout<<cumuFitness<<"\n";
    for(int i=0;i<NUM_DEER;i++){
        fitness[i].first = i;
        fitness[i].second = deer[i].fitness / cumuFitness;
        // cout<<i<<" "<<fitness[i].second<<"\n";
    }
    sort(fitness.begin(), fitness.end(), pairSecondComparator);

    // if(DEBUG) {
    //     cout<<"fitness\n";
    //     for(int i=0;i<NUM_DEER;i++){
    //         cout<<fitness[i].first<<" "<<fitness[i].second<<"  "<<deer[fitness[i].first].tour_distance<<"\n";
    //     }
    // }
    return fitness;
}

// void RDA_Roar();

void RDA_Distrib_Male_Hind(){
    if(DEBUG) cout<<"Distributing Males and Hinds\n";
    males = vector<int>(NUM_MALE);
    hinds = vector<int>(NUM_DEER - NUM_MALE);

    for(int i=0;i<NUM_MALE;i++){
        males[i] = indexedFitness[NUM_DEER - 1 - i].first;
    }
    for(int i=0;i<NUM_DEER - NUM_MALE;i++){
        hinds[i] = indexedFitness[i].first;
    }
    shuffle(hinds.begin(), hinds.end(), default_random_engine(rand())); 
}

vector<RDA_Deer> RDA_Make_Commanders(){
    if(DEBUG) cout<<"Making Commanders\n";
    vector<RDA_Deer> commander;
    int numCommander = GAMMA * NUM_MALE;

    for(int i=0;i<numCommander;i++){
        int deerno = indexedFitness[NUM_DEER - 1 - i].first;
        commander.push_back(deer[deerno]);
    }
    return commander;
}

RDA_Deer Fight_Mate_pair(RDA_Deer &a, RDA_Deer &b){
    if(DEBUG) cout<<"Fight_Mate operation running\n";
    RDA_Deer child;
    child.rkd = vector<double>(NUM_CITIES);
    for(int i=0;i<NUM_CITIES;i++){
        child.rkd[i] = (a.rkd[i] + b.rkd[i]) / 2;
    }
    // child.normalize_rkd();
    child.calculate_tour();
    if(child.tour_distance == 0){
        cout<<"\n\nPANIC\n\n";
        exit(-1);
    }
    return child;
}

vector<RDA_Deer> RDA_Fight_Comm_Male(vector<RDA_Deer> &commanders){
    if(DEBUG) cout<<"Commanders vs Males\n";
    int nC = commanders.size();
    int nM = males.size();
    for(int i=0;i<nC;i++){
        int maleNo = rand() % NUM_MALE;
        RDA_Deer offspring = Fight_Mate_pair(commanders[i], deer[males[maleNo]]);
        
        if(offspring.fitness > commanders[i].fitness) {
            commanders[i] = offspring;
        }
    }
    return commanders;
}

vector<double> relativePower(vector<RDA_Deer> &commanders){
    if(DEBUG) cout<<"Calculating Relative Power of Commanders\n";
    double totPower = 0;
    int nC = commanders.size();
    for(int i=0;i<nC;i++) totPower += commanders[i].fitness;

    vector<double> power = vector<double>(nC);
    for(int i=0;i<nC;i++){
        power[i] = commanders[i].fitness / totPower;
    }
    return power;
}

vector<vector<int>> RDA_Make_Harems(vector<RDA_Deer> &commanders){
    if(DEBUG) cout<<"Making Harems\n";
    int nC = commanders.size();
    vector<double> power = relativePower(commanders);

    vector<vector<int>> harems = vector<vector<int>>(nC);
    int nHinds = NUM_DEER - NUM_MALE;

    for(int i=0, hi=0;i<nC;i++){
        int haremSize = power[i] * nHinds;
        harems[i] = vector<int>(haremSize);
        for(int j=0;j<haremSize;j++){
            harems[i][j] = hinds[hi++];
        }

    }
    return harems;
}

vector<RDA_Deer> RDA_Mate_Self_Harem(vector<RDA_Deer> &commanders, vector<vector<int>> &harems){
    if(DEBUG) cout<<"Commanders in self harems\n";
    int nC = commanders.size();
    vector<RDA_Deer> children;

    for(int i=0;i<nC;i++){
        int power = harems[i].size() * ALPHA;
        for(int j=0;j<power;j++){
            RDA_Deer child = Fight_Mate_pair(commanders[i], deer[harems[i][j]]);
            children.push_back(child);
        }
    }
    return children;
}

vector<RDA_Deer> RDA_Mate_Other_Harem(vector<RDA_Deer> &commanders, vector<vector<int>> &harems){
    if(DEBUG) cout<<"Commanders in other harems\n";
    int nC = commanders.size();
    vector<RDA_Deer> children;

    for(int i=0;i<nC;i++){
        int power = harems[i].size() * BETA;
        for(int j=0;j<power;j++){
            int ti = ((rand() % (nC-1) ) + i ) % nC;
            int tj = rand() % harems[ti].size();
            RDA_Deer child = Fight_Mate_pair(commanders[i], deer[harems[ti][tj]]);
            children.push_back(child);
        }
    }
    return children;
}

vector<RDA_Deer> RDA_Mate_Males(){
    if(DEBUG) cout<<"Remaining Males with hinds\n";
    int nM = males.size();
    int nH = hinds.size();

    vector<RDA_Deer> children;

    for(int i=0;i<nM;i++){
        int hindNo = hinds[rand() % nH];
        RDA_Deer child = Fight_Mate_pair(deer[males[i]], deer[hindNo]);
        children.push_back(child);
    }
    return children;
}

// vector<RDA_Deer> RDA_Roulette_Pick(vector<RDA_Deer> children, int num){
//     vector<RDA_Deer> selection = vector<RDA_Deer>(num);

//     int totChildren = children.size();
//     double totFitness = 0;

//     for(int i=0;i<totChildren;i++){
//         totFitness += children[i].fitness;
//     }

//     for(int i=0, childi = 0;i<num;i++){

//     }
//     return selection;
// };

RDA_Deer mutate(RDA_Deer x){
    bool changed = false;
    for(int i=0;i<NUM_CITIES;i++){
        if(rand_frac < MUTATION_RATE){
            int j = rand() % NUM_CITIES;
            if(i != j){
                changed = true;
                swap(x.rkd[i], x.rkd[j]);
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

vector<RDA_Deer> mutatePopulation(vector<RDA_Deer> generation){
    // cout<<"Elite was "<<generation[0].tour_distance<<" ";
    for(int i=0;i<NUM_DEER;i++){
        generation[i] = mutate(generation[i]);
    }
    // cout<<"became "<<generation[0].tour_distance<<"\n";
    return generation;
}

vector<RDA_Deer> RDA_Tournament_Pick(vector<RDA_Deer> &children, int num){
    if(DEBUG) cout<<"Tournament Going to Start\n";
    vector<RDA_Deer> selection = vector<RDA_Deer>(num);

    int nHinds = hinds.size();
    for(int i=0;i<nHinds;i++) children.emplace_back(deer[hinds[i]]);

    int totChildren = children.size();

    if(num > totChildren){
        cout<<"totChildren: "<<totChildren<<"\n";
        cout<<"Num Hinds: "<<nHinds<<"\n";
        cout<<"Num to Pick: "<<num<<"\n";
        cout<<"num > totChildren .. .exiting";
        exit(-1);
    }

    for(int i=0;i<num;i++){
        int c1 = rand() % totChildren;
        int c2 = rand() % totChildren;
        // cout<<children[c1].fitness<<" "<<children[c1].fitness<<"\n";
        if(children[c1].fitness >= children[c1].fitness){
            selection[i] = children[c1];
        } else {
            selection[i] = children[c2];
        }
    }
    // if(DEBUG) cout<<"Tournament Ended\n";
    return selection;
};

vector<RDA_Deer> RDA_Best_Pick(vector<RDA_Deer> &children, int num){
    if(DEBUG) cout<<"Tournament Going to Start\n";
    vector<RDA_Deer> selection = vector<RDA_Deer>(num);

    int nHinds = hinds.size();
    for(int i=0;i<nHinds;i++) children.emplace_back(deer[hinds[i]]);

    children = mutatePopulation(children);
    sort(children.begin(), children.end(), deerFitnessDecComparator);

    int totChildren = children.size();

    for(int i=0;i<num;i++){
        selection[i] = children[i];
    }
    // if(DEBUG) cout<<"Tournament Ended\n";
    return selection;
};

vector<RDA_Deer> RDA_Child_Pick_2(vector<RDA_Deer> &children, int num){
    if(DEBUG) cout<<"Tournament Going to Start\n";
    vector<RDA_Deer> selection = children;

    num -= children.size();

    shuffle(hinds.begin(), hinds.end(), default_random_engine(rand()));

    for(int i=0;i<num;i++){
        selection.emplace_back(deer[hinds[i]]);
    }
    // if(DEBUG) cout<<"Tournament Ended\n";
    return selection;
};

vector<RDA_Deer> mergeVectors3(vector<RDA_Deer> &a, vector<RDA_Deer> &b, vector<RDA_Deer> &c){
    if(DEBUG) cout<<"Merging 3 Children Vectors\n";
    vector<RDA_Deer> merged = vector<RDA_Deer>(a.size() + b.size() + c.size());

    int mergeI = 0;
    for(int i=0;i<a.size();i++) merged[mergeI++] = a[i];
    for(int i=0;i<b.size();i++) merged[mergeI++] = b[i];
    for(int i=0;i<c.size();i++) merged[mergeI++] = c[i];
    return merged;
}

vector<RDA_Deer> mergePopulation(vector<RDA_Deer> &a, vector<RDA_Deer> &b){
    if(DEBUG) cout<<"Gathering Next Generation\n";
    vector<RDA_Deer> nextGen = vector<RDA_Deer>(NUM_MALE);
    for(int i=0;i<NUM_MALE;i++) nextGen[i] = deer[males[i]];
    
    for(int i=0;i<a.size();i++) nextGen.emplace_back(a[i]);
    for(int i=0;i<b.size();i++) nextGen.emplace_back(b[i]);

    shuffle(nextGen.begin(), nextGen.end(), default_random_engine(rand()));
    return nextGen;  
}

vector<RDA_Deer> nextGeneration(){
    // RDA_Roar();  // Roaring changes the order of the cities, useless in our case
    indexedFitness = RDA_indexedFitness();

    RDA_Distrib_Male_Hind();
    vector<RDA_Deer> commanders = RDA_Make_Commanders();

    commanders = RDA_Fight_Comm_Male(commanders);

    vector<vector<int>> harems = RDA_Make_Harems(commanders);

    vector<RDA_Deer> pureChildren = RDA_Mate_Self_Harem(commanders, harems);
    vector<RDA_Deer> mixedChildren = RDA_Mate_Other_Harem(commanders, harems);
    vector<RDA_Deer> mudChildren = RDA_Mate_Males();

    vector<RDA_Deer> contestants = mergeVectors3(pureChildren, mixedChildren, mudChildren);

    int numChildren = NUM_DEER - males.size() - commanders.size();
    // vector<RDA_Deer> children = RDA_Tournament_Pick(contestants, numChildren);
    vector<RDA_Deer> children = RDA_Best_Pick(contestants, numChildren);

    return mergePopulation(commanders, children);
}

int bestinGeneration(){
    int best = 0;
    for(int i=1;i<NUM_DEER;i++){
        if(deer[i].tour_distance < deer[best].tour_distance)
            best = i;
    }
    return best;
}

void printDeer(int deerNo){
    cout<<"Tour Distance: "<<bestDeer[deerNo].tour_distance<<"\n";
}

