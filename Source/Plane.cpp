#include "../Headers/Plane.hpp"
#include "../Headers/APP.hpp"
#include "../Headers/TWR.hpp"
#include "../Headers/Journal.hpp"
#include "../Headers/Agent.hpp"
#include "../Headers/Functions.hpp"
#include <iostream>
#include <string>
#include <chrono>
#include <thread>


Plane::Plane(const std::string& code, double speed_max, APP* target, TWR* spawn, std::mutex& mtx, APP* app) : Agent(code, mtx), speed_max_(speed_max), app_(app), target_(target), state_(PARKED) {
	pos_.x = spawn->getPos().x; pos_.y = spawn->getPos().y; pos_.altitude=0;
	trajectory_.x = (target->getPos().x - spawn->getPos().x);
	trajectory_.y = (target->getPos().y - spawn->getPos().y);
	trajectory_.altitude = 0;
	make_unitary(&trajectory_);
	spawn->addParkedPlane(this);
	C
};
	
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

void Plane::run() {
	while (running_) {
		std::cout << "[" << code_ << "] : ";

		std::cout << pos_.x << ", ";
		std::cout << pos_.y << ", ";
		std::cout << pos_.altitude << std::endl;

		if (pos_.altitude == 0 && requestTakeoff()){
			state_ = TAKINGOFF;
			trajectory_.altitude+=0.1;
			std::cout << "[" << code_ << "] Requests Takeoff from APP : " << app_->getCode() << std::endl;
		}

		if (pos_.altitude >= 100) trajectory_.altitude = 0;

		if (pos_.altitude != 0 && target_ == app_ && requestLanding()) {
			state_ = LANDING;
			std::cout << "[" << code_ << "] Requests Landing to APP : " << app_->getCode() << std::endl;

			running_ = false;
			stop();
		}

		if (state_ == TAKINGOFF) speed_ = (speed_ + 1 > speed_max_) ? speed_max_ : speed_ + 1;

		pos_.x += trajectory_.x * speed_;
		pos_.y += trajectory_.y * speed_;
		pos_.altitude += trajectory_.altitude * speed_;

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

