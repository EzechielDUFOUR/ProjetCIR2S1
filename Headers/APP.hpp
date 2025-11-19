#pragma once
#include "Agent.hpp"
#include <vector>
#include <queue>
#include <mutex>

class Plane;
class TWR;
class Journal;
struct Position;

class APP : public Agent {
private:
	Position pos_;
	// Journal* journal_;
	TWR* twr_ = nullptr;
	std::vector<Plane*> PlanesInRange_;
	const double radius_; // rayon d'action de l'APP

public:
	APP(const std::string& code, Position& pos, TWR* twr, const double& radius, std::mutex& mtx);

	void run() override;

	// Un avion arrive depuis le CCR
	void receivePlane(Plane* p);

	Position getPos();
};