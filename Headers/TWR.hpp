#pragma once
#include "Agent.hpp"
#include <vector>
#include <mutex>
#include <atomic>
#include <deque>

class Plane;
class Journal;
struct Position;

class TWR : public Agent {
private:
	Position pos_;

	//Journal* journal_;

	std::atomic<bool> runwayFree_{true};

	std::vector<Plane*> parking_;
	const int parkingSize_;
	//APP* app_;

	std::deque<Plane*> waitingLine_;

public:
	TWR(const std::string& code, const int&parkingSize, Position& pos);

	void run() override;

	// Demandées par APP
	// void requestLanding(Plane* p);

	// Demandées par l'avion pour décoller
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
