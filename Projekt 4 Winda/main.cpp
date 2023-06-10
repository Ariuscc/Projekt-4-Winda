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
const float MaxWeight = 600.0f;        // Maksymalny udŸwig windy w kg

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
    for (int i = 0; i < NumFloors; ++i)
    {
        sf::RectangleShape floor = createFloor(i * FloorHeight);
        floors.push_back(floor);
    }

    std::queue<int> floorQueue;
    int currentFloor = 0;
    float targetY = 0.0f;

    float totalWeight = 0.0f;
    sf::Font font;
    if (!font.loadFromFile("arial.ttf"))
    {
        // Obs³uga b³êdu w przypadku nieudanego za³adowania czcionki
        return -1;
    }
    sf::Text weightText = createText("Total Weight: 0 kg", font, 20, sf::Color::Black, 300, 10);

    sf::Text instructionsText = createText("INSTRUCTIONS\n\n"
        "Nacisnij spacje aby wstrzymac program\n"
        "Press A to add passengers\n"
        "Enter the number of passengers\n"
        "Press the number keys (1-9) to select the target floor\n"
        "Press Enter to confirm", font, 16, sf::Color::Black, 700, 10);

    sf::Text pauseText = createText("PAUZA", font, 50, sf::Color::Red, window.getSize().x / 2, window.getSize().y / 2);
    pauseText.setOrigin(pauseText.getLocalBounds().width / 2, pauseText.getLocalBounds().height / 2);


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
                if (event.key.code == sf::Keyboard::Num0)
                {
                    if (!passengerEntering)
                        floorQueue.push(0);
                }
                else if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num9)
                {
                    int floor = event.key.code - sf::Keyboard::Num1 + 1;
                    if (floor <= NumFloors)
                    {
                        if (!passengerEntering)
                            floorQueue.push(floor);
                    }
                }
                else if (event.key.code == sf::Keyboard::A)
                {
                    if (!passengerEntering)
                    {
                        passengerEntering = true;
                        numPassengersEntering = 0;
                    }
                }
                else if (passengerEntering && event.key.code >= sf::Keyboard::Num0 && event.key.code <= sf::Keyboard::Num9)
                {
                    int num = event.key.code - sf::Keyboard::Num0;
                    numPassengersEntering = numPassengersEntering * 10 + num;
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
            // Poruszanie wind¹
            float distance = std::abs(elevator.getPosition().y - targetY);
            if (distance > 1.0f)
            {
                float direction = (targetY < elevator.getPosition().y) ? -1.0f : 1.0f;
                float movement = direction * ElevatorSpeed * deltaTime;

                elevator.move(0, movement);
            }
            else
            {
                // Winda osi¹gnê³a docelowe piêtro
                if (!floorQueue.empty())
                {
                    int nextFloor = floorQueue.front();
                    floorQueue.pop();
                    targetFloors.push_back(nextFloor);
                    stoppingAtFloor = true;
                    targetY = nextFloor * FloorHeight;
                    currentFloor = nextFloor;
                }
                else if (!targetFloors.empty() && stoppingAtFloor)
                {
                    // Sprawdzanie, czy winda jest pusta po zatrzymaniu siê na piêtrze
                    if (totalWeight == 0.0f)
                    {
                        // Zatrzymywanie windy na 2 sekundy na piêtrze
                        if (stopFloorClock.getElapsedTime().asSeconds() >= 100.0f)
                        {
                            targetFloors.erase(targetFloors.begin());
                            stoppingAtFloor = false;
                            stopFloorClock.restart();
                        }
                    }
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

        if (isPaused) // Wyœwietlanie tekstu "PAUZA" w przypadku wstrzymania programu
        {
            window.draw(pauseText);
        }

        window.display();
    }

    return 0;
}
