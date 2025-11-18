#include "../Headers/Plane.hpp"
#include "../Headers/APP.hpp"
#include "../Headers/TWR.hpp"
#include "../Headers/Journal.hpp"
#include "../Headers/Agent.hpp"
#include <iostream>
#include <string>

APP::APP(const std::string& code, Position& pos, TWR* twr, const double& radius) : Agent(code), pos_(pos), radius_(radius), twr_(twr){}

Position APP::getPos() {
	return pos_;
}

void APP::receivePlane(Plane* p){
	PlanesInRange_.push_back(p);
}