#include "Agent.hpp"

Agent::Agent(const std::string& name) : name_(name) {}

Agent::~Agent() {
    stop();  // s'assurer que le thread est arrêté à la destruction
    if (thread_.joinable()) {
        thread_.join();
    }
}

void Agent::start() {
    if (running_) return; // déjà en cours d'exécution

    running_ = true;
    thread_ = std::thread([this] {this->run();});
}

void Agent::stop() {
    running_ = false;  // le thread doit vérifier cette variable dans run()
}

bool Agent::isRunning() const {
    return running_;
}

const std::string& Agent::getName() const {
    return name_;
}
