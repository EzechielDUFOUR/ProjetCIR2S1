#include "../Headers/Plane.hpp"
#include "../Headers/APP.hpp"
#include "../Headers/TWR.hpp"
#include "../Headers/Journal.hpp"
#include "../Headers/Agent.hpp"
#include "../Headers/CCR.hpp"
#include <iostream>
#include <string>

CCR::CCR(const std::string& code, std::mutex& mtx) : Agent(code, mtx){}

void CCR::handoverToAPP(Plane* p, APP* app){
	app->receivePlane(p);
	deletePlane(p);
}

void CCR::run() {
	while (true) {

		for (auto p : FlyingPlanes_) {
			for (auto app : AllAPP_) {
				if (pow((p->getPos().x - app->getPos().x), 2) + pow((p->getPos().y - app->getPos().y), 2) <= pow(app->getRadius(), 2)) { // équation du cercle pour la range des APP
					if (app->receivePlane(p)) {}
					// std::cout << "[" << code_ << "]" << "Added [" << p->getCode() << "] to " << app->getCode() << std::endl;
				}
				else {
					if (app->deletePlane(p)) {}
					// std::cout << "[" << code_ << "]" << "Deleted [" << p->getCode() << "] from " << app->getCode() << std::endl;
				}
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void CCR::addPlane(Plane* p){
	FlyingPlanes_.push_back(p);
}

void CCR::addAPP(APP* app) {
	AllAPP_.push_back(app);
}

bool CCR::deletePlane(Plane* p) {
	auto it = std::find(FlyingPlanes_.begin(), FlyingPlanes_.end(), p);
	if (it != FlyingPlanes_.end()) {
		FlyingPlanes_.erase(it);
		return true;
	}
	return false;
}
