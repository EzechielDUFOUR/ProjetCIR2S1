#include "../Headers/Plane.hpp"
#include "../Headers/APP.hpp"
#include "../Headers/TWR.hpp"
#include "../Headers/Journal.hpp"
#include "../Headers/Agent.hpp"
#include "../Headers/CCR.hpp"
#include <iostream>
#include <string>
#include <cmath>

CCR::CCR(const std::string& code) : Agent(code){}

void CCR::handoverToAPP(Plane* p, APP* app){
	app->receivePlane(p);
	deletePlane(p);
}

APP* CCR::getRandomTarget(APP* app) {
	APP* ptr = AllAPP_[rand() % AllAPP_.size()];
	while (ptr == app) {
		ptr = AllAPP_[rand() % AllAPP_.size()];
	}
	return ptr;
}

std::vector<APP*> CCR::getAPPS() {
	return AllAPP_;
}

void CCR::run() {
	while (running_) {
		// Copie locale pour éviter les modifications pendant l'itération
		std::vector<Plane*> flyingCopy;
		std::vector<APP*> appsCopy;
		{
			std::lock_guard<std::mutex> lock(mtx_);
			flyingCopy = FlyingPlanes_;
			appsCopy = AllAPP_;
		}

		std::vector<std::pair<Plane*, APP*>> toHandover;
		for (auto p : flyingCopy) {
			if (!p) continue;
			CurrentState pState = p->getState();
			
			// Gérer les avions en état EVASION pour les remettre en FLYING
			if (pState == EVASION) {
				bool hasNearbyCollision = false;
				double minDist = 1000.0;
				
				// Vérifier s'il y a encore un risque de collision
				for (auto p2 : flyingCopy) {
					if (p != p2 && p2) {
						Position pFuture, p2Future;
						pFuture.x = p->getPos().x + 5 * p->getTrajectory().x * p->getSpeed() / 360;
						pFuture.y = p->getPos().y + 5 * p->getTrajectory().y * p->getSpeed() / 360;
						p2Future.x = p2->getPos().x + 5 * p2->getTrajectory().x * p2->getSpeed() / 360;
						p2Future.y = p2->getPos().y + 5 * p2->getTrajectory().y * p2->getSpeed() / 360;
						
						double dx = pFuture.x - p2Future.x, dy = pFuture.y - p2Future.y;
						double distFuture = sqrt(dx * dx + dy * dy);
						
						dx = p->getPos().x - p2->getPos().x;
						dy = p->getPos().y - p2->getPos().y;
						double distCurrent = sqrt(dx * dx + dy * dy);
						
						if (distFuture <= 10.0 && std::abs(p->getPos().altitude - p2->getPos().altitude) <= 0.1 && p2->getState() != HOLDING) {
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
						std::cout << "[" << p->getCode() << "] Sortie d'evasion, retour vers " << target->getCode() << std::endl;
					}
				}
			}
			
			// Ne déroute pas les avions déjà en approche/holding/landing
			if (pState != FLYING && pState != EVASION) continue;

		//std::cout << p->getCode() << " ";
		
		for (auto p2 : flyingCopy) {
			if (p != p2) {
				Position pFuture, p2Future;
				// Calcul de la position future dans 5 ticks
				pFuture.x = p->getPos().x + 5 * p->getTrajectory().x * p->getSpeed() / 360;
				pFuture.y = p->getPos().y + 5 * p->getTrajectory().y * p->getSpeed() / 360;
				p2Future.x = p2->getPos().x + 5 * p2->getTrajectory().x * p2->getSpeed() / 360;
				p2Future.y = p2->getPos().y + 5 * p2->getTrajectory().y * p2->getSpeed() / 360;
				
				double dx = pFuture.x - p2Future.x, dy = pFuture.y - p2Future.y;
				double dist = sqrt(dx * dx + dy * dy);
				if (p->getState() != HOLDING && p2->getState() != HOLDING) {
					if (dist <= 10.0 && (p->getState() != EVASION || p2->getState() != EVASION) && std::abs(p->getPos().altitude - p2->getPos().altitude) <= 0.1) {
						p->rotateTrajectory(-30);
						p->changeState(EVASION);
						p2->rotateTrajectory(-30);
						p2->changeState(EVASION);
						std::cout << "Collision entre " << p->getCode() << " et " << p2->getCode() << std::endl;
					}
				}
			}
		}

			for (auto app : appsCopy) {
				if (app && pow((p->getPos().x - app->getPos().x), 2) + pow((p->getPos().y - app->getPos().y), 2) <= pow(app->getRadius(), 2)) {
					toHandover.push_back({p, app});
					break;
				}
			}
		}

		for (auto& pair : toHandover) {
			handoverToAPP(pair.first, pair.second);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void CCR::addPlane(Plane* p){
	std::lock_guard<std::mutex> lock(mtx_);
	auto it = std::find(FlyingPlanes_.begin(), FlyingPlanes_.end(), p);
	if (it == FlyingPlanes_.end()) {
		FlyingPlanes_.push_back(p);
	}
}

void CCR::addAPP(APP* app) {
	std::lock_guard<std::mutex> lock(mtx_);
	AllAPP_.push_back(app);
}

bool CCR::deletePlane(Plane* p) {
	std::lock_guard<std::mutex> lock(mtx_);
	auto it = std::find(FlyingPlanes_.begin(), FlyingPlanes_.end(), p);
	if (it != FlyingPlanes_.end()) {
		FlyingPlanes_.erase(it);
		return true;
	}
	return false;
}