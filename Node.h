#pragma once
class Node {
public:
	int* inputs;
	int function;
	int arity;

	Node(int* inputs, int func, int ar);
	~Node();
};