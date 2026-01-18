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

	Journal* journal_;

	bool runwayFree_ = true;  // Retiré volatile - la protection mutex suffit

	std::vector<Plane*> parking_;
	const int parkingSize_;
	//APP* app_;

	std::deque<Plane*> waitingLine_;

public:
	TWR(const std::string& code, const int& parkingSize, Position& pos, Journal* journal = nullptr);

	void run() override;

	// Demand�es par APP
	// void requestLanding(Plane* p);

	// Demand�es par l�avion pour d�coller
	bool requestTakeoff(Plane* p);
	bool requestLanding(Plane* p);

	void requestTakeoff2(Plane* p);
	void requestLanding2(Plane* p);

	Position getPos();


	void changeRunwayToFree();
	void changeRunwayToOccupied();

	void addParkedPlane(Plane* p);
	void deleteParkedPlane(Plane* p);

	bool isParkingFull();
};