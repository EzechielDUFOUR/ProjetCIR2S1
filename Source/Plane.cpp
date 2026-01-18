#include "../Headers/Plane.hpp"
#include "../Headers/APP.hpp"
#include "../Headers/TWR.hpp"
#include "../Headers/Journal.hpp"
#include "../Headers/Agent.hpp"
#include "../Headers/Functions.hpp"
#include <iostream>
#include <string>
#include <chrono>
#include <cmath>
#include <thread>
#include <sstream>
#include <iomanip>

Plane::Plane(const std::string& code, double speed_max, APP* target, TWR* spawn, APP* app, Journal* journal) 
	: Agent(code), speed_max_(speed_max), app_(app), target_(target), state_(PARKED), journal_(journal) {
	pos_.x = spawn->getPos().x;
	pos_.y = spawn->getPos().y;
	pos_.altitude = 0;
	trajectory_.x = (target->getPos().x - spawn->getPos().x);
	trajectory_.y = (target->getPos().y - spawn->getPos().y);
	trajectory_.altitude = 0;
	make_unitary(&trajectory_);
	spawn->addParkedPlane(this);
}

std::string Plane::getCode() const {
	return Agent::getCode();
}

Position Plane::getPos() const {
	std::lock_guard<std::mutex> lock(mtx_);
	return pos_;
}

CurrentState Plane::getState() {
	std::lock_guard<std::mutex> lock(mtx_);
	return state_;
}

bool Plane::requestTakeoff() {
	return app_->requestTakeoff(this);
}

void Plane::setAltitude(double altitude) {
	trajectory_.altitude = altitude;
}

bool Plane::requestLanding() {
	return app_->requestLanding(this);
}

void Plane::requestTakeoff2() {
	app_->requestTakeoff2(this);
}

void Plane::requestLanding2() {
	app_->requestLanding2(this);
}

void Plane::setAPP(APP* app) {
	app_ = app;
}

void Plane::changeRunwayToFree() {
	app_->getTWR()->changeRunwayToFree();
}

void Plane::changeRunwayToOccupied() {
	app_->getTWR()->changeRunwayToOccupied();
}

void Plane::changeTarget(APP* app) {
	std::lock_guard<std::mutex> lock(mtx_);
	target_ = app;
	trajectory_.x = (target_->getPos().x - pos_.x);
	trajectory_.y = (target_->getPos().y - pos_.y);
	trajectory_.altitude = 0;
	make_unitary(&trajectory_);
}

double Plane::getSpeed() {
	return speed_;
}

void Plane::changeState(CurrentState newstate) {
	std::lock_guard<std::mutex> lock(mtx_);
	state_ = newstate;
}

APP* Plane::getTarget() {
	return target_;
}

void Plane::rotateTrajectory(double angleDegrees) {
	double angleRad = angleDegrees * 3.1415 / 180.0;
	double x = trajectory_.x;
	double y = trajectory_.y;
	trajectory_.x = x * cos(angleRad) + y * sin(angleRad);
	trajectory_.y = -x * sin(angleRad) + y * cos(angleRad);
	make_unitary(&trajectory_);
}

APP* Plane::getRandomTarget() {
	return app_->getRandomTarget();
}

bool Plane::getEvasion() {
	return isEvading;
}

void Plane::changeEvasion() {
	isEvading = !isEvading;
}

APP* Plane::getAPP() {
	return app_;
}

void Plane::run() {
	bool hasRequestedTakeoff = false;
	bool hasRequestedLanding = false;
	
	while (running_) {
		CurrentState localState;
		APP* localApp = nullptr;
		APP* localTarget = nullptr;
		double localAltitude;
		Position localPos;
	
		// Lecture atomique de l'état
		{
			std::lock_guard<std::mutex> lock(mtx_);
			localState = state_;
			localApp = app_;
			localTarget = target_;
			localAltitude = pos_.altitude;
			localPos = pos_;
		}

		// Demande décollage (UNE SEULE FOIS par cycle PARKED)
		if (localState == PARKED && localAltitude == 0 && localApp != localTarget && localApp != nullptr && localTarget != nullptr) {
			if (!hasRequestedTakeoff) {
				requestTakeoff2();
				hasRequestedTakeoff = true;
			}
		} else if (localState != PARKED) {
			hasRequestedTakeoff = false;
		}

		// Demande atterrissage (UNE SEULE FOIS par approche)
		if ((localState == FLYING || localState == HOLDING) && localAltitude > 0 && localTarget == localApp && localTarget != nullptr) {
			double dx = localApp->getPos().x - localPos.x;
			double dy = localApp->getPos().y - localPos.y;
			double distSquared = dx * dx + dy * dy;
	
			if ((localState == HOLDING || distSquared <= 100) && !hasRequestedLanding) {
				requestLanding2();
				hasRequestedLanding = true;
			}
		} else if (localState != FLYING && localState != HOLDING) {
			hasRequestedLanding = false;
		}

		// HOLDING pattern
		if (localState == HOLDING && localTarget != nullptr) {
			if (localTarget != localApp) {
				std::lock_guard<std::mutex> lock(mtx_);
				state_ = FLYING;
			} else {
				const double HOLDING_RADIUS = 10.0;
				double dx = localPos.x - localTarget->getPos().x;
				double dy = localPos.y - localTarget->getPos().y;
				double dist = sqrt(dx * dx + dy * dy);
				if (dist < 0.01) dist = 0.01;

				double newTrajX = -dy / dist;
				double newTrajY = dx / dist;

				if (fabs(dist - HOLDING_RADIUS) > 0.1) {
					double correction = (HOLDING_RADIUS - dist) * 0.1;
					newTrajX += correction * dx / dist;
					newTrajY += correction * dy / dist;
				}

				std::lock_guard<std::mutex> lock(mtx_);
				trajectory_.x = newTrajX;
				trajectory_.y = newTrajY;
				make_unitary(&trajectory_);

				speed_ = (speed_ - 10 >= 300) ? speed_ - 10 : 300;
			}
		}

		// TAKINGOFF
		if (localState == TAKINGOFF) {
			bool shouldFreeRunway = false;
		
			{
				std::lock_guard<std::mutex> lock(mtx_);
				speed_ = (speed_ + 10 > speed_max_) ? speed_max_ : speed_ + 10;
		
				if (pos_.altitude >= 9.99) {
					trajectory_.altitude = 0;
					state_ = FLYING;
					//std::cout << "[" << code_ << "] passe en FLYING, piste liberee" << std::endl;
					shouldFreeRunway = true;
				}
			}
		
			if (shouldFreeRunway) {
				changeRunwayToFree();
			}
		}

		// Accélération en vol
		if (localState == FLYING) {
			std::lock_guard<std::mutex> lock(mtx_);
			if (speed_ < speed_max_) {
				speed_ = (speed_ + 10 > speed_max_) ? speed_max_ : speed_ + 10;
			}
		}

		// LANDING
		if (localState == LANDING && localTarget != nullptr) {
			double dx = localTarget->getPos().x - localPos.x;
			double dy = localTarget->getPos().y - localPos.y;
			double distance_to_app = sqrt(dx * dx + dy * dy);

			changeTarget(localTarget);

			if (distance_to_app <= 10.01 && distance_to_app >= 0) {
				double factor_altitude = (distance_to_app - 0.1) / 10.0;
				double factor_speed = distance_to_app / 10.0;
		
				std::lock_guard<std::mutex> lock(mtx_);
				speed_ = std::max(speed_, speed_max_) * factor_speed;
				pos_.altitude = 10.0 * factor_altitude;
				speed_ = std::max(speed_, 0.0);
				pos_.altitude = std::max(pos_.altitude, 0.0);
			}

			if (localAltitude <= 0.1 && distance_to_app <= 1.0) {
				bool shouldFreeRunway = false;
				std::string planeCode;
				std::string appCode;
		
				{
					std::lock_guard<std::mutex> lock(mtx_);
					if (app_ != nullptr) {
						state_ = PARKED;
						fuel_ = 1000;
						speed_ = 0;
						pos_.x = app_->getPos().x;
						pos_.y = app_->getPos().y;
						pos_.altitude = 0;
						planeCode = code_;
						appCode = app_->getCode();
						//std::cout << "[" << code_ << "] a atterri a " << app_->getCode() << std::endl;
						shouldFreeRunway = true;
					}
				}
		
				if (shouldFreeRunway) {
					changeRunwayToFree();
					// Logger l'atterrissage
					if (journal_) {
						std::ostringstream oss;
						oss << "[" << planeCode << "] atterrit à " << appCode;
						journal_->logEvent(oss.str());
					}
				}
			}
		}

		// Mouvement
		{
			std::lock_guard<std::mutex> lock(mtx_);
			
			pos_.x += trajectory_.x * speed_ / 360;
			pos_.y += trajectory_.y * speed_ / 360;

			if (state_ != LANDING) {
				pos_.altitude = (pos_.altitude + trajectory_.altitude > 10) ? 10 :
					(pos_.altitude + trajectory_.altitude < 0.1) ? 0 :
					pos_.altitude + trajectory_.altitude;
			}

			fuel_ -= consumption_ * speed_ / 360;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}
