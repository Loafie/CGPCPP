// CGP.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include "CGP.h"
#include "CGPFunction.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <thread>
#include <string>

#define NUM_PER_GEN 100
#define SAMPLES 500
#define SAMPLES_RANGE 10.0
#define MUTATION_RATE 0.01


float target(float x) {
    return 2 * pow(x, 3) - 12 * pow(x, 2) + x - 71;
}

struct CGPContainer {
    
    public:
        CGP* theCGP;
        double score;

        CGPContainer(CGP* c, double s) {
            this->theCGP = c;
            this->score = s;
    }

        ~CGPContainer() {
            delete theCGP;
    }
};

void main_approximation_test(CGPFunction** funcs) {
    srand(time(NULL));

    double range[2] = { -SAMPLES_RANGE, SAMPLES_RANGE };
    CGPContainer* theCGPs[NUM_PER_GEN];
    for (int i = 0; i < NUM_PER_GEN; i++) {
        theCGPs[i] = new CGPContainer(new CGP(1, 1, 4, 5, 20, 7, funcs), 0.0);
    }
    int gens = 0;
    CGP* Winner;
    while (true) {
        for (int i = 0; i < NUM_PER_GEN; i++) {
            for (int j = 0; j < SAMPLES; j++) {
                double roll = range[0] + ((range[1] - range[0]) * (double)(rand() / (double)RAND_MAX));
                double* in = new double[1];
                in[0] = roll;
                double* results = theCGPs[i]->theCGP->evaluate(in);
                double result = results[0];
                delete results;
                theCGPs[i]->score += abs(result - target(roll));
                delete[] in;
            }
        }
        std::sort(theCGPs, theCGPs + NUM_PER_GEN, [](CGPContainer* s1, CGPContainer* s2) {return s1->score < s2->score; });
        Winner = new CGP(theCGPs[0]->theCGP);
        double top_score = theCGPs[0]->score;
        for (int i = 0; i < NUM_PER_GEN; i++) {
            delete theCGPs[i];
            theCGPs[i] = new CGPContainer(new CGP(Winner, MUTATION_RATE), 0.0);
        }
        std::cout << "Generation: " << gens << ", Low Score: " << top_score << "\n";
        gens++;
    }

}

void CGPsTests(CGPFunction** funcs) {
    CGP* a = new CGP(3, 3, 1, 3, 3, 5, funcs);
    std::cout << *a;
    double* ins = new double[3];
    ins[0] = rand() % 10;
    ins[1] = rand() % 10;
    ins[2] = rand() % 10;
    for (int i = 0; i < 3; i++) {
        std::cout << "In " << i << " : " << ins[i] << " ";
    }
    std::cout << "\n";
    double* outs = a->evaluate(ins);
    for (int i = 0; i < 3; i++) {
        std::cout << "Out " << i << " : " << outs[i] << " ";
    }
    CGP* b = new CGP(a, 0.5);
    delete a;
    delete[] outs;
    std::cout << "\n\n" << *b;
    outs = b->evaluate(ins);
    for (int i = 0; i < 3; i++) {
        std::cout << "Out " << i << " : " << outs[i] << " ";
    }

}



double* normalizeOuts(double* outs, int len) {
    double sum = 0;
    for (int i = 0; i < len; i++) {
        sum += abs(outs[i]);
    }
    if (sum != 0.0) {
        for (int i = 0; i < len; i++) {
            outs[i] = abs(outs[i] / sum);
        }
    }
    return outs;
}



void scoreCGP(unsigned char* images, unsigned char* labels, int* samples, CGPFunction** funcs, CGPContainer* theCGP) {
    for (int i = 0; i < SAMPLES; i++) {
        double input[784];
        for (int x = 0; x < 784; x++) {
            input[x] = (double)images[x + (784 * samples[i])];
        }
        // evaluate each CGP
        double* output;
        output = normalizeOuts(theCGP->theCGP->evaluate(input), 10);
        theCGP->score += output[(int)labels[samples[i]]];
        delete[] output;
    }
}

void imageCatStressTest(unsigned char* images, unsigned char* labels, CGPFunction** funcs) {
    CGPContainer* theCGPs[NUM_PER_GEN];
    for (int i = 0; i < NUM_PER_GEN; i++) {
        theCGPs[i] = new CGPContainer(new CGP(784, 10, 20, 200, 200, 10, funcs), 0.0);
    }
    int gens = 0;
    CGP* Winner;
    while (true) {
        //generate sample set
        int samples[SAMPLES];
        for (int i = 0; i < SAMPLES; i++) {
            samples[i] = rand() % 60000;
        }


        
        std::thread* threads[NUM_PER_GEN];

        // evaluate each CGP
        for (int j = 0; j < NUM_PER_GEN; j++) {
            threads[j] = new std::thread(scoreCGP, images, labels, samples, funcs, theCGPs[j]);
        }
        for (int j = 0; j < NUM_PER_GEN; j++) {
            threads[j]->join();
           // delete threads[j];
        }
        //delete[] threads;
        //sort by score and reproduce
        std::sort(theCGPs, theCGPs + NUM_PER_GEN, [](CGPContainer* s1, CGPContainer* s2) {return s1->score > s2->score; });
        Winner = new CGP(theCGPs[0]->theCGP);
        double top_score = theCGPs[0]->score / (float)SAMPLES;
        for (int i = 0; i < NUM_PER_GEN; i++) {
            delete theCGPs[i];
            theCGPs[i] = new CGPContainer(new CGP(Winner, MUTATION_RATE), 0.0);
        }
        std::cout << "Generation: " << gens << ", Low Score: " << top_score << "\n";
        gens++;
        Winner->writeToFile("Current-Winner.CGP");
        if (gens % 1000 == 0) {
            std::string fname = "Gen-" + std::to_string(gens) + "-Winner.CGP";
            Winner->writeToFile(fname.c_str());
        }
        delete Winner;
    }

}


unsigned char* getTrainingImages() {
    std::ifstream inFile("C:/MNIST/train/train-images.idx3-ubyte", std::ios::binary);
    char header[16];
    unsigned char *images = new unsigned char[60000 * 784];
    inFile.read(header, 16);
    inFile.read((char*)images, 60000 * 784);
    inFile.close();
    return images;
}

unsigned char* getTrainingLabels() {
    
    std::ifstream inFile("C:/MNIST/train/train-labels.idx1-ubyte", std::ios::binary);
    char header[16];
    unsigned char* labels = new unsigned char[60000];
    inFile.read(header, 8);
    inFile.read((char*)labels, 60000);
    inFile.close();
    return labels;
}

void showImageLabels(unsigned char* i, unsigned char* l) {
    int d = rand() % 60000;
    std::cout << "Label: " << (int)l[d] << "\n";
        for (int x = 0; x < 784; x++) {
            if (x % 28 == 0) std::cout << "\n";
            if ((int)i[x + 784 * d] > 10) std::cout << "X"; else std::cout << " ";
    }


}

void testFileWrite(CGPFunction** funcs) {
    CGP* a = new CGP(2, 2, 2, 4, 4, 10, funcs);
    a->writeToFile("Test.CGP");
    std::cout << *a;
    delete a;
    CGP* b = new CGP("Test.CGP", funcs);
    std::cout << *b;
}


int main()
{
    srand(time(NULL));
    CGPFunction* funcs[10];
    funcs[0] = new CGPFunction([](double* ins) {return ins[0] + ins[1]; }, 2);
    funcs[1] = new CGPFunction([](double* ins) {return ins[0] - ins[1]; }, 2);
    funcs[2] = new CGPFunction([](double* ins) {return ins[0] * ins[1]; }, 2);
    funcs[3] = new CGPFunction([](double* ins) {return ins[1] != 0 ? ins[0] / ins[1] : 0; }, 2);
    funcs[4] = new CGPFunction([](double* ins) {return 1.0; }, 0);
    funcs[5] = new CGPFunction([](double* ins) {return 2.0; }, 0);
    funcs[6] = new CGPFunction([](double* ins) {return 10.0; }, 0);
    funcs[7] = new CGPFunction([](double* ins) {return 0.0; }, 0);
    funcs[8] = new CGPFunction([](double* ins) {return 1.0 ? ins[0] >= ins[2] && ins[1] >= ins[2] : 0.0; }, 3);
    funcs[9] = new CGPFunction([](double* ins) {return 1.0 ? ins[0] >= ins[2] || ins[1] >= ins[2] : 0.0; }, 3);
    //testFileWrite(funcs);
    //main_approximation_test(funcs);
    unsigned char* trainingImages = getTrainingImages();
    unsigned char* trainingLabels = getTrainingLabels();
    imageCatStressTest(trainingImages, trainingLabels, funcs);
    //showImageLabels(trainingImages, trainingLabels);
    return 0;
}

