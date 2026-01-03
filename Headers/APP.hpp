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
	APP(const std::string& code, Position& pos, TWR* twr, const double& radius, CCR* ccr);

	void run() override;

	// Un avion arrive depuis le CCR
	bool receivePlane(Plane* p);
	bool deletePlane(Plane* p);
	double getRadius();

	Position getPos();
	TWR* getTWR();
	bool requestTakeoff(Plane* p);
	bool requestLanding(Plane* p);

	void requestTakeoff2(Plane* p);
	void requestLanding2(Plane* p);

	void changeRunwayToOccupied(Plane* p);
	void changeRunwayToFree(Plane* p);

	APP* getRandomTarget();
};