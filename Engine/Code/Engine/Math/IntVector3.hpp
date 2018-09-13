#pragma once

class IntVector3
{
public:
	int x;
	int y;
	int z;

	IntVector3(int xInput, int yInput, int zInput);
	IntVector3();
	~IntVector3();

	static const IntVector3 ZERO;
};
