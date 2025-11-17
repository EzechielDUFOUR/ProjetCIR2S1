#pragma once
#include "Agent.hpp"
#include <string>
#include <vector>
#include <mutex>

class Plane;
class APP;
class Journal;

class CCR : public Agent {
public:
	CCR(const std::string& name, APP* app, Journal* journal);
	~CCR();

	void run() override;

	// Création d'un vol
	void spawnFlight(const std::string& code);

private:
	mutable std::mutex mtx_;

	APP* app_;
	Journal* journal_;
	std::vector<Plane*> enRoutePlanes_;

	void handoverToAPP(Plane* p);
};
