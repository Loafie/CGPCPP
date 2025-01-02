#include "CGP.h"
#include <stdlib.h>
#include <time.h>
#include <fstream>

CGP::CGP(int inputs, int outputs, int columnsBack, int rows, int cols, int numFuncs, CGPFunction** functions) {
	this->inputs = inputs;
	this->outputs = outputs;
	this->columnsBack = columnsBack;
	this->rows = rows;
	this->cols = cols;
	this->numFuncs = numFuncs;
	this->functions = functions;
	this->nodes = initializeRandomNodes();
	this->outputNodes = initializeRandomOutputs();
	this->activeNodes = getActiveNodes();
	this->activeNodeCount = countActiveNodes();
}

CGP::CGP(int inputs, int outputs, int columnsBack, int rows, int cols) {
	this->inputs = inputs;
	this->outputs = outputs;
	this->columnsBack = columnsBack;
	this->rows = rows;
	this->cols = cols;
	this->numFuncs = 10;
	this->functions = defaultFunctions();
	this->nodes = initializeRandomNodes();
	this->outputNodes = initializeRandomOutputs();
	this->activeNodes = getActiveNodes();
	this->activeNodeCount = countActiveNodes();
}


CGP::CGP(CGP* other, double mRate) {
	this->inputs = other->inputs;
	this->outputs = other->outputs;
	this->columnsBack = other->columnsBack;
	this->rows = other->rows;
	this->cols = other->cols;
	this->numFuncs = other->numFuncs;
	this->functions = other->functions;
	this->nodes = mutatedNodes(other, mRate);
	this->outputNodes = mutatedOutputs(other, mRate);
	this->activeNodes = getActiveNodes();
	this->activeNodeCount = countActiveNodes();
}

CGP::CGP(const char* fileName, CGPFunction** funcs) {
	std::ifstream inFile(fileName, std::ios::binary);
	inFile.read((char *)&this->inputs, sizeof(int));
	inFile.read((char*)&this->outputs, sizeof(int));
	inFile.read((char*)&this->columnsBack, sizeof(int));
	inFile.read((char*)&this->rows, sizeof(int));
	inFile.read((char*)&this->cols, sizeof(int));
	inFile.read((char*)&this->numFuncs, sizeof(int));
	Node** theNodes = new Node * [this->rows * this->cols];
	for (int i = 0; i < this->rows * this->cols; i++) {
		int a = 0;
		int f = 0;
		inFile.read((char*)&a, sizeof(int));
		inFile.read((char*)&f, sizeof(int));
		int* ins = new int[a];
		inFile.read((char*)ins, sizeof(int) * a);
		Node* n = new Node(ins, f, a);
		theNodes[i] = n;
	}
	int* theOuts = new int[this->outputs];
	for (int i = 0; i < this->outputs; i++) {
		inFile.read((char*)&theOuts[i], sizeof(int));
	}
	this->nodes = theNodes;
	this->outputNodes = theOuts;
	inFile.close();
	this->activeNodes = getActiveNodes();
	this->activeNodeCount = countActiveNodes();
	this->functions = funcs;
}


int* CGP::getRangeBack(int index) {
	int* result = new int[2];
	int max = (index - (index % this->rows) - 1) + this->inputs;
	int min = ((max - (this->rows * this->columnsBack) + 1) >= 0) ? (max - (this->rows * this->columnsBack) + 1) : 0;
	result[0] = min;
	result[1] = max;
	return result;
}

Node** CGP::initializeRandomNodes() {
	Node** theNodes = new Node*[this->rows * this->cols];
	for (int i = 0; i < (this->rows * this->cols); i++) {
		int funcIdx = rand() % this->numFuncs;
		int* rangeBack = this->getRangeBack(i);
		int arity = this->functions[funcIdx]->arity; 
		int* ins = new int[arity];
		for (int i = 0; i < arity; i++) {
			ins[i] = rangeBack[0] + rand() % (rangeBack[1] - rangeBack[0] + 1);
		}
		delete[] rangeBack;
		theNodes[i] = new Node(ins, funcIdx, arity);
	}
	return theNodes;
}

int* CGP::initializeRandomOutputs() {
	int* theOuts = new int[this->outputs];
	for (int i = 0; i < outputs; i++) {
		theOuts[i] = rand() % (this->inputs + (this->rows * this->cols));
	}
	return theOuts;
}


void CGP::setNodeActive(int index, bool* act) {
	//std::cout << index << ", ";
	if (index >= this->inputs && act[index - this->inputs] != true) {
		act[index - this->inputs] = true;
		for (int i = 0; i < nodes[index - this->inputs]->arity; i++) {
			if (nodes[index - this->inputs]->inputs[i] >= this->inputs) {
				this->setNodeActive(nodes[index - this->inputs]->inputs[i], act);
			}
		}
	};
}

bool* CGP::getActiveNodes(){
	bool* theActiveNodes = new bool[this->rows * this->cols];
	for (int i = 0; i < (this->rows * this->cols); i++) {
		theActiveNodes[i] = false;
	}
	for (int i = 0; i < this->outputs; i++) {
		this->setNodeActive(this->outputNodes[i], theActiveNodes);
	}
	return theActiveNodes;
}

double* CGP::evaluate(double* ins) {
	double* vals = new double[this->rows * this->cols];
	for (int i = 0; i < (this->rows * this->cols); i++) {
		if (this->activeNodes[i] == true) {
			double* params = new double[nodes[i]->arity];
			for (int j = 0; j < nodes[i]->arity; j++) {
				if (nodes[i]->inputs[j] < this->inputs) {
					params[j] = ins[nodes[i]->inputs[j]];
				}
				else
				{
					params[j] = vals[nodes[i]->inputs[j] - this->inputs];
				}
			}
			vals[i] = functions[nodes[i]->function]->theFunc(params);
			delete[] params;
		}
	}
	double* results = new double[this->outputs];
	for (int i = 0; i < this->outputs; i++) {
		if (outputNodes[i] < this->inputs) {
			results[i] = ins[outputNodes[i]];
		}
		else {
			results[i] = vals[outputNodes[i] - this->inputs];
		}
	}
	delete[] vals;
	return results;
}

Node** CGP::mutatedNodes(CGP* other, double mRate) {
	Node** newNodes = new Node*[this->rows * this->cols];
	for (int i = 0; i < (this->rows * this->cols); i++) {
		double roll = (double)(rand() / (double)RAND_MAX);
		if (mRate > roll) {
			int f = rand() % this->numFuncs;
			int* rangeBack = this->getRangeBack(i);
			int arity = this->functions[f]->arity;
			int* ins = new int[arity];
			for (int i = 0; i < arity; i++) {
				ins[i] = rangeBack[0] + rand() % (rangeBack[1] - rangeBack[0] + 1);
			}
			newNodes[i] = new Node(ins, f, arity);
			delete[] rangeBack;
		}
		else
		{
			int* ins = new int[other->nodes[i]->arity];
			for (int k = 0; k < other->nodes[i]->arity; k++) {
				ins[k] = other->nodes[i]->inputs[k];
			}
			newNodes[i] = new Node(ins, other->nodes[i]->function, other->nodes[i]->arity);
		}
	}
	return newNodes;
}

int* CGP::mutatedOutputs(CGP* other, double mRate) {
	int* newOuts = new int[this->outputs];
	for (int i = 0; i < this->outputs; i++) {
		double roll = (double)(rand() / (double)RAND_MAX);
		if (mRate > roll) {
			newOuts[i] = rand() % (this->inputs + (this->rows * this->cols));
		}
		else
		{
			newOuts[i] = other->outputNodes[i];
		}
	}
	return newOuts;
}

int CGP::countActiveNodes() {
	int count = 0;
	for (int i = 0; i < (this->rows * this->cols); i++) {
		
		if (this->activeNodes[i] == true) {
			count++;
		}
	}
	return count;
}

CGPFunction** CGP::defaultFunctions() {
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
	return funcs;
}

CGP::~CGP() {
	for (int i = 0; i < (this->rows * this->cols); i++) {
		delete this->nodes[i];
	}
	delete[] this->nodes;
	delete[] this->outputNodes;
	delete[] this->activeNodes;
}

std::ostream& operator << (std::ostream& out, const CGP& c)
{
	out << "Inputs: " << c.inputs << ", Outputs: " << c.outputs << ", Columns Back: " << c.columnsBack << ", Columns: " << c.cols << ", Rows: " << c.rows << ", Number of Functions: " << c.numFuncs;
	out << "\nActive Node Count: " << c.activeNodeCount << "\n";
	for (int i = 0; i < c.rows; i++) {
		for (int j = 0; j < c.cols; j++) {
			out << "N: " << j * c.rows + i + c.inputs << " ( I: [";
			for (int k = 0; k < c.nodes[j * c.rows + i]->arity; k++) {
				out << " " << c.nodes[j * c.rows + i]->inputs[k];
			}
			out << "] F: " << c.nodes[j * c.rows + i]->function << ") ";
		}
		if (i < c.outputs) {
			out << "O: " << i << " : " << c.outputNodes[i] << "\n";
		}
		else
		{
			out << "\n";
		}
	}
	for (int i = 0; i < (c.cols * c.rows); i++) {
		std::cout << "Node " << i + c.inputs << ":" << (c.activeNodes[i] ? "O, " : "X, ");
	}
	std::cout << "\n";
	return out;
}


void CGP::writeToFile(const char* fileName) {
	std::ofstream outFile(fileName, std::ios::out | std::ios::binary);
	outFile.write((char*)&this->inputs, sizeof(int));
	outFile.write((char*)&this->outputs, sizeof(int));
	outFile.write((char*)&this->columnsBack, sizeof(int));
	outFile.write((char*)&this->rows, sizeof(int));
	outFile.write((char*)&this->cols, sizeof(int));
	outFile.write((char*)&this->numFuncs, sizeof(int));
	for (int i = 0; i < rows * cols; i++) {
		int f = this->nodes[i]->function;
		int a = this->nodes[i]->arity;
		outFile.write((char*)&a, sizeof(int));
		outFile.write((char*)&f, sizeof(int));
		outFile.write((char*)this->nodes[i]->inputs, sizeof(int) * a);
	}
	for (int i = 0; i < outputs; i++) {
		outFile.write((char*)&this->outputNodes[i], sizeof(int));
	} 
	outFile.close();
}