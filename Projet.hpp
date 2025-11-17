#include <iostream>
#include <string>
#include <vector>
#include <map>


#include <iostream>

class Pos {
private:
	float x, y, z;

public:
	Pos() : x(rand() % 101), y(rand() % 101), z(rand() % 101) {};
	float getX() { return x; }
	float getY() { return y; }
	float getZ() { return z; }
};

class Plane {
private:
	std::string name;
	Pos pos;
	float trajectory;
	float speed;
	float fuel;
	float conso;
public:
	Plane() : name("AA200"), pos(Pos()), trajectory(0), speed(0), fuel(100), conso(0) {}
	Pos getPos() { return pos; }
};