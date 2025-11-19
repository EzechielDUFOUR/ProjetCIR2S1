#include "../Headers/Plane.hpp"
#include "../Headers/APP.hpp"
#include "../Headers/TWR.hpp"
#include "../Headers/Journal.hpp"
#include "../Headers/Agent.hpp"
#include <iostream>
#include <string>
#include <chrono>
#include <thread>


Plane::Plane(const std::string& code, double speed, APP* target, TWR* spawn, std::mutex& mtx) : Agent(code, mtx), speed_(speed) {
	pos_.x = spawn->getPos().x; pos_.y = spawn->getPos().y; pos_.altitude=0;
	trajectory_.x = (target->getPos().x - spawn->getPos().x);
	trajectory_.y = (target->getPos().y - spawn->getPos().y);
	trajectory_.altitude = 0;
	double norme = sqrt(trajectory_.x * trajectory_.x + trajectory_.y * trajectory_.y);
	trajectory_.x /= norme; // On rend le vecteur unitaire en le divisant par sa norme
	trajectory_.y /= norme;
};
	
std::string Plane::getCode() const {
	return Agent::getCode();
}

Position Plane::getPos() const {
	return pos_;
}

void Plane::run() {
	while (running_) {
		std::cout << code_ << " : ";

		std::cout << pos_.x << ", ";
		std::cout << pos_.y << std::endl;

		pos_.x += trajectory_.x * speed_;
		pos_.y += trajectory_.y * speed_;

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

