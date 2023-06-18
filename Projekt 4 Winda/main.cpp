#include <SFML/Graphics.hpp>
#include <queue>
#include <vector>
#include <string>
#include <iostream>

const float FloorHeight = 100.0f;      // Wysokoœæ piêtra
const float ElevatorWidth = 100.0f;    // Szerokoœæ windy
const float ElevatorHeight = 100.0f;   // Wysokoœæ windy
const float ElevatorSpeed = 200.0f;    // Prêdkoœæ windy (piksele na sekundê)
const int NumFloors = 7;               // Liczba piêter
const float PassengerWeight = 70.0f;   // Waga pasa¿era w kg
const float MaxPassengers = 8;        // Maksymalna iloœæ pasa¿erów


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
	std::queue<int> temp1Queue;
	std::queue<int> temp2Queue;
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
		"Po skonczeniu nacisnij Enter", font, 16, sf::Color::Black, 700, 10);

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

	// Zegar odmierzaj¹cy czas zatrzymania windy na piêtrze
	sf::Clock stopFloorClock;

	bool isPaused = false; // Flaga wskazuj¹ca, czy program jest wstrzymany

	int floor = 0;
	float direction = 1.0f;
	int numPassengers = 1;
	bool waitEntering = false;
	int howmany = 0;
	int display = 0;
	int num = 1;

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
				else if (isPaused && event.key.code >= sf::Keyboard::Num0 && event.key.code <= sf::Keyboard::Num7)
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
				else if (event.key.code == sf::Keyboard::Enter && passengerEntering == false)
				{
					isPaused = false; // Odwrócenie stanu pauzy
				}
			}
		}

		if (!isPaused) // Sprawdzenie, czy program nie jest wstrzymany
		{
			while (!waitQueue.empty() && numPassengers < MaxPassengers)      // w ka¿dej chwili dodanych do kolejki mo¿e byæ do 8 pasa¿erów
			{
				passengerQueue.push(waitQueue.front());    // dodanie pasa¿era do kolejki
				waitQueue.pop();
				numPassengers++;
				howmany++;
			}
			while (!passengerQueue.empty()) {
				if (passengerQueue.front() == currentFloor) {          //jeœli wiêcej pasa¿erów na danym piêtrze, wszyscy wsiadaj¹
					passengerQueue.pop();
					num++;
				}
				else {
					temp1Queue.push(passengerQueue.front());
					passengerQueue.pop();
				}
			}
			while (!temp1Queue.empty()) {
				passengerQueue.push(temp1Queue.front());
				temp1Queue.pop();
			}
			if (!passengerQueue.empty())
			{
				temp1Queue.push(passengerQueue.front());
				passengerQueue.pop();
			}
			while (!passengerQueue.empty()) {
				if (passengerQueue.front() == temp1Queue.front()) {
					temp1Queue.push(passengerQueue.front());
					passengerQueue.pop();
				}
				else {
					temp2Queue.push(passengerQueue.front());
					passengerQueue.pop();
				}
			}
			while (!temp1Queue.empty()) {
				passengerQueue.push(temp1Queue.front());
				temp1Queue.pop();
			}
			while (!temp2Queue.empty()) {
				passengerQueue.push(temp2Queue.front());
				temp2Queue.pop();
			}
			while (!lastpassengerQueue.empty()) {
				if (lastpassengerQueue.front() == currentFloor && howmany > 0) {        // dodanie piêtra do kolejki dopiero kiedy pasa¿er wsi¹dzie
					floorQueue.push(floorwaitQueue.front());
					floorwaitQueue.pop();
					lastpassengerQueue.pop();
					howmany--;
				}
				else {
					temp1Queue.push(lastpassengerQueue.front());
					lastpassengerQueue.pop();
					temp2Queue.push(floorwaitQueue.front());
					floorwaitQueue.pop();
				}
			}
			while (!temp1Queue.empty()) {
				lastpassengerQueue.push(temp1Queue.front());
				temp1Queue.pop();
			}
			while (!temp2Queue.empty()) {
				floorwaitQueue.push(temp2Queue.front());
				temp2Queue.pop();
			}
			display = num;

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
				// Aktualizowanie wag pasa¿erów
				totalWeight = display * PassengerWeight;
				// Winda osi¹gnê³a docelowe piêtro
				if ((!floorQueue.empty() or !passengerQueue.empty()) && !stoppingAtFloor)
				{
					int nextFloor = currentFloor;
					if (passengerQueue.empty())
					{
						nextFloor = floorQueue.front();
						floorQueue.pop();
						numPassengers--;
						num--;
					}
					else if (floorQueue.empty())
					{
						nextFloor = passengerQueue.front();
						passengerQueue.pop();
						num++;
					}     // winda zatrzymuje siê po pasa¿era jeœli jest pusta
					else if (abs(currentFloor - floorQueue.front()) > abs(currentFloor - passengerQueue.front()) && ((currentFloor - passengerQueue.front()) * (currentFloor - floorQueue.front())) >= 0)
					{
						nextFloor = passengerQueue.front();
						passengerQueue.pop();
						num++;
					}    // winda zatrzymuje siê po pasa¿era jeœli jest on po drodze do aktualnego celu
					else
					{
						nextFloor = floorQueue.front();
						floorQueue.pop();
						numPassengers--;
						num--;
					}
					targetFloors.push_back(nextFloor);
					stoppingAtFloor = true;
					targetY = nextFloor * FloorHeight;
					currentFloor = nextFloor;
					while (!floorQueue.empty()) {
						if (floorQueue.front() == currentFloor) {            //dodanie do kolejki miejsca docelowego ka¿dego pasa¿era na danym piêtrze
							floorQueue.pop();
							numPassengers--;
							num--;
						}
						else {
							temp1Queue.push(floorQueue.front());
							floorQueue.pop();
						}
					}
					while (!temp1Queue.empty()) {
						floorQueue.push(temp1Queue.front());
						temp1Queue.pop();
					}

				}
				else if (!targetFloors.empty())
				{
					// Zatrzymywanie windy na 0.5 sekundy na piêtrze
					if (stopFloorClock.getElapsedTime().asSeconds() >= 0.5f)
					{
						targetFloors.erase(targetFloors.begin());
						stoppingAtFloor = false;
						stopFloorClock.restart();
					}
				}
				else if (stopFloorClock.getElapsedTime().asSeconds() >= 5.0f)
				{
					targetFloors.push_back(NumFloors - 1);
					stoppingAtFloor = true;
					targetY = (NumFloors - 1) * FloorHeight;
					currentFloor = (NumFloors - 1);
				}
				display = num;
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
