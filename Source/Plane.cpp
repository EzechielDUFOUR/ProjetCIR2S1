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


Plane::Plane(const std::string& code, double speed_max, APP* target, TWR* spawn, std::mutex& mtx, APP* app) : Agent(code, mtx), speed_max_(speed_max), app_(app), target_(target), state_(PARKED) {
	pos_.x = spawn->getPos().x; pos_.y = spawn->getPos().y; pos_.altitude=0;
	trajectory_.x = (target->getPos().x - spawn->getPos().x);
	trajectory_.y = (target->getPos().y - spawn->getPos().y);
	trajectory_.altitude = 0;
	make_unitary(&trajectory_);
	spawn->addParkedPlane(this);
}

void Plane::recomputeTrajectory(){
	trajectory_.x = (target_->getPos().x - app_->getPos().x);
	trajectory_.y = (target_->getPos().y - app_->getPos().y);
	trajectory_.altitude = 0;
	make_unitary(&trajectory_);
}
	
std::string Plane::getCode() const {
	return Agent::getCode();
}

Position Plane::getPos() const {
	return pos_;
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
}

void Plane::run() {
	while (running_) {
		double timer = 0.0;
		std::cout << "[" << code_ << "] : ";

		std::cout << pos_.x << ", ";
		std::cout << pos_.y << ", ";
		std::cout << pos_.altitude << " / SPEED : " << speed_ << std::endl;

		if (state_ != TAKINGOFF && pos_.altitude == 0 && requestTakeoff()){
			state_ = TAKINGOFF;
			trajectory_.altitude = 0.2;
			std::cout << "[" << code_ << "] Requests Takeoff from APP : " << app_->getCode() << std::endl;
		}

		if (state_ != LANDING && pos_.altitude != 0 && target_ == app_ && requestLanding()) {
			state_ = LANDING;
			trajectory_.altitude = -0.2;
			std::cout << "[" << code_ << "] Requests Landing to APP : " << app_->getCode() << std::endl;
		}

		if (state_ == TAKINGOFF) speed_ = (speed_ + 10 > speed_max_) ? speed_max_ : speed_ + 10;

		if (state_ == LANDING) {
			double dx = app_->getPos().x - pos_.x;
			double dy = app_->getPos().y - pos_.y;
			double distance_to_app = sqrt(dx * dx + dy * dy);

			if (distance_to_app <= 10.0 && distance_to_app >= 0) {
				double factor_altitude = (distance_to_app - 0.1) / 9.0;
				double factor_speed = distance_to_app / 9.0; 
				speed_ = (speed_max_) * factor_speed;
				pos_.altitude = 10.0 * factor_altitude;
			}

			speed_ = std::max(speed_, 0.0);
			pos_.altitude = std::max(pos_.altitude, 0.0);

			if (pos_.altitude == 0 && distance_to_app <= 0.01){
				state_ = PARKED;
				pos_.x = app_->getPos().x; pos_.y = app_->getPos().y, pos_.altitude=0;
				changeRunwayState();
				running_ = false;
				stop();
			}
		}

		pos_.x += trajectory_.x * speed_/3600;
		pos_.y += trajectory_.y * speed_/3600;
		pos_.altitude = (pos_.altitude + trajectory_.altitude > 10) ? pos_.altitude : (pos_.altitude + trajectory_.altitude < 0.1) ? 0 : pos_.altitude + trajectory_.altitude;

		if (pos_.altitude >= 10) {
			trajectory_.altitude = 0;
			state_ = FLYING;
			changeRunwayState();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

