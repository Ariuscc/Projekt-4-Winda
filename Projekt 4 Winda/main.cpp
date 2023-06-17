#include <SFML/Graphics.hpp>
#include <queue>
#include <vector>
#include <string>

const float FloorHeight = 100.0f;      // Wysokoœæ piêtra
const float ElevatorWidth = 100.0f;    // Szerokoœæ windy
const float ElevatorHeight = 100.0f;   // Wysokoœæ windy
const float ElevatorSpeed = 200.0f;    // Prêdkoœæ windy (piksele na sekundê)
const int NumFloors = 7;               // Liczba piêter
const float PassengerWeight = 70.0f;   // Waga pasa¿era w kg
const float MaxPassengers = 7;        // Maksymalna iloœæ pasa¿erów

struct Passenger
{
	int targetFloor;  // Piêtro docelowe pasa¿era
};

sf::RectangleShape createFloor(float y)
{
	sf::RectangleShape floor(sf::Vector2f(ElevatorWidth, FloorHeight));
	floor.setPosition(0, y);
	floor.setFillColor(sf::Color::Green);
	floor.setOutlineThickness(2);
	floor.setOutlineColor(sf::Color::Black);
	return floor;
}

sf::RectangleShape createElevator()
{
	sf::RectangleShape elevator(sf::Vector2f(ElevatorWidth, ElevatorHeight));
	elevator.setFillColor(sf::Color::Red);
	elevator.setOutlineThickness(2);
	elevator.setOutlineColor(sf::Color::Black);
	return elevator;
}

sf::Text createText(const std::string& content, const sf::Font& font, unsigned int size, const sf::Color& color, float x, float y)
{
	sf::Text text(content, font, size);
	text.setFillColor(color);
	text.setPosition(x, y);
	return text;
}

int main()
{
	sf::RenderWindow window(sf::VideoMode(1280, 720), "Symulacja windy");

	sf::RectangleShape elevator = createElevator();

	std::vector<sf::RectangleShape> floors;
	for (int i = NumFloors - 1; i >= 0; --i)
	{
		sf::RectangleShape floor = createFloor(i * FloorHeight);
		floors.push_back(floor);
	}

	std::queue<int> floorQueue;
	std::queue<int> passengerQueue;
	std::queue<int> waitQueue;
	std::queue<int> floorwaitQueue;
	std::queue<int> lastpassengerQueue;
	floorQueue.push(6); // Dodajemy parter do kolejki
	int currentFloor = 6;
	float targetY = currentFloor * FloorHeight;

	float totalWeight = 0.0f;
	sf::Font font;
	if (!font.loadFromFile("arial.ttf"))
	{
		// Obs³uga b³êdu w przypadku nieudanego za³adowania czcionki
		return -1;
	}
	sf::Text weightText = createText("Total Weight: 0 kg", font, 20, sf::Color::Black, 300, 10);

	sf::Text instructionsText = createText("INSTRUCTIONS\n\n"
		"Nacisnij A zeby dodac pasazera\n"
		"Wprowadz na ktorym pietrze wsiada dany pasazer\n"
		"Wprowadz na ktore pietro jedzie dany pasazer\n"
		"Po skonczeniu nacisnij spacje", font, 16, sf::Color::Black, 700, 10);

	sf::Text wsiadaText = createText("Gdzie wsiada?", font, 50, sf::Color::Red, window.getSize().x / 2, window.getSize().y / 2);
	wsiadaText.setOrigin(wsiadaText.getLocalBounds().width / 2, wsiadaText.getLocalBounds().height / 2);

	sf::Text jedzieText = createText("Gdzie jedzie?", font, 50, sf::Color::Red, window.getSize().x / 2, window.getSize().y / 2);
	jedzieText.setOrigin(jedzieText.getLocalBounds().width / 2, jedzieText.getLocalBounds().height / 2);

	sf::Clock clock;
	float deltaTime = 0.0f;

	std::vector<int> targetFloors;
	int numPassengersEntering = 0;
	bool passengerEntering = false;
	bool stoppingAtFloor = false;
	std::vector<Passenger> passengers;

	// Zegar odmierzaj¹cy czas zatrzymania windy na piêtrze
	sf::Clock stopFloorClock;

	bool isPaused = false; // Flaga wskazuj¹ca, czy program jest wstrzymany

	int floor = 0;
	float direction = 1.0f;
	int numPassengers = 0;
	bool waitEntering = false;
	int howmany = 0;

	while (window.isOpen())
	{
		deltaTime = clock.restart().asSeconds();
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			else if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::A)
				{
					isPaused = true;
					passengerEntering = false;
				}
				else if (isPaused && event.key.code >= sf::Keyboard::Num0 && event.key.code <= sf::Keyboard::Num9)
				{
					if (!passengerEntering) {
						floor = NumFloors - (event.key.code - sf::Keyboard::Num1 + 1);
						if (floor >= 0)
							waitQueue.push(floor);
						lastpassengerQueue.push(floor);
						passengerEntering = true;
					}
					else {
						floor = NumFloors - (event.key.code - sf::Keyboard::Num1 + 1);
						if (floor >= 0)
							floorwaitQueue.push(floor);
						passengerEntering = false;
					}
				}
				else if (passengerEntering && event.key.code == sf::Keyboard::Enter)
				{
					passengerEntering = false;
					for (int i = 0; i < numPassengersEntering; ++i)
					{
						Passenger passenger;
						passenger.targetFloor = currentFloor;
						passengers.push_back(passenger);
					}
					numPassengersEntering = 0;
				}
				else if (event.key.code == sf::Keyboard::Space)
				{
					isPaused = !isPaused; // Odwrócenie stanu pauzy
				}
			}
		}

		if (!isPaused) // Sprawdzenie, czy program nie jest wstrzymany
		{
			while (!waitQueue.empty() && numPassengers <= 6)
			{
				passengerQueue.push(waitQueue.front());  // dodanie pasa¿era do kolejki
				waitQueue.pop();
				numPassengers++;
				howmany++;
			}
			while (!floorwaitQueue.empty()) {
				if ((lastpassengerQueue.front() != currentFloor) or howmany == 0) break; // dodanie piêtra do kolejki dopiero kiedy pasa¿er wsi¹dzie
				floorQueue.push(floorwaitQueue.front());
				lastpassengerQueue.pop();
				floorwaitQueue.pop();
				howmany--;
			}

			// Poruszanie wind¹
			float distance = std::abs(elevator.getPosition().y - targetY);
			if (distance > 1.0f)
			{
				direction = (targetY < elevator.getPosition().y) ? -1.0f : 1.0f;
				float movement = direction * ElevatorSpeed * deltaTime;

				elevator.move(0, movement);
				stopFloorClock.restart();
			}
			else
			{
				// Winda osi¹gnê³a docelowe piêtro
				if ((!floorQueue.empty() or !passengerQueue.empty()) && !stoppingAtFloor)
				{
					int nextFloor = currentFloor;
					if (passengerQueue.empty())
					{
						nextFloor = floorQueue.front();
						floorQueue.pop();
						numPassengers--;
					}
					else if (floorQueue.empty())
					{
						nextFloor = passengerQueue.front();
						passengerQueue.pop();
					}     // winda zatrzymuje siê po pasa¿era jeœli jest pusta
					else if (abs(currentFloor - floorQueue.front()) > abs(currentFloor - passengerQueue.front()) && ((currentFloor - passengerQueue.front()) * (currentFloor - floorQueue.front())) >= 0)
					{
						nextFloor = passengerQueue.front();
						passengerQueue.pop();
					}    // winda zatrzymuje siê po pasa¿era jeœli jest on po drodze do aktualnego celu
					else
					{
						nextFloor = floorQueue.front();
						floorQueue.pop();
						numPassengers--;
					}
					targetFloors.push_back(nextFloor);
					stoppingAtFloor = true;
					targetY = nextFloor * FloorHeight;
					currentFloor = nextFloor;
				}
				else if (!targetFloors.empty())
				{
					// Zatrzymywanie windy na 2 sekundy na piêtrze
					if (stopFloorClock.getElapsedTime().asSeconds() >= 0.2f)
					{
						targetFloors.erase(targetFloors.begin());
						stoppingAtFloor = false;
						stopFloorClock.restart();
					}
				}
				else if (stopFloorClock.getElapsedTime().asSeconds() >= 5.0f)
				{
					floorQueue.push(NumFloors - 1);
				}
			}

			// Aktualizowanie wag pasa¿erów
			if (passengers.size() > 0)
			{
				for (int i = static_cast<int>(passengers.size()) - 1; i >= 0; --i)
				{
					if (targetFloors[0] == currentFloor)
					{
						passengers.erase(passengers.begin() + i);
						totalWeight -= PassengerWeight;
					}
				}
			}

			// Aktualizowanie tekstu z wag¹ pasa¿erów
			weightText.setString("Total Weight: " + std::to_string(totalWeight) + " kg");
		}

		window.clear(sf::Color::White);

		// Rysowanie piêter
		for (const auto& floor : floors)
		{
			window.draw(floor);
		}

		// Rysowanie windy
		window.draw(elevator);

		// Rysowanie tekstu z wag¹ pasa¿erów
		window.draw(weightText);

		// Rysowanie instrukcji
		window.draw(instructionsText);

		if (isPaused && !passengerEntering) // Wyœwietlanie tekstu "PAUZA" w przypadku wstrzymania programu
		{
			window.draw(wsiadaText);
		}
		if (isPaused && passengerEntering) // Wyœwietlanie tekstu "PAUZA" w przypadku wstrzymania programu
		{
			window.draw(jedzieText);
		}

		window.display();
	}

	return 0;
}
