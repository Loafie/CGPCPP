#pragma once
#include <iostream>
#include "Node.h"
#include "CGPFunction.h"

class CGP {
private:
	int inputs;
	int outputs;
	int columnsBack;
	int rows;
	int cols;
	int numFuncs;
	CGPFunction** functions;
	Node** nodes;
	int* outputNodes;
	bool* activeNodes;
	int activeNodeCount;
	
	int countActiveNodes();
	int* mutatedOutputs(CGP* other, double mRate);
	Node** mutatedNodes(CGP* other, double mRate);
	bool* getActiveNodes();
	int* initializeRandomOutputs();
	Node** initializeRandomNodes();
	int* getRangeBack(int index);
	void setNodeActive(int index, bool* act);

public:
	CGP(int inputs, int outputs, int columnsBack, int rows, int cols, int numFuncs, CGPFunction** functions);
	CGP(CGP* other) : CGP(other, 0.0) {};
	CGP(CGP* other, double mutationRate);
	~CGP();
	double* evaluate(double* inputs);
	friend std::ostream& operator << (std::ostream& out, const CGP& c);
};