#include "../Headers/Plane.hpp"
#include "../Headers/APP.hpp"
#include "../Headers/TWR.hpp"
#include "../Headers/Journal.hpp"
#include "../Headers/Agent.hpp"
#include <iostream>
#include <string>
#include <algorithm>

TWR::TWR(const std::string& code, const int& parkingSize, Position& pos, std::mutex& mtx) : Agent(code, mtx), parkingSize_(parkingSize), pos_(pos), runwayFree_(true) {}

Position TWR::getPos() {
	return pos_;
}

void TWR::run() {
	while (running_) {
		/*mtx_.lock();
		std::cout << "[" << code_ << "] " << ((runwayFree_) ? "Piste Dispo " : "Piste Indispo ") << parkingSize_ - parking_.size() << " places restantes !" << std::endl;
		std::cout << "Parked Planes : ";
		for (auto p : parking_) {
			std::cout << p->getCode() << " ";
		}
		std::cout << std::endl;
		mtx_.unlock();*/
		for (auto p : parking_) {
			if (p->getState() == PARKED) {
				if (rand() % 100 > 80) {
					p->changeTarget(p->getRandomTarget());
					p->start();
					break;
					//std::cout << p->getCode() << " / " << p->getTarget()->getCode();
				}
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

bool TWR::requestTakeoff(Plane* p) {
	if (runwayFree_) {
		runwayFree_ = false;
		deleteParkedPlane(p);
		return true;
	}
	else {
		return false;
	}
}

bool TWR::requestLanding(Plane* p) {
	if (runwayFree_ && parking_.size() < parkingSize_) {
		runwayFree_ = false;
		addParkedPlane(p);
		return true;
	}
	else {
		//std::cout << "Landing Refused " << ((runwayFree_) ? "Piste Dispo " : "Piste Indispo ") << parkingSize_ - parking_.size() << " places restantes !" << std::endl;
		return false;
	}
}

void TWR::addParkedPlane(Plane* p) {
	parking_.push_back(p);
}

void TWR::deleteParkedPlane(Plane* p) {
	auto it = std::find(parking_.begin(), parking_.end(), p);
	if (it != parking_.end()) {
		parking_.erase(it);
	}
}

void TWR::changeRunwayState() {
	runwayFree_ = !runwayFree_;
}

bool TWR::isParkingFull() {
	return parking_.size() >= parkingSize_;
}