#include "../Headers/Agent.hpp"

Agent::Agent(const std::string& code) : code_(code) {}

Agent::~Agent() {
	stop();  // s'assurer que le thread est arrt la destruction
	if (thread_.joinable()) {
		thread_.join();
	}
}

void Agent::start() {
	if (running_) return;

	if (thread_.joinable()) {
		thread_.join();
	}

	running_ = true;
	thread_ = std::thread([this] { this->run(); });
}

void Agent::stop() {
	running_ = false; 
}

bool Agent::isRunning() const {
	return running_;
}

const std::string& Agent::getCode() const {
	return code_;
}
