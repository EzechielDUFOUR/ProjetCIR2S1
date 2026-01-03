#pragma once
#include "Agent.hpp"
#include <string>
#include <vector>
#include <mutex>

class Plane;
class APP;
class Journal;

class CCR : public Agent {
private:

	std::vector<APP*> AllAPP_;
	// Journal* journal_;
	std::vector<Plane*> FlyingPlanes_;

public:
	CCR(const std::string& code);

	void run() override;

	void handoverToAPP(Plane* p, APP* app);
	
	void addPlane(Plane* p);

	void addAPP(APP* app);

	bool deletePlane(Plane* p);

	void detectCollision();

	APP* getRandomTarget(APP* app);

	std::vector<APP*> getAPPS();
};
