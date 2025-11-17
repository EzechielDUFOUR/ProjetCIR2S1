#pragma once
#include "Agent.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <mutex>

class Plane;
class Journal;

class TWR : public Agent {
public:
	TWR(const std::string& name, Journal* journal);
	~TWR();

	void run() override;

	// Demandées par APP
	void requestLanding(Plane* p);

	// Demandées par l’avion pour décoller
	void requestTakeoff(Plane* p);

private:
	mutable std::mutex mtx_;
	Position pos;

	Journal* journal_;

	bool runwayFree_ = true;
	std::vector<bool> parking_;  // parking_[i] = occupé ?
	std::vector<Plane*> groundPlanes_;

	void authorizeLanding(Plane* p);
	void authorizeTakeoff(Plane* p);
	int assignParkingSlot();

	void drawScene(const std::vector<Plane*>& planes);
};
