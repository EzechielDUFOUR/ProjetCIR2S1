#include "../Headers/Plane.hpp"
#include "../Headers/APP.hpp"
#include "../Headers/TWR.hpp"
#include "../Headers/Journal.hpp"
#include "../Headers/Agent.hpp"
#include <iostream>
#include <string>
#include <algorithm>

APP::APP(const std::string& code, Position& pos, TWR* twr, const double& radius, std::mutex& mtx, CCR* ccr) : Agent(code, mtx), pos_(pos), radius_(radius), twr_(twr), ccr_(ccr){}

Position APP::getPos() {
	return pos_;
}

double APP::getRadius() {
	return radius_;
}

bool APP::receivePlane(Plane* p){
	auto it = std::find(PlanesInRange_.begin(), PlanesInRange_.end(), p);
	if (it == PlanesInRange_.end()) {
		PlanesInRange_.push_back(p);
		p->setAPP(this);
		return true;
	}
	return false;
}

bool APP::deletePlane(Plane* p){
	auto it = std::find(PlanesInRange_.begin(), PlanesInRange_.end(), p);
	if (it != PlanesInRange_.end()) {
		PlanesInRange_.erase(it);
		return true;
	}
	return false;
}

void APP::run() {
	std::cout << code_ << std::endl;
}

bool APP::requestTakeoff(Plane* p){
	if (twr_->requestTakeoff(p)){
		receivePlane(p);
		ccr_->addPlane(p);
		return true;
	}
	return false;
}

bool APP::requestLanding(Plane* p) {
	if (twr_->requestTakeoff(p)) {
		deletePlane(p);
		ccr_->deletePlane(p);
		return true;
	}
	return false;
}