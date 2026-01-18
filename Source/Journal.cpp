#include "../Headers/Journal.hpp"
#include <iostream>
#include <fstream>

Journal::Journal(const std::string& filename) {
	ofs_.open(filename, std::ios::out);  // Mode out pour réinitialiser le fichier à chaque exécution
	if (!ofs_.is_open()) {
		std::cerr << "Erreur: Impossible d'ouvrir le fichier journal: " << filename << std::endl;
		return;
	}
}

Journal::~Journal() {
	if (ofs_.is_open()) {
		ofs_.close();
	}
}

void Journal::logEvent(const std::string& message) {
	std::lock_guard<std::mutex> lock(mtx_);
	
	if (!ofs_.is_open()) {
		return;
	}
	
	ofs_ << message << "\n";
}

