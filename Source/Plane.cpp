#include "../Headers/Plane.hpp"
#include "../Headers/APP.hpp"
#include "../Headers/TWR.hpp"
#include "../Headers/Journal.hpp"
#include <iostream>
#include <mutex>

std::mutex mtx;

Plane::Plane(const std::string& code, double speed, double fuel, Journal* journal) : 
Agent(code), pos_(/*la position de la twr */ ), speed_(speed), fuel_(fuel), journal_(journal), consumption_(1), mtx_(mtx) {
	mtx.lock();
}

PlaneState Plane::getState() const {
	return state_;
}

void Plane::requestLanding() {
	if (getState() == PlaneState::APPROACH) { // Si l'avion s'approche il doit demander à atterrir
		if (app_ != NULL) {
			app_->receivePlane(this); // Pour cela on actualise 
		}
	}
}

void Plane::requestTakeoff() {
	
}