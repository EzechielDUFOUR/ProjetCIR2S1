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

		for (auto p : FlyingPlanes_) {

			//std::cout << p->getCode() << " ";
			
			for (auto p2 : FlyingPlanes_) {
				if (p != p2) {
					Position pFuture, p2Future;
					// Calcul de la position future dans 5 ticks
					pFuture.x = p->getPos().x + 5*p->getTrajectory().x * p->getSpeed() / 360; pFuture.y = p->getPos().y + 5*p->getTrajectory().y * p->getSpeed() / 360;
					p2Future.x = p2->getPos().x + 5*p2->getTrajectory().x * p2->getSpeed() / 360; p2Future.y = p2->getPos().y + 5*p2->getTrajectory().y * p2->getSpeed() / 360;
					
					double dx = pFuture.x - p2Future.x, dy = pFuture.y - p2Future.y;
					double dist = sqrt(dx * dx + dy * dy);
					if (p->getState() != HOLDING && p2->getState() != HOLDING) {
						if (dist <= 10.0 && (p->getState() != EVASION || p2->getState() != EVASION) && abs(p->getPos().altitude - p2->getPos().altitude) <= 0.1) {
							p->rotateTrajectory(-30);
							p->changeState(EVASION);
							p2->rotateTrajectory(-30);
							p2->changeState(EVASION);
							std::cout << "Collision entre " << p->getCode() << " et " << p2->getCode() << std::endl;
						}
						else if (sqrt((p->getPos().x - p2->getPos().x) * (p->getPos().x - p2->getPos().x) + (p->getPos().y - p2->getPos().y) * (p->getPos().y - p2->getPos().y)) >= 10.0 && p->getState() == EVASION && p2->getState() == EVASION) {
							p->changeState(FLYING);
							p->changeTarget(p->getTarget());
							p2->changeState(FLYING);
							p2->changeTarget(p2->getTarget());
						}
					}

				}
			}

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
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
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

void CCR::detectCollision(){

}


