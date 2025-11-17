#pragma once
#include "Agent.hpp"
#include <vector>
#include <queue>
#include <mutex>

class Plane;
class TWR;
class Journal;

class APP : public Agent {
public:
	APP(const std::string& name, Journal* journal);
	~APP();

	void run() override;

	// Un avion arrive depuis le CCR
	void receivePlane(Plane* p);

	// L’APP doit connaître sa TWR
	void setTWR(TWR* twr);

private:
	mutable std::mutex mtx_;

	Journal* journal_;
	TWR* twr_ = nullptr;

	std::vector<Plane*> approachPlanes_;
	std::queue<Plane*> landingQueue_;

	void processArrivals();
	void assignApproach(Plane* p);
	void requestRunway(Plane* p);
};
