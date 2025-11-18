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

class Plane : public Agent {
private :

	// Journal* journal_;
	// APP* app_ = nullptr;
	// TWR* twr_ = nullptr;

	Position pos_;
	Position trajectory_;
	double speed_;
	// double fuel_;
	// double consumption_;

public:
	Plane(const std::string& code, double speed, APP* target, TWR* spawn);

	~Plane();

	void run() override;

	// Actions initiées par l’avion
	//void requestLanding();
	//void requestTakeoff();

	// Références vers contrôleurs
	//void setAPP(APP* app);
	//void setTWR(TWR* twr);

	// Accesseurs
	std::string getCode() const;
	Position getPos() const;
};