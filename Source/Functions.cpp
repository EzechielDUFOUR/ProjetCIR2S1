#include "../Headers/Plane.hpp"
#include "../Headers/APP.hpp"
#include "../Headers/TWR.hpp"
#include "../Headers/Journal.hpp"
#include "../Headers/Agent.hpp"
#include "../Headers/Functions.hpp"
#include <iostream>
#include <string>
#include <cmath>

void make_unitary(Position* pos){
	double norme = sqrt(pos->x*pos->x + pos->y*pos->y + pos->altitude*pos->altitude);
	if (norme != 0.0) {
		pos->x /= norme;
		pos->y /= norme;
		pos->altitude /= norme;
	}

}