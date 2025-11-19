#include "../Headers/Plane.hpp"
#include "../Headers/APP.hpp"
#include "../Headers/TWR.hpp"
#include "../Headers/Journal.hpp"
#include "../Headers/Agent.hpp"
#include <iostream>
#include <string>

TWR::TWR(const std::string& code, const int&parkingSize, Position& pos, std::mutex& mtx) : Agent(code, mtx), parkingSize_(parkingSize), pos_(pos){}

Position TWR::getPos(){
	return pos_;
}

void TWR::run(){
	std::cout << code_ << std::endl;
}