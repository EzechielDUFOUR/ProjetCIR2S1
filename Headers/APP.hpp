#pragma once
#include "Agent.hpp"
#include "CCR.hpp"
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
	CCR* ccr_ = nullptr;

public:
	APP(const std::string& code, Position& pos, TWR* twr, const double& radius, std::mutex& mtx, CCR* ccr);

	void run() override;

	// Un avion arrive depuis le CCR
	bool receivePlane(Plane* p);
	bool deletePlane(Plane* p);
	double getRadius();

	Position getPos();
	TWR* getTWR();
	bool requestTakeoff(Plane* p);
	bool requestLanding(Plane* p);
	void changeRunwayState(Plane* p);
};