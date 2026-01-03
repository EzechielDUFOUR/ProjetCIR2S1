#include <iostream>
#include "../Headers/Agent.hpp"
#include "../Headers/Plane.hpp"
#include "../Headers/APP.hpp"
#include "../Headers/TWR.hpp"
#include "../Headers/CCR.hpp"
#include "../Headers/Functions.hpp"
#include <SFML/Graphics.hpp>
#include <thread>


#define MAP_RESIZING_FACTOR 0.7

using namespace sf;
constexpr unsigned WINDOW_SIZE_X = static_cast<unsigned>(1796 * MAP_RESIZING_FACTOR), WINDOW_SIZE_Y = static_cast<unsigned>(1796 * MAP_RESIZING_FACTOR);

#define SPEEDMAX 850
#define M_PI 3.1415

#define _PATH_IMG_ "./img/"

const std::string path_image(PATH_IMG);

void initWindow(std::vector<APP*>& apps, std::vector<Plane*>& planes) {
	RenderWindow window(VideoMode({ WINDOW_SIZE_X, WINDOW_SIZE_Y }), "Air Traffic Control");
	window.setFramerateLimit(60);

	Texture backgroundImage;
	if (!backgroundImage.loadFromFile(path_image + "carte.png")) {
		std::cerr << "Erreur Chargement Carte" << std::endl;
		return;
	}

	Texture planeImage;
	if (!planeImage.loadFromFile(path_image + "avion.png")) {
		std::cerr << "Erreur Chargement Avion" << std::endl;
		return;
	}

	Sprite backgroundSprite(backgroundImage);
	Sprite planeSprite(planeImage);
	backgroundSprite.scale({ MAP_RESIZING_FACTOR, MAP_RESIZING_FACTOR });
	backgroundSprite.setPosition({ 0.0f, 0.0f });

	std::vector<CircleShape> appShapes;
	std::vector<CircleShape> rangeCircles;

	for (auto app : apps) {
		CircleShape circle(4);
		circle.setFillColor(Color::Blue);
		circle.setOutlineColor(Color::White);
		circle.setOutlineThickness(1);
		circle.setOrigin({ 4.0f, 4.0f });
		appShapes.push_back(circle);

		CircleShape rangeCircle(app->getRadius());
		rangeCircle.setFillColor(Color::Transparent);
		rangeCircle.setOutlineColor(Color(100, 150, 255, 80));
		rangeCircle.setOutlineThickness(2);
		rangeCircle.setOrigin({ (float)app->getRadius(), (float)app->getRadius() });
		rangeCircles.push_back(rangeCircle);
	}

	std::vector<Sprite> planeSprites;

	planeSprite.setOrigin({ 500.0f / 2.0f, 500.0f / 2.0f });
	planeSprite.scale({ 0.1f, 0.1f });

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
			rangeCircles[i].setPosition({ (float)pos.x, (float)pos.y });
			window.draw(rangeCircles[i]);
		}

		for (size_t i = 0; i < apps.size(); ++i) {
			Position pos = apps[i]->getPos();

			TWR* twr = apps[i]->getTWR();
			if (twr != nullptr) {
				if (twr->isParkingFull()) {
					appShapes[i].setFillColor(Color::Red);
				}
				else {
					appShapes[i].setFillColor(Color::Blue);
				}
			}
			appShapes[i].setPosition({ (float)pos.x, (float)pos.y });
			rangeCircles[i].setPosition({ (float)pos.x, (float)pos.y });
			window.draw(rangeCircles[i]);
			window.draw(appShapes[i]);
		}

		for (size_t i = 0; i < planes.size(); ++i) {
			Position pos = planes[i]->getPos();
			float angle = std::atan2(planes[i]->getTrajectory().y, planes[i]->getTrajectory().x) * 180.0f / static_cast<float>(M_PI);
			float altitudeScale = 0.1 * (static_cast<float>(pos.altitude) / 15.0f);
			planeSprites[i].setPosition({ (float)pos.x, (float)pos.y });
			planeSprites[i].setRotation(degrees(angle + 90));
			planeSprites[i].setScale({ altitudeScale, altitudeScale });
			window.draw(planeSprites[i]);
		}


		window.display();
	}
}


int main() {
	CCR global_satellite("NASA");
	Position Paris(917 * MAP_RESIZING_FACTOR, 532 * MAP_RESIZING_FACTOR, 0.0);

	TWR TWR_Paris("TWR_Paris", 5, Paris);
	APP APP_Paris("APP_Paris", Paris, &TWR_Paris, 50.0, &global_satellite);

	Position Ajaccio(1658 * MAP_RESIZING_FACTOR, 1653 * MAP_RESIZING_FACTOR, 0.0);
	TWR TWR_Ajaccio("TWR_Ajaccio", 3, Ajaccio);
	APP APP_Ajaccio("APP_Ajaccio", Ajaccio, &TWR_Ajaccio, 50.0, &global_satellite);

	Position Lille(1021 * MAP_RESIZING_FACTOR, 239 * MAP_RESIZING_FACTOR, 0.0);
	TWR TWR_Lille("TWR_Lille", 5, Lille);
	APP APP_Lille("APP_Lille", Lille, &TWR_Lille, 50.0, &global_satellite);

	Position Perpignan(1005 * MAP_RESIZING_FACTOR, 1555 * MAP_RESIZING_FACTOR);
	TWR TWR_Perpignan("TWR_Perpignan", 4, Perpignan);
	APP APP_Perpignan("APP_Perpignan", Perpignan, &TWR_Perpignan, 50.0, &global_satellite);

	Position Marseille(1275 * MAP_RESIZING_FACTOR, 1458 * MAP_RESIZING_FACTOR);
	TWR TWR_Marseille("TWR_Marseille", 6, Marseille);
	APP APP_Marseille("APP_Marseille", Marseille, &TWR_Marseille, 50.0, &global_satellite);

	Position Bordeaux(615 * MAP_RESIZING_FACTOR, 1200 * MAP_RESIZING_FACTOR);
	TWR TWR_Bordeaux("TWR_Bordeaux", 4, Bordeaux);
	APP APP_Bordeaux("APP_Bordeaux", Bordeaux, &TWR_Bordeaux, 50.0, &global_satellite);

	Position Brest(180 * MAP_RESIZING_FACTOR, 626 * MAP_RESIZING_FACTOR);
	TWR TWR_Brest("TWR_Brest", 3, Brest);
	APP APP_Brest("APP_Brest", Brest, &TWR_Brest, 50.0, &global_satellite);

	// Avions pour Paris (5 places)
	Plane A512("A512", SPEEDMAX, &APP_Lille, &TWR_Paris, &APP_Paris);
	Plane B739("B739", SPEEDMAX, &APP_Ajaccio, &TWR_Paris, &APP_Paris);
	Plane B740("B740", SPEEDMAX, &APP_Ajaccio, &TWR_Paris, &APP_Paris);
	Plane B741("B741", SPEEDMAX, &APP_Ajaccio, &TWR_Paris, &APP_Paris);

	// Avions pour Lille (5 places)
	Plane B737("B737", SPEEDMAX, &APP_Paris, &TWR_Lille, &APP_Lille);
	Plane B738("B738", SPEEDMAX, &APP_Ajaccio, &TWR_Lille, &APP_Lille);
	Plane PAX001("PAX001", SPEEDMAX, &APP_Bordeaux, &TWR_Lille, &APP_Lille);
	Plane PAX002("PAX002", SPEEDMAX, &APP_Marseille, &TWR_Lille, &APP_Lille);

	// Avions pour Ajaccio (3 places)
	Plane B744("B744", SPEEDMAX, &APP_Ajaccio, &TWR_Ajaccio,  &APP_Ajaccio);
	Plane AJC001("AJC001", SPEEDMAX, &APP_Marseille, &TWR_Ajaccio,  &APP_Ajaccio);

	// Avions pour Perpignan (4 places)
	Plane PGN001("PGN001", SPEEDMAX, &APP_Marseille, &TWR_Perpignan,  &APP_Perpignan);
	Plane PGN002("PGN002", SPEEDMAX, &APP_Bordeaux, &TWR_Perpignan,  &APP_Perpignan);
	Plane PGN003("PGN003", SPEEDMAX, &APP_Ajaccio, &TWR_Perpignan,  &APP_Perpignan);

	// Avions pour Marseille (6 places)
	Plane B745("B745", SPEEDMAX, &APP_Ajaccio, &TWR_Marseille,  &APP_Marseille);
	Plane B746("B746", SPEEDMAX, &APP_Ajaccio, &TWR_Marseille,  &APP_Marseille);
	Plane MRS001("MRS001", SPEEDMAX, &APP_Paris, &TWR_Marseille,  &APP_Marseille);
	Plane MRS002("MRS002", SPEEDMAX, &APP_Lille, &TWR_Marseille,  &APP_Marseille);
	Plane MRS003("MRS003", SPEEDMAX, &APP_Bordeaux, &TWR_Marseille,  &APP_Marseille);

	// Avions pour Bordeaux (4 places)
	Plane B743("B743", SPEEDMAX, &APP_Ajaccio, &TWR_Bordeaux,  &APP_Bordeaux);
	Plane BOD001("BOD001", SPEEDMAX, &APP_Paris, &TWR_Bordeaux,  &APP_Bordeaux);
	Plane BOD002("BOD002", SPEEDMAX, &APP_Lille, &TWR_Bordeaux,  &APP_Bordeaux);

	// Avions pour Brest (3 places)
	Plane B747("B747", SPEEDMAX, &APP_Ajaccio, &TWR_Brest,  &APP_Brest);
	Plane B748("B748", SPEEDMAX, &APP_Ajaccio, &TWR_Brest,  &APP_Brest);


	global_satellite.addAPP(&APP_Paris);
	global_satellite.addAPP(&APP_Lille);
	global_satellite.addAPP(&APP_Ajaccio);
	global_satellite.addAPP(&APP_Perpignan);
	global_satellite.addAPP(&APP_Marseille);
	global_satellite.addAPP(&APP_Bordeaux);
	global_satellite.addAPP(&APP_Brest);

	global_satellite.start();
	//APP_Lille.start();
	//APP_Paris.start();
	//APP_Ajaccio.start();
	TWR_Ajaccio.start();
	TWR_Paris.start();
	TWR_Lille.start();
	TWR_Perpignan.start();
	TWR_Marseille.start();
	TWR_Bordeaux.start();
	TWR_Brest.start();
	APP_Ajaccio.start();
	APP_Paris.start();
	APP_Lille.start();
	APP_Perpignan.start();
	APP_Marseille.start();
	APP_Bordeaux.start();
	APP_Brest.start();
	//A512.start();
	//B737.start();

	std::vector<APP*> apps = global_satellite.getAPPS();//{ &APP_Lille, &APP_Paris, &APP_Ajaccio };
	std::vector<Plane*> planes = {
		&A512, &B737, &B738, &B739, &B740, &B741, &B743, &B744, &B745, &B746, &B747, &B748,
		&PAX001, &PAX002,
		&AJC001,
		&PGN001, &PGN002, &PGN003,
		&MRS001, &MRS002, &MRS003,
		&BOD001, &BOD002,
	};

	initWindow(apps, planes);

	global_satellite.stop();
	//APP_Lille.stop();
	//APP_Paris.stop();
	//APP_Ajaccio.stop();
	A512.stop();
	//B737.stop();
}