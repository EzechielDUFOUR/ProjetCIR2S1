#pragma once
#include "Agent.hpp"
#include <vector>
#include <mutex>

class Plane;
class Journal;
struct Position;

class TWR : public Agent {
private:
	Position pos_;

	//Journal* journal_;

	// bool runwayFree_ = true;

	std::vector<Plane*> parking_;
	const int parkingSize_;

public:
	TWR(const std::string& code, const int&parkingSize, Position& pos);

	~TWR();

	void run() override;

	// Demandées par APP
	// void requestLanding(Plane* p);

	// Demandées par l’avion pour décoller
	// void requestTakeoff(Plane* p);

	Position getPos();
};
