#include "../Headers/Plane.hpp"
#include "../Headers/APP.hpp"
#include "../Headers/TWR.hpp"
#include "../Headers/Journal.hpp"
#include "../Headers/Agent.hpp"
#include <iostream>
#include <string>
#include <algorithm>

TWR::TWR(const std::string& code, const int& parkingSize, Position& pos) : Agent(code), parkingSize_(parkingSize), pos_(pos), runwayFree_(true) {}

Position TWR::getPos() {
    return pos_;
}

void TWR::run() {
    while (running_) {

        //mtx_.lock();
        //std::cout << "[" << getCode() << "] " << parkingSize_ - parking_.size() << " places restantes / " << ((runwayFree_) ? "Runway Free" : "Runway Occupied") << std::endl;
        //std::cout << "Ordre dans la file : ";
        //for (auto p : waitingLine_) {
        //    if (p->getState() == PARKED) std::cout << "WANNA TAKE OFF / ";
        //    else if (p->getState() == FLYING || p->getState() == HOLDING) std::cout << "WANNA LAND / ";
        //}
        //std::cout << std::endl;
        //mtx_.unlock();

        // Départs aléatoires (avec protection mutex)
        {
            std::lock_guard<std::mutex> lock(mtx_);
            std::vector<Plane*> parkingCopy = parking_;
            for (auto p : parkingCopy) {
                if (p && p->getState() == PARKED && rand() % 100 > 80) {
                    p->changeTarget(p->getRandomTarget());
                    auto it = std::find(waitingLine_.begin(), waitingLine_.end(), p);
                    if (it == waitingLine_.end()) {
                        waitingLine_.push_back(p);
                    }
                    if (!p->isRunning()) {
                        p->start();
                    }
                    break;
                }
            }
        }

        // Traite la queue si piste libre
        {
            std::lock_guard<std::mutex> lock(mtx_);

            if (runwayFree_ && !waitingLine_.empty()) {
                Plane* p = waitingLine_.front();
                waitingLine_.pop_front();

                if (p && p->getState() == PARKED) {
                    runwayFree_ = false;
                    p->getAPP()->receivePlane(p);
                    auto pit = std::find(parking_.begin(), parking_.end(), p);
                    if (pit != parking_.end()) {
                        parking_.erase(pit);
                    }
                    // S'assurer que le thread de l'avion tourne avant le décollage
                    if (!p->isRunning()) {
                        p->start();
                    }
                    // Définir l'altitude AVANT de changer l'état pour éviter la race condition
                    p->setAltitude(0.2);
                    p->changeState(TAKINGOFF);
                    std::cout << "[" << getCode() << "] " << p->getCode() << " autorise au decollage" << std::endl;
                }
                else if (p && (p->getState() == FLYING || p->getState() == HOLDING)) {
                    if (parking_.size() < static_cast<size_t>(parkingSize_)) {
                        runwayFree_ = false;
                        parking_.push_back(p);
                        p->setAltitude(-0.2);
                        p->changeState(LANDING);
                        std::cout << "[" << getCode() << "] " << p->getCode() << " autorise l'atterrissage" << std::endl;
                    } else {
                        // Parking plein, remettre dans la file
                        waitingLine_.push_back(p);
                        runwayFree_ = true;
                    }
                }
            }
        }

        // Met en HOLDING ceux qui attendent
        {
            std::lock_guard<std::mutex> lock(mtx_);
            for (auto p : waitingLine_) {
                if (p && p->getState() != PARKED && p->getState() != LANDING) {
                    p->changeState(HOLDING);
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

bool TWR::requestTakeoff(Plane* p) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (runwayFree_) {
        runwayFree_ = false;
        deleteParkedPlane(p);
        return true;
    }
    return false;
}

bool TWR::requestLanding(Plane* p) {
    std::lock_guard<std::mutex> lock(mtx_);
    bool result = false;
    if (runwayFree_ && parking_.size() < parkingSize_) {
        runwayFree_ = false;
        addParkedPlane(p);
        result = true;
    }
    return result;
}

void TWR::requestTakeoff2(Plane* p) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = std::find(waitingLine_.begin(), waitingLine_.end(), p);
    if (it == waitingLine_.end()) {
        waitingLine_.push_back(p);
    }
}

void TWR::requestLanding2(Plane* p) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = std::find(waitingLine_.begin(), waitingLine_.end(), p);
    if (it == waitingLine_.end()) {
        waitingLine_.push_back(p);
    }
}

void TWR::addParkedPlane(Plane* p) {
    std::lock_guard<std::mutex> lock(mtx_);
    parking_.push_back(p);
}

void TWR::deleteParkedPlane(Plane* p) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = std::find(parking_.begin(), parking_.end(), p);
    if (it != parking_.end()) {
        parking_.erase(it);
    }
}

void TWR::changeRunwayToFree() {
    std::lock_guard<std::mutex> lock(mtx_);
    runwayFree_ = true;
    //std::cout << "Runway freed : " << getCode() << std::endl;
}

void TWR::changeRunwayToOccupied() {
    std::lock_guard<std::mutex> lock(mtx_);
    runwayFree_ = false;
}

bool TWR::isParkingFull() {
    std::lock_guard<std::mutex> lock(mtx_);
    return parking_.size() >= parkingSize_;
}
