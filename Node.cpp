#include "Node.h"

Node::Node(int* inputs, int func, int ar)
{
	this->inputs = inputs;
	this->arity = ar;
	this->function = func;
}

Node::~Node() {
	delete this->inputs;
}