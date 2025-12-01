#pragma once
#include "Agent.hpp"
#include <string>
#include <mutex>

class APP;
class TWR;
class Journal;

struct Position {
	double x = 0.0;
	double y = 0.0;
	double altitude = 0.0;
};

enum CurrentState { LANDING, TAKINGOFF, PARKED, FLYING, EMERGENCY, HOLDING, EVASION };

class Plane : public Agent {
private :

	// Journal* journal_;
	APP* app_ = nullptr;
	APP* target_ = nullptr;
	// TWR* twr_ = nullptr;

	Position pos_;
	Position trajectory_;
	double speed_ = 0;
	double speed_max_;
	double fuel_ = 1000;
	double consumption_ = 0.2;
	
	CurrentState state_;

public:
	Plane(const std::string& code, double speed_max, APP* target, TWR* spawn, std::mutex& mtx, APP* app);

	void run() override;

	// Actions initiées par l’avion
	bool requestLanding();
	bool requestTakeoff();

	// Références vers contrôleurs
	void setAPP(APP* app);
	//void setTWR(TWR* twr);

	// Accesseurs
	std::string getCode() const;
	Position getPos() const;
	Position getTrajectory() const { return trajectory_; }

	void changeRunwayState();
	void changeTarget(APP* app);
	void changeState(CurrentState newstate);

	APP* getTarget();

	CurrentState getState();

	void recomputeTrajectory();
	double getSpeed();

	void rotateTrajectory(double angleDegrees);

	APP* getRandomTarget();
};