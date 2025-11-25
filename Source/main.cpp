#include <iostream>
#include "../Headers/Agent.hpp"
#include "../Headers/Plane.hpp"
#include "../Headers/APP.hpp"
#include "../Headers/TWR.hpp"
#include "../Headers/CCR.hpp"
#include "../Headers/Functions.hpp"

#define SPEEDMAX 850

int main() {
	std::mutex mtx_global;
	CCR global_satellite("NASA", mtx_global);
	Position Paris(0.0, 0.0, 0.0);

	TWR TWR_Paris("TWR_Paris", 5, Paris, mtx_global);
	APP APP_Paris("APP_Paris", Paris, &TWR_Paris, 50.0, mtx_global, &global_satellite);

	Position Lille(200.0, 0.0, 0.0);
	TWR TWR_Lille("TWR_Lille", 5, Lille, mtx_global);
	APP APP_Lille("APP_Lille", Lille, &TWR_Lille, 50.0, mtx_global, &global_satellite);

	Plane A512("A512", SPEEDMAX, &APP_Paris, &TWR_Lille, mtx_global, &APP_Lille);

	global_satellite.addAPP(&APP_Paris);
	global_satellite.addAPP(&APP_Lille);

	/*global_satellite.run();
	TWR_Paris.run();
	APP_Paris.run();
	TWR_Lille.run();
	APP_Lille.run();*/
	
	A512.start();
	global_satellite.start();

	while (true) {
		int var;
		std::cin >> var;
		if (var==1){
			A512.changeTarget(&APP_Lille);
			A512.recomputeTrajectory();
			A512.start();
		}
	}

	A512.stop();
}