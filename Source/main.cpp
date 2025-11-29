#include <iostream>
#include "../Headers/Agent.hpp"
#include "../Headers/Plane.hpp"
#include "../Headers/APP.hpp"
#include "../Headers/TWR.hpp"
#include "../Headers/CCR.hpp"
#include "../Headers/Functions.hpp"
#include <SFML/Graphics.hpp>
#include <thread>



using namespace sf;
constexpr unsigned WINDOW_SIZE_X = static_cast<unsigned>(1917*0.7), WINDOW_SIZE_Y = static_cast<unsigned>(1920*0.7);

#define SPEEDMAX 850
#define M_PI 3.1415

#define _PATH_IMG_ "./img/"

const std::string path_image(PATH_IMG);

std::mutex mtx_global;
std::mutex mutex_cin;


void initWindow(std::vector<APP*>& apps, std::vector<Plane*>& planes) {
	RenderWindow window(VideoMode({ WINDOW_SIZE_X, WINDOW_SIZE_Y }), "Air Traffic Control");
	window.setFramerateLimit(60);

	Texture backgroundImage;
	if (!backgroundImage.loadFromFile(path_image + "carte1.jpg")) {
		std::cerr << "Erreur Chargement Carte" << std::endl;
		return;
	}

	Texture planeImage;
	if (!planeImage.loadFromFile(path_image + "plane.png")) {
		std::cerr << "Erreur Chargement Avion" << std::endl;
		return;
	}

	Sprite backgroundSprite(backgroundImage);
	Sprite planeSprite(planeImage);
	backgroundSprite.scale({ 0.7f, 0.7f });
	backgroundSprite.setPosition({ 0.0f, 0.0f });

	std::vector<CircleShape> appShapes;

	for (auto app : apps) {
		CircleShape circle(4);
		circle.setFillColor(Color::Blue);
		circle.setOutlineColor(Color::White);
		circle.setOutlineThickness(1);
		circle.setOrigin({ 4.0f, 4.0f });
		appShapes.push_back(circle);
	}

	std::vector<Sprite> planeSprites;

	planeSprite.setOrigin({ 1920.0f / 2.0f, 1920.0f / 2.0f });
	planeSprite.scale({ 0.02f, 0.02f });

	for (auto p : planes) {
		planeSprites.push_back(planeSprite);
	}

	while (window.isOpen()) {
		while (const std::optional event = window.pollEvent())
		{
			if ((event->is<sf::Event::KeyPressed>() &&
				event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape) ||
				event->is<sf::Event::Closed>())
			{
				window.close();
			}
		}
		window.clear();
		window.draw(backgroundSprite);

		for (size_t i = 0; i < apps.size(); ++i) {
			Position pos = apps[i]->getPos();

			TWR* twr = apps[i]->getTWR();
			if(twr != nullptr){
				if (twr->isParkingFull()) {
					appShapes[i].setFillColor(Color::Red);
				}
				else{
					appShapes[i].setFillColor(Color::Blue);
				}
			}
			appShapes[i].setPosition({ (float)pos.x, (float)pos.y });
			window.draw(appShapes[i]);
		}

		for (size_t i = 0; i < planes.size(); ++i) {
			Position pos = planes[i]->getPos();
			float angle = std::atan2(planes[i]->getTrajectory().y, planes[i]->getTrajectory().x) * 180.0f / static_cast<float>(M_PI);
			float altitudeScale = 0.02f * (static_cast<float>(pos.altitude) / 15.0f);
			planeSprites[i].setPosition({ (float)pos.x, (float)pos.y });
			planeSprites[i].setRotation(degrees(angle+90));
			planeSprites[i].setScale({ altitudeScale, altitudeScale });
			window.draw(planeSprites[i]);
		}


		window.display();
	}
}

void inputThread(Plane& plane, APP& app1, APP& app2) {
	while (true) {
		int var;
		mutex_cin.lock();
		std::cin >> var;
		mutex_cin.unlock();
		if (var == 1) { plane.changeTarget(&app1); plane.start(); }
		if (var == 2) { plane.changeTarget(&app2); plane.start(); };
	}
}


int main() {
	CCR global_satellite("NASA", mtx_global);
	Position Paris(WINDOW_SIZE_Y/2.0 - 25, WINDOW_SIZE_X/4.0, 0.0);

	TWR TWR_Paris("TWR_Paris", 5, Paris, mtx_global);
	APP APP_Paris("APP_Paris", Paris, &TWR_Paris, 50.0, mtx_global, &global_satellite);

	Position Ajaccio(WINDOW_SIZE_Y/15 * 14, WINDOW_SIZE_X/10 * 9, 0.0);
	TWR TWR_Ajaccio("TWR_Ajaccio", 3, Ajaccio, mtx_global);
	APP APP_Ajaccio("APP_Ajaccio", Ajaccio, &TWR_Ajaccio, 50.0, mtx_global, &global_satellite);

	Position Lille(WINDOW_SIZE_Y / 2.0 + 25, WINDOW_SIZE_X / 9.0, 0.0);
	TWR TWR_Lille("TWR_Lille", 5, Lille, mtx_global);
	APP APP_Lille("APP_Lille", Lille, &TWR_Lille, 50.0, mtx_global, &global_satellite);

	Plane A512("A512", SPEEDMAX, &APP_Lille, &TWR_Paris, mtx_global, &APP_Paris);
	Plane B737("B737", SPEEDMAX, &APP_Paris, &TWR_Lille, mtx_global, &APP_Lille);
	Plane B738("B738", SPEEDMAX, &APP_Ajaccio, &TWR_Paris, mtx_global, &APP_Paris);
	Plane B739("B739", SPEEDMAX, &APP_Ajaccio, &TWR_Paris, mtx_global, &APP_Paris);
	Plane B740("B740", SPEEDMAX, &APP_Ajaccio, &TWR_Paris, mtx_global, &APP_Paris);
	Plane B741("B741", SPEEDMAX, &APP_Ajaccio, &TWR_Paris, mtx_global, &APP_Paris);
	Plane B742("B742", SPEEDMAX, &APP_Ajaccio, &TWR_Paris, mtx_global, &APP_Paris);

	global_satellite.addAPP(&APP_Paris);
	global_satellite.addAPP(&APP_Lille);
	global_satellite.addAPP(&APP_Ajaccio);

	/*global_satellite.run();
	TWR_Paris.run();
	APP_Paris.run();
	TWR_Lille.run();
	APP_Lille.run();*/
	
	global_satellite.start();
	//APP_Lille.start();
	//APP_Paris.start();
	//APP_Ajaccio.start();
	//TWR_Ajaccio.start();
	A512.start();
	B737.start();

	std::vector<APP*> apps = { &APP_Lille, &APP_Paris, &APP_Ajaccio };
	std::vector<Plane*> planes = { &A512, &B737, &B738, &B739, &B740, &B741, &B742 };

	std::thread t(inputThread, std::ref(B740), std::ref(APP_Lille), std::ref(APP_Ajaccio));

	initWindow(apps, planes);

	//while (true) {
	//	//int var;
	//	//std::cin >> var;
	//	//if (var==1){
	//	//	A512.changeTarget(&APP_Lille);
	//	//	A512.start();
	//	//}
	//	//if (var == 2) {
	//	//	A512.changeTarget(&APP_Ajaccio);
	//	//	A512.start();
	//	//}
	//}

	t.join();

	global_satellite.stop();
	//APP_Lille.stop();
	//APP_Paris.stop();
	//APP_Ajaccio.stop();
	A512.stop();
	B737.stop();
}