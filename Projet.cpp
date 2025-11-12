#include <iostream>
#include <ctime>
#include <cstdlib>
#include "Projet.hpp"

int main(){
    srand(time(0));
    Plane p;
    std::cout << p.getPos().getX() << std::endl;
}