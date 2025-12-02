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

        //mtx_.lock();
        //std::cout << "[" << getCode() << "] " << parkingSize_ - parking_.size() << " places restantes / " << ((runwayFree_) ? "Runway Free" : "Runway Occupied") << std::endl;
        //mtx_.unlock();

        // Départs aléatoires (sans mutex, fait avant le lock)
        for (auto p : parking_) {
            if (p->getState() == PARKED && rand() % 100 > 80) {
                p->changeTarget(p->getRandomTarget());
                requestTakeoff2(p); // Ajoute à la queue
                p->start(); // Démarre le thread
                break;
            }
        }

        // Traite la queue si piste libre
        if (runwayFree_ && !waitingLine_.empty()) {
            Plane* p = waitingLine_.front();
            waitingLine_.pop_front();
            runwayFree_ = false;

            if (p->getState() == PARKED) {
                p->getAPP()->receivePlane(p);
                deleteParkedPlane(p);
                p->changeState(TAKINGOFF);
                p->setAltitude(0.2);
            }
            else if (p->getState() == FLYING || p->getState() == HOLDING) {
                addParkedPlane(p);
                p->changeState(LANDING);
                p->setAltitude(-0.2);
            }
        }

        // Met en HOLDING ceux qui attendent
        for (auto p : waitingLine_) {
            if (p && p->getState() != PARKED && p->getState() != LANDING) {
                p->changeState(HOLDING);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
	mtx_.lock();
	bool result = false;
	if (runwayFree_ && parking_.size() < parkingSize_) {
		runwayFree_ = false;
		addParkedPlane(p);
		result = true;
	}
	mtx_.unlock();
	return result;
}

void TWR::requestTakeoff2(Plane* p) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = std::find(waitingLine_.begin(), waitingLine_.end(), p);
    if (it == waitingLine_.end()) {
        waitingLine_.push_back(p);
    }
}

void TWR::requestLanding2(Plane* p) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = std::find(waitingLine_.begin(), waitingLine_.end(), p);
    if (it == waitingLine_.end()) {
        waitingLine_.push_back(p);
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

void TWR::changeRunwayToFree() {
    std::lock_guard<std::mutex> lock(mtx_);
    runwayFree_ = true;
    std::cout << "Runway freed : " << getCode() << std::endl;
}

void TWR::changeRunwayToOccupied() {
    std::lock_guard<std::mutex> lock(mtx_);
    runwayFree_ = false;
}


bool TWR::isParkingFull() {
	return parking_.size() >= parkingSize_;
}