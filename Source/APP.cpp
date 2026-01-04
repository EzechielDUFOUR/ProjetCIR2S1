#include "../Headers/Plane.hpp"
#include "../Headers/APP.hpp"
#include "../Headers/TWR.hpp"
#include "../Headers/Journal.hpp"
#include "../Headers/Agent.hpp"
#include <iostream>
#include <string>
#include <algorithm>
#include <cmath>

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
	
	// Appel externe APR�S avoir rel�ch� le mutex
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
			CurrentState pState = p->getState();
			double dx = p->getPos().x - pos_.x;
			double dy = p->getPos().y - pos_.y;

			// Ne pas retirer les avions en HOLDING ou LANDING - ils sont en train d'atterrir ici
			if (dx * dx + dy * dy > radius_ * radius_ && pState != HOLDING && pState != LANDING) {
				toRemove.push_back(p);
			}

			// Ignorer les avions en HOLDING pour la détection de collision - ils sont en attente d'atterrissage
			if (pState == HOLDING || pState == LANDING) continue;

			// Gérer les avions en état EVASION pour les remettre en FLYING
			if (pState == EVASION) {
				bool hasNearbyCollision = false;
				double minDist = 1000.0;
				
				// Vérifier s'il y a encore un risque de collision
				for (auto p2 : planesCopy) {
					if (p != p2 && p2) {
						CurrentState p2State = p2->getState();
						// Ne pas considérer les avions en HOLDING ou LANDING pour la détection de collision
						if (p2State == HOLDING || p2State == LANDING) continue;
						
						Position pFuture, p2Future;
						pFuture.x = p->getPos().x + 5 * p->getTrajectory().x * p->getSpeed() / 360;
						pFuture.y = p->getPos().y + 5 * p->getTrajectory().y * p->getSpeed() / 360;
						p2Future.x = p2->getPos().x + 5 * p2->getTrajectory().x * p2->getSpeed() / 360;
						p2Future.y = p2->getPos().y + 5 * p2->getTrajectory().y * p2->getSpeed() / 360;
						
						double dxFuture = pFuture.x - p2Future.x, dyFuture = pFuture.y - p2Future.y;
						double distFuture = sqrt(dxFuture * dxFuture + dyFuture * dyFuture);
						
						dxFuture = p->getPos().x - p2->getPos().x;
						dyFuture = p->getPos().y - p2->getPos().y;
						double distCurrent = sqrt(dxFuture * dxFuture + dyFuture * dyFuture);
						
						if (distFuture <= 10.0 && std::abs(p->getPos().altitude - p2->getPos().altitude) <= 0.1) {
							hasNearbyCollision = true;
							minDist = std::min(minDist, distCurrent);
						}
					}
				}
				
				// Si pas de collision proche et suffisamment séparé, retourner vers l'objectif
				if (!hasNearbyCollision || minDist >= 30.0) {
					APP* target = p->getTarget();
					if (target != nullptr) {
						p->changeState(FLYING);
						p->changeTarget(target);
						std::cout << "[" << code_ << "] " << p->getCode() << " sortie d'evasion, retour vers " << target->getCode() << std::endl;
					}
				}
				continue; // Ne pas faire la détection de collision normale pour les avions en EVASION (déjà géré)
			}

			// Détection de collision normale (uniquement pour FLYING)
			if (pState == FLYING) {
				for (auto p2 : planesCopy) {
					if (p != p2 && p2) {
						CurrentState p2State = p2->getState();
						// Ne pas déclencher d'évasion si un des avions est en HOLDING ou LANDING
						if (p2State == HOLDING || p2State == LANDING || p2State == EVASION) continue;
						
						Position pFuture, p2Future;
						// Calcul de la position future dans 5 ticks
						pFuture.x = p->getPos().x + 5 * p->getTrajectory().x * p->getSpeed() / 360;
						pFuture.y = p->getPos().y + 5 * p->getTrajectory().y * p->getSpeed() / 360;
						p2Future.x = p2->getPos().x + 5 * p2->getTrajectory().x * p2->getSpeed() / 360;
						p2Future.y = p2->getPos().y + 5 * p2->getTrajectory().y * p2->getSpeed() / 360;
						
						double dxFuture = pFuture.x - p2Future.x, dyFuture = pFuture.y - p2Future.y;
						double dist = sqrt(dxFuture * dxFuture + dyFuture * dyFuture);
						
						if (dist <= 10.0 && std::abs(p->getPos().altitude - p2->getPos().altitude) <= 0.1) {
							p->rotateTrajectory(-30);
							p->changeState(EVASION);
							p2->rotateTrajectory(-30);
							p2->changeState(EVASION);
							std::cout << "[" << code_ << "] Collision entre " << p->getCode() << " et " << p2->getCode() << std::endl;
						}
					}
				}
			}
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