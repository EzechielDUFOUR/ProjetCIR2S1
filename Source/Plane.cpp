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

Plane::Plane(const std::string& code, double speed_max, APP* target, TWR* spawn, std::mutex& mtx, APP* app) : Agent(code, mtx), speed_max_(speed_max), app_(app), target_(target), state_(PARKED) {
	pos_.x = spawn->getPos().x; pos_.y = spawn->getPos().y; pos_.altitude=0;
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
	return pos_;
}

CurrentState Plane::getState() {
	return state_;
}

bool Plane::requestTakeoff(){
	return app_->requestTakeoff(this);
}

bool Plane::requestLanding(){
	return app_->requestLanding(this);
}

void Plane::setAPP(APP* app) {
	app_ = app;
}

void Plane::changeRunwayState(){
	app_->changeRunwayState(this);
}

void Plane::changeTarget(APP* app){
	target_ = app;
	trajectory_.x = (target_->getPos().x - getPos().x);
	trajectory_.y = (target_->getPos().y - getPos().y);
	trajectory_.altitude = 0;
	make_unitary(&trajectory_);
}

double Plane::getSpeed(){
	return speed_;
}

void Plane::changeState(CurrentState newstate) {
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
	// Normalise le vecteur si besoin
	make_unitary(&trajectory_);
}

APP* Plane::getRandomTarget() {
	return app_->getRandomTarget();
}

void Plane::run() {
	while (running_) {
		std::ostringstream msg;
		msg << "[" << code_ << "] : ";

		msg << pos_.x << ", ";
		msg << pos_.y << ", ";
		msg << pos_.altitude << " / SPEED : " << speed_;
		msg << " / FUEL : " << fuel_;
		msg << " / STATE : ";
		switch (state_){
			case FLYING: msg << " FLYING" << std::endl; break;
			case TAKINGOFF: msg << " TAKINGOFF" << std::endl; break;
			case EMERGENCY: msg << " EMERGENCY" << std::endl; break;
			case LANDING: msg << " LANDING" << std::endl; break;
			case PARKED: msg << " PARKED" << std::endl; break;
			default: msg << "RIEN !" << std::endl; break;
		}

		mtx_.lock();
		std::cout << msg.str();

		if (state_ == PARKED && pos_.altitude == 0 && app_ != target_) {
			if (requestTakeoff()) {
				state_ = TAKINGOFF;
				trajectory_.altitude = 0.1;
				//std::cout << "[" << code_ << "] Requests Takeoff from APP : " << app_->getCode() << std::endl;
			}
			else {
				//std::cout << "[" << code_ << "] Requests Takeoff REFUSED : " << std::endl;
			}
		}

		if ((state_ == FLYING || state_ == HOLDING) && pos_.altitude != 0 && target_ == app_) {
			if ((pow((app_->getPos().x - pos_.x), 2) + pow((app_->getPos().y - pos_.y), 2) <= 100) || state_ == HOLDING){
				if (requestLanding()) {
					state_ = LANDING;
					changeTarget(target_);
					trajectory_.altitude = -0.2;
					//std::cout << "[" << code_ << "] Requests Landing to APP : " << app_->getCode() << std::endl;
				}
				else {
					state_ = HOLDING;
					//std::cout << "[" << code_ << "] Requests Landing REFUSED" << std::endl;
				}
			}
		}
		mtx_.unlock();

		/*if (fuel_ < 150) {
			state_ = EMERGENCY;
		}*/

		if (state_ == HOLDING) {
			if (target_ != app_) state_ = FLYING;
			float holdingAngle = atan2(pos_.y - target_->getPos().y, pos_.x - target_->getPos().x);
			trajectory_.x = -sin(holdingAngle);
			trajectory_.y = cos(holdingAngle);
		}

		if (state_ == TAKINGOFF) { 
			speed_ = (speed_ + 10 > speed_max_) ? speed_max_ : speed_ + 10; 
			if (pos_.altitude >= 9.99 && speed_==speed_max_) {
				trajectory_.altitude = 0;
				state_ = FLYING;
				changeRunwayState();
				
			}
		}

		double dx = app_->getPos().x - pos_.x;
		double dy = app_->getPos().y - pos_.y;
		double distance_to_app = sqrt(dx * dx + dy * dy);

		if (state_ == LANDING) {

			if (distance_to_app <= 10.0 && distance_to_app >= 0) {
				double factor_altitude = (distance_to_app - 0.1) / 10.0;
				double factor_speed = distance_to_app / 10.0; 
				speed_ = (speed_max_) * factor_speed;
				pos_.altitude = 10.0 * factor_altitude;
			}

			speed_ = std::max(speed_, 0.0);
			pos_.altitude = std::max(pos_.altitude, 0.0);
		}

		if (state_ != TAKINGOFF && pos_.altitude == 0 && distance_to_app <= 0.01) {
			state_ = PARKED;
			fuel_ = 1000;
			pos_.x = app_->getPos().x; pos_.y = app_->getPos().y, pos_.altitude = 0;
			changeRunwayState();
			running_ = false;
			stop();
		}

		pos_.x += trajectory_.x * speed_/360;
		pos_.y += trajectory_.y * speed_/360;
		if (state_ != LANDING) {
			pos_.altitude = (pos_.altitude + trajectory_.altitude > 10) ? pos_.altitude : (pos_.altitude + trajectory_.altitude < 0.1) ? 0 : pos_.altitude + trajectory_.altitude;
		}
		fuel_ -= consumption_ * speed_/360;

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

