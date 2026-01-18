#pragma once
#include <string>
#include <fstream>
#include <mutex>

class Journal {
public:
	Journal(const std::string& filename);
	~Journal();

	void logEvent(const std::string& message);

private:
	std::ofstream ofs_;
	std::mutex mtx_;
};
