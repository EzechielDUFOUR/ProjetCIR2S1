#include "../Headers/Plane.hpp"
#include "../Headers/APP.hpp"
#include "../Headers/TWR.hpp"
#include "../Headers/Journal.hpp"
#include "../Headers/Agent.hpp"
#include <iostream>
#include <string>

CCR(const std::string& code) : Agent(code){}

void CCR:handoverToAPP(Plane* p, APP* app){
	app->receivePlane(p);
}