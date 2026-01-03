#include "../Headers/Plane.hpp"
#include "../Headers/APP.hpp"
#include "../Headers/TWR.hpp"
#include "../Headers/Journal.hpp"
#include "../Headers/Agent.hpp"
#include <iostream>
#include <string>
#include <algorithm>

APP::APP(const std::string& code, Position& pos, TWR* twr, const double& radius, CCR* ccr) : Agent(code), pos_(pos), radius_(radius), twr_(twr), ccr_(ccr){}

Position APP::getPos() {
	return pos_;
}

double APP::getRadius() {
	return radius_;
}

bool APP::receivePlane(Plane* p){
	{
		std::lock_guard<std::mutex> lock(mtx_);
		auto it = std::find(PlanesInRange_.begin(), PlanesInRange_.end(), p);
		if (it == PlanesInRange_.end()) {
			PlanesInRange_.push_back(p);
		} else {
			return false;
		}
	}
	
	// Appel externe APRÈS avoir relâché le mutex
	p->setAPP(this);
	return true;
}

bool APP::deletePlane(Plane* p){
	std::lock_guard<std::mutex> lock(mtx_);
	auto it = std::find(PlanesInRange_.begin(), PlanesInRange_.end(), p);
	if (it != PlanesInRange_.end()) {
		PlanesInRange_.erase(it);
		return true;
	}
	return false;
}

void APP::run() {
	while (running_) {
		std::vector<Plane*> toRemove;
		std::vector<Plane*> planesCopy;
		{
			std::lock_guard<std::mutex> lock(mtx_);
			planesCopy = PlanesInRange_;
		}
		for (auto p : planesCopy) {
			if (!p) continue;
			double dx = p->getPos().x - pos_.x;
			double dy = p->getPos().y - pos_.y;

			// Ne pas retirer les avions en HOLDING ou LANDING - ils sont en train d'atterrir ici
			if (dx * dx + dy * dy > radius_ * radius_ && p->getState() != HOLDING && p->getState() != LANDING) {
				toRemove.push_back(p);
			}

		//	for (auto p2 : PlanesInRange_) {
		//		if (p != p2) {
		//			Position pFuture, p2Future;
		//			// Calcul de la position future dans 5 ticks
		//			pFuture.x = p->getPos().x + 5 * p->getTrajectory().x * p->getSpeed() / 360; pFuture.y = p->getPos().y + 5 * p->getTrajectory().y * p->getSpeed() / 360;
		//			p2Future.x = p2->getPos().x + 5 * p2->getTrajectory().x * p2->getSpeed() / 360; p2Future.y = p2->getPos().y + 5 * p2->getTrajectory().y * p2->getSpeed() / 360;

		//			double dx = pFuture.x - p2Future.x, dy = pFuture.y - p2Future.y;
		//			double dist = sqrt(dx * dx + dy * dy);
		//			if (p->getState() != HOLDING && p2->getState() != HOLDING) {
		//				if (dist <= 10.0 && (p->getState() != EVASION || p2->getState() != EVASION) && abs(p->getPos().altitude - p2->getPos().altitude) <= 0.1) {
		//					p->rotateTrajectory(-30);
		//					p->changeState(EVASION);
		//					p2->rotateTrajectory(-30);
		//					p2->changeState(EVASION);
		//					std::cout << "Collision entre " << p->getCode() << " et " << p2->getCode() << std::endl;
		//				}
		//				else if (sqrt((p->getPos().x - p2->getPos().x) * (p->getPos().x - p2->getPos().x) + (p->getPos().y - p2->getPos().y) * (p->getPos().y - p2->getPos().y)) >= 10.0 && p->getState() == EVASION && p2->getState() == EVASION) {
		//					p->changeState(FLYING);
		//					p->changeTarget(p->getTarget());
		//					p2->changeState(FLYING);
		//					p2->changeTarget(p2->getTarget());
		//				}
		//			}

		//		}
		//	}
		}

		// retrait hors boucle
		for (auto p : toRemove) {
			if (p) {
				ccr_->addPlane(p);
				deletePlane(p);
				//std::cout << "[" << code_ << "]" << "Removed [" << p->getCode() << "] from " << getCode() << std::endl;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

bool APP::requestTakeoff(Plane* p){
	if (twr_ != nullptr && twr_->requestTakeoff(p)){
		receivePlane(p);
		ccr_->addPlane(p);
		return true;
	}
	return false;
}

bool APP::requestLanding(Plane* p) {
	if (twr_->requestLanding(p)) {
		deletePlane(p);
		ccr_->deletePlane(p);
		return true;
	}
	return false;
}

void APP::requestTakeoff2(Plane* p) {
	if (twr_ != nullptr) twr_->requestTakeoff2(p);
}

void APP::requestLanding2(Plane* p) {
	if (twr_ != nullptr) twr_->requestLanding2(p);
}

void APP::changeRunwayToFree(Plane* p){
	twr_->changeRunwayToFree();
}

void APP::changeRunwayToOccupied(Plane* p) {
	twr_->changeRunwayToOccupied();
}

TWR* APP::getTWR(){
	return twr_;
}

APP* APP::getRandomTarget() {
	return ccr_->getRandomTarget(this);
}