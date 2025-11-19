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
}

void CCR::run() {
	std::cout << code_ << std::endl;
}