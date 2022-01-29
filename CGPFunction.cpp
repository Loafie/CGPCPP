#include "CGPFunction.h"

CGPFunction::CGPFunction(double (*f)(double*), int ar) {
	this->arity = ar;
	this->theFunc = f;
}

CGPFunction::~CGPFunction() {

}