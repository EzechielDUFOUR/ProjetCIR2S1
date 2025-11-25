#include "../Headers/Plane.hpp"
#include "../Headers/APP.hpp"
#include "../Headers/TWR.hpp"
#include "../Headers/Journal.hpp"
#include "../Headers/Agent.hpp"
#include <iostream>
#include <string>
#include <algorithm>

TWR::TWR(const std::string& code, const int&parkingSize, Position& pos, std::mutex& mtx) : Agent(code, mtx), parkingSize_(parkingSize), pos_(pos), runwayFree_(true){}

Position TWR::getPos(){
	return pos_;
}

void TWR::run(){
	std::cout << code_ << std::endl;
}

bool TWR::requestTakeoff(Plane* p){
	if (runwayFree_){
		runwayFree_ = false;
		deleteParkedPlane(p);
		return true;
	}
	else {
		return false;
	}
}

bool TWR::requestLanding(Plane* p) {
	if (runwayFree_ && parking_.size() != parkingSize_) {
		runwayFree_ = false;
		addParkedPlane(p);
		return true;
	}
	else {
		return false;
	}
}

void TWR::addParkedPlane(Plane* p){
	parking_.push_back(p);
}

void TWR::deleteParkedPlane(Plane* p) {
	auto it = std::find(parking_.begin(), parking_.end(), p);
	if (it != parking_.end()) {
		parking_.erase(it);
	}
}

void TWR::changeRunwayState(){
	runwayFree_ = !runwayFree_;
}