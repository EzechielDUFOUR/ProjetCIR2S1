#include <iostream>

class Pos {
private:
	float x, y, z;

public:
	Pos() : x(rand()%101), y(rand()%101), z(rand()%101){}; 
	float getX() { return x; }
	float getY() { return y; }
	float getZ() { return z; }

};

class Plane {
private:
	Pos pos;
public:
    Plane() : pos(Pos()) {}
    Pos getPos() { return pos; }
};