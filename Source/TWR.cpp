#include "../Headers/Plane.hpp"
#include "../Headers/APP.hpp"
#include "../Headers/TWR.hpp"
#include "../Headers/Journal.hpp"
#include "../Headers/Agent.hpp"
#include <iostream>
#include <string>
#include <algorithm>

TWR::TWR(const std::string& code, const int& parkingSize, Position& pos) : Agent(code), parkingSize_(parkingSize), pos_(pos), runwayFree_(true) {}

Position TWR::getPos() {
	return pos_;
}

void TWR::run() {
	while (running_) {
		// Départs aléatoires
		{
			std::vector<Plane*> parkingCopy;
			{
				std::lock_guard<std::mutex> lock(mtx_);
				parkingCopy = parking_;
			}
		
			for (auto p : parkingCopy) {
				if (p && p->getState() == PARKED && rand() % 1000 >= 998) {
					APP* randomTarget = p->getRandomTarget();
	
					{
						std::lock_guard<std::mutex> lock(mtx_);
						auto it = std::find(waitingLine_.begin(), waitingLine_.end(), p);
						if (it == waitingLine_.end()) {
							waitingLine_.push_back(p);
						}
					}
	
					// Appels externes SANS tenir le mutex de TWR
					p->changeTarget(randomTarget);
					if (!p->isRunning()) {
						p->start();
					}
					break;
				}
			}
		}

		// Traite la queue si piste libre
		{
			Plane* planeToTakeoff = nullptr;
			Plane* planeToLand = nullptr;
			APP* appForTakeoff = nullptr;
		
			{
				std::lock_guard<std::mutex> lock(mtx_);

				if (runwayFree_ && !waitingLine_.empty()) {
					Plane* p = waitingLine_.front();
		
					// Vérifier l'état actuel de l'avion AVANT de le retirer
					CurrentState currentState = p->getState();
		
					if (p && currentState == PARKED) {
						waitingLine_.pop_front();
						runwayFree_ = false;
						auto pit = std::find(parking_.begin(), parking_.end(), p);
						if (pit != parking_.end()) {
							parking_.erase(pit);
						}
						planeToTakeoff = p;
						appForTakeoff = p->getAPP();
					}
					else if (p && (currentState == FLYING || currentState == HOLDING)) {
						if (parking_.size() < static_cast<size_t>(parkingSize_)) {
							waitingLine_.pop_front();
							runwayFree_ = false;
							parking_.push_back(p);
							planeToLand = p;
						}
						else {
							// Parking plein : retirer de la tête et remettre en fin de file
							// Cela permet aux décollages de passer en priorité
							waitingLine_.pop_front();
							waitingLine_.push_back(p);
							std::cout << "[" << getCode() << "] Parking plein - " << p->getCode() << " remis en fin de file" << std::endl;
						}
					}
					else {
						// État invalide, retirer de la file
						waitingLine_.pop_front();
					}
				}
			}

			// Appels externes APRÈS avoir relâché le mutex de TWR
			if (planeToTakeoff && appForTakeoff) {
				if (!planeToTakeoff->isRunning()) {
					planeToTakeoff->start();
				}
				appForTakeoff->receivePlane(planeToTakeoff);
				planeToTakeoff->setAltitude(0.2);
				planeToTakeoff->changeState(TAKINGOFF);
				std::cout << "[" << getCode() << "] " << planeToTakeoff->getCode() << " autorise au decollage" << std::endl;
			}
		
			if (planeToLand) {
				planeToLand->setAltitude(-0.2);
				planeToLand->changeState(LANDING);
				std::cout << "[" << getCode() << "] " << planeToLand->getCode() << " autorise l'atterrissage" << std::endl;
			}
		}
		// Met en HOLDING ceux qui attendent
		{
			std::vector<Plane*> waitingCopy;
			{
				std::lock_guard<std::mutex> lock(mtx_);
				waitingCopy.assign(waitingLine_.begin(), waitingLine_.end());
			}
		
			// Appels externes SANS tenir le mutex de TWR
			for (auto p : waitingCopy) {
				if (p) {
					CurrentState state = p->getState();
					if (state != PARKED && state != LANDING && state != TAKINGOFF) {
						p->changeState(HOLDING);
					}
				}
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

bool TWR::requestTakeoff(Plane* p) {
	std::lock_guard<std::mutex> lock(mtx_);
	if (runwayFree_) {
		runwayFree_ = false;
		auto it = std::find(parking_.begin(), parking_.end(), p);
		if (it != parking_.end()) {
			parking_.erase(it);
		}
		return true;
	}
	return false;
}

bool TWR::requestLanding(Plane* p) {
	std::lock_guard<std::mutex> lock(mtx_);
	if (runwayFree_ && parking_.size() < static_cast<size_t>(parkingSize_)) {
		runwayFree_ = false;
		parking_.push_back(p);
		return true;
	}
	return false;
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
	std::lock_guard<std::mutex> lock(mtx_);
	parking_.push_back(p);
}

void TWR::deleteParkedPlane(Plane* p) {
	std::lock_guard<std::mutex> lock(mtx_);
	auto it = std::find(parking_.begin(), parking_.end(), p);
	if (it != parking_.end()) {
		parking_.erase(it);
	}
}

void TWR::changeRunwayToFree() {
	std::lock_guard<std::mutex> lock(mtx_);
	runwayFree_ = true;
}

void TWR::changeRunwayToOccupied() {
	std::lock_guard<std::mutex> lock(mtx_);
	runwayFree_ = false;
}

bool TWR::isParkingFull() {
	std::lock_guard<std::mutex> lock(mtx_);
	return parking_.size() >= static_cast<size_t>(parkingSize_);
}
