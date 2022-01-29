#pragma once
class CGPFunction {
public:
	double (*theFunc)(double*);
	int arity;
	CGPFunction(double (*f)(double*), int ar);
	~CGPFunction();
};