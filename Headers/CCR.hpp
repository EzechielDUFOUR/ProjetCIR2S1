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
	
	~CCR();

	void run() override;

	void handoverToAPP(Plane* p);
};
