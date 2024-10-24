#include "pitches.h"



// START ====== DATA TYPES ====================

struct Player {
public:
  bool isAlive;
  int* chamber;
  int clickIndex;
  int ledPin;
  bool isSelected;

  // Default constructor
  Player() : isAlive(false), chamber(nullptr), clickIndex(-1), ledPin(-1), isSelected(false) {
      // Optionally, you can initialize chamber to null or skip allocation here
  }

  // custom constructor that randomizes chamber.
  Player(int ledPinIn){

    isAlive = true;
    clickIndex = 0;
    ledPin = ledPinIn;
    isSelected = false;

    chamber = new int[6];

    for (int i = 0; i < 6; i++) {
      chamber[i] = 0;
    }
    chamber[random(0,6)] = 1;
  }

  bool triggerGun() {
    if (chamber[clickIndex] == 1) {
      isAlive = false;
      return true;
    } else {
      clickIndex++;
      return false;
    }
  }

  // Pretty prints the player's info and full chamber status
  void printInfo() {
    Serial.print("Player:: ");
    Serial.print(" ledPin: ");
    Serial.print(ledPin);
    Serial.print("; isAlive: ");
    Serial.print(isAlive);
    Serial.print("; isSelected: ");
    Serial.print(isSelected);
    Serial.print("; clickIndex: ");
    Serial.print(clickIndex);
    Serial.print("; chamber: [");

    for (int i = 0; i < 6; i++) {
      Serial.print(chamber[i]);
      Serial.print(i != 5 ? "," : "");
    }

    Serial.println("]");
  }

  // Copy assignment operator
  Player& operator=(const Player& other) {

    // Skip copy if the source is this same instance
    if (this == &other) {
      return *this;
    }

    // Skip copying if the source is invalid (default constructed)
    if (other.clickIndex == -1) {
        Serial.println("Skipping assignment from default-constructed Player.");
        return *this;
    }

    // Free existing memory if already allocated
    if (chamber != nullptr) {
        delete[] chamber;
    }

    // Deep copy the values
    isAlive = other.isAlive;
    ledPin = other.ledPin;
    clickIndex = other.clickIndex;
    isSelected = other.isSelected;

    chamber = new int[6];
    for (int i = 0; i < 6; i++) {
        chamber[i] = other.chamber[i];
    }
    return *this;
  }

  // Destructor to clean up dynamically allocated memory
  ~Player() {
      if (chamber != nullptr) {
          delete[] chamber;
      }
  }
};

// Struct to map LEDs with their state and the player they represent.
struct PlayerLed {
  static const int blinkInterval = 200;

  int state;
  int pin;
  int playerIndex;
  unsigned long prevBlinkMillis;

  // Default constructor
  PlayerLed() : state(-1), pin(-1), playerIndex(-1), prevBlinkMillis(0) { }
  
  // Custom constructor
  PlayerLed(int pinIn, int playerIndexIn) {
    state = HIGH;
    pin = pinIn;
    playerIndex = playerIndexIn;
    prevBlinkMillis = 0;
  }

  void turnOnIfAlive(const Player* players) {
    if (players[playerIndex].isAlive) {
      digitalWrite(pin, HIGH);
    } else {
      digitalWrite(pin, LOW);
    }
  }

  void turnOnIfSelected(const Player* players) {
    if (players[playerIndex].isSelected) {
      digitalWrite(pin, HIGH);
    } else {
      digitalWrite(pin, LOW);
    }
  }

  void flashIfAlive(const Player* players) {
    // Uses same value for the duration of off and on state.
    if (players[playerIndex].isAlive) {
      const unsigned long currentMillis = millis();
      if (state == LOW) {
          // if the Led is off, we must wait for the interval to expire before turning it on
        if (currentMillis - prevBlinkMillis >= blinkInterval) {
          // time is up, so change the state to HIGH
          state = HIGH;
          digitalWrite(pin, state);
          // and save the time when we made the change
          prevBlinkMillis = currentMillis;
        }
      } else {  // i.e. if onBoardLedState is HIGH
        // if the Led is on, we must wait for the duration to expire before turning it off
        if (currentMillis - prevBlinkMillis >= blinkInterval) {
          // time is up, so change the state to LOW
          state = LOW;
          digitalWrite(pin, state);
          // and save the time when we made the change
          prevBlinkMillis = currentMillis;
        }
      }
    } else {
      digitalWrite(pin, LOW);
    }
  }

  void flashIfSelected(const Player* players) {
    // Uses same value for the duration of off and on state.
    if (players[playerIndex].isSelected) {
      const unsigned long currentMillis = millis();
      if (state == LOW) {
          // if the Led is off, we must wait for the interval to expire before turning it on
        if (currentMillis - prevBlinkMillis >= blinkInterval) {
          // time is up, so change the state to HIGH
          state = HIGH;
          digitalWrite(pin, state);
          // and save the time when we made the change
          prevBlinkMillis = currentMillis;
        }
      } else {  // i.e. if onBoardLedState is HIGH
        // if the Led is on, we must wait for the duration to expire before turning it off
        if (currentMillis - prevBlinkMillis >= blinkInterval) {
          // time is up, so change the state to LOW
          state = LOW;
          digitalWrite(pin, state);
          // and save the time when we made the change
          prevBlinkMillis = currentMillis;
        }
      }
    } else {
      digitalWrite(pin, LOW);
    }
  }

  void printInfo() {
    Serial.print("LED:: ");
    Serial.print(" State: ");
    Serial.print(state == HIGH ? "HIGH" : "LOW");
    Serial.print("; playerIndex: ");
    Serial.print(playerIndex);
    Serial.print("; pin: ");
    Serial.println(pin);
  }

  // I don't believe this is needed given that this struct only has primitive values...
  // however, I'm scared of cpp now and have trust issues.
  PlayerLed& operator=(const PlayerLed& other) {
    if (other.state == -1) {
      Serial.println("Skipping assignment from default-constructed PlayerLed.");
      return *this;
    }
    state = other.state;
    pin = other.pin;
    playerIndex = other.playerIndex;
    prevBlinkMillis = other.prevBlinkMillis;

    return *this;
  }
};

struct Button {
  int pin;
  unsigned long previousPressMillis;
  unsigned int debounceInterval;
  bool wasPressed;

  void readButton() {
    wasPressed = false; // This makes sure that a press is only true in the loop that it happened.
    if (millis() - previousPressMillis >= debounceInterval) {
      if (digitalRead(pin) == LOW) {
        wasPressed = true;
        Serial.println("Button down!");
        previousPressMillis = millis();
      }
    }
  }
};

struct Buzzer {
  int pin;
  unsigned long previousToneMillis;

  void makeNoise() {
    tone(pin, NOTE_G6, 200);
  }

  // TODO: Implement this method so we can play a full melody.
  // void playMelody() {}
};



// END  ======  DATA TYPES ====================


// START ====== GLOBAL STUFF ====================

#define PLAYER_COUNT 4
Player* players = new Player[PLAYER_COUNT];

// LEDs
const int playerLedPins[] = {2,3,4,5}; // Need to add more pins if more players wanted.
PlayerLed* playerLeds = new PlayerLed[PLAYER_COUNT];
static constexpr int ledBlinkDuration = 300;
const int ledIntervalDuration = 500;

// Buzzer
static constexpr int buzzerPin = 6;
struct Buzzer buzzer = {buzzerPin, 0};

//  Button
static constexpr int buttonAPin = 7;
static constexpr int buttonBPin = 8;
static constexpr int buttonInterval = 200; // number of millisecs between button readings

struct Button buttonA = {buttonAPin, 0, buttonInterval, false};
struct Button buttonB = {buttonBPin, 0, buttonInterval, false};

// Potentiometer
static constexpr int knobPin = A0;


// Time
unsigned long currentMillis = 0;    // stores the value of millis() in each iteration of loop()

// END ====== GLOBAL STUFF ====================


// Player state vars
const int playerCount = 4; // Total players playing
int activePlayerCount = 4; // Players that are alive
byte activePlayers = 0b1111; // Byte with lower 4 bits representing which players are still alive
int playerChambers[playerCount][6]; // Represent each player's chamber
int playerChambersIndexes[playerCount] = {};

// Potentiometer
int previousKnobRead = 0;
boolean knobReadSet = false;
int knobSensitivity = 50;

// Selection input type
enum SelectionMode: int {
  Knob,
  Button
};

SelectionMode currentSelectionMode = Button; // Change this depending on secondary input type

// GameState
enum class GameState: int {
  GameRunning,
  PlayerSelection,
  PlayerTrigger,
  GameOverWinner
};

GameState currentGameState = GameState::GameRunning;

// Specific game state aux variables and functions
int currentSelectedPlayer = -1; // State for player selection, do not use directly

int getCurrentSelectedPlayer() {
  return currentSelectedPlayer;
}

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(A5)); // seed our RNG with an open analogue pin for better randomness.

  // Pin setups
  Serial.println("Setting up game for 4 players");
  // Set up player LED pins
  for (int i = 0; i < playerCount; i++) {
    pinMode(playerLedPins[i], OUTPUT);
  }
  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonAPin, INPUT_PULLUP);
  pinMode(buttonBPin, INPUT_PULLUP);
  pinMode(knobPin, INPUT);

  // initialize our data structures.
  // - Create the players and playerLeds
  for (int i = 0; i < PLAYER_COUNT; i++) {
    players[i] = Player(playerLedPins[i]);
    playerLeds[i] = PlayerLed(playerLedPins[i], i);
    players[i].printInfo();
    playerLeds[i].printInfo();
  }

  // Game start sound!
  buzzer.makeNoise();
}

void loop() {

  // Read inputs
  buttonA.readButton();
  buttonB.readButton();

  switch (currentGameState) {
    case GameState::GameRunning: {
        // If we've reached the end of the game when only one player is alive, go to GameOverWinner state.
        if (checkGameEndConditionMet()) {
          currentGameState = GameState::GameOverWinner;
          Serial.println("Entering State: GameOverWinner");
        }

        // If button A is pressed, move to player selection.
        if (buttonA.wasPressed) {
          currentGameState = GameState::PlayerSelection;
          Serial.println("Entering State: PlayerSelection");
        }

        // Update output leds.
        for (int i = 0; i < PLAYER_COUNT; i++) {
          playerLeds[i].turnOnIfAlive(players);
        }
    }
    break;
    case GameState::PlayerSelection: {
        // If currentSelectedPlayer has invalid value, select a random live player.
        if (getCurrentSelectedPlayer() == -1 || !players[getCurrentSelectedPlayer()].isAlive) {
          selectRandomLivePlayer();
        }

        // If button A is pressed, select the next live player.
        if (buttonA.wasPressed) {
          // Select next live player.
          for (int i = 1; i < PLAYER_COUNT; i++) {
            const int nextPlayerIndex = (getCurrentSelectedPlayer() + i) % 4;
            if (players[nextPlayerIndex].isAlive) {
              selectPlayer(nextPlayerIndex);
              break;
            }
          }
        }

        if (buttonB.wasPressed) {
          currentGameState = GameState::PlayerTrigger;
          Serial.println("Entering State: PlayerTrigger");
        }

        // Make selected LED blink when we're in this state.
        // Update output leds.
        for (int i = 0; i < PLAYER_COUNT; i++) {
          playerLeds[i].flashIfSelected(players);
        }
    }
    break;
    case GameState::PlayerTrigger: {
        // Perform gun trigger on selected player.
        const bool didPlayerDie = players[getCurrentSelectedPlayer()].triggerGun();

        // TODO: Change game state to new states for PlayerSurvived or PlayerDied to perform animations and sounds.

        // For the moment, send game state back to GameRunning
        currentGameState = GameState::GameRunning;
        Serial.println("Entering State: GameRunning");
    }
    break;
    case GameState::GameOverWinner: {

        // TODO: Add winning music!
        buzzer.makeNoise();

        // Flash the winning player's LED.
        for (int i = 0; i < PLAYER_COUNT; i++) {
          playerLeds[i].flashIfAlive(players);
        }
    }
    break;
    default: {
        Serial.println("State is defaulting switch block");
    }
    break;
  }
}

// Must call this function for player selection.
void selectPlayer(int playerIndex) {
  currentSelectedPlayer = playerIndex; // Keep aux variable in sync.

  for (int i = 0; i < PLAYER_COUNT; i++) {
    if (i == playerIndex) {
      players[i].isSelected = true;
    } else {
      players[i].isSelected = false;
    }
  }
}

void selectRandomLivePlayer() {
  // Function not player count safe.
  int livePlayerCount = 0;
  int livePlayerIndices[] = {-1,-1,-1,-1};

  for (int i = 0; i < PLAYER_COUNT; i++) {
    if (players[i].isAlive) {
      livePlayerCount++;
      livePlayerIndices[livePlayerCount - 1] = i;
    }
  }

  if (livePlayerCount == 0) {
    return;
  }

  if (livePlayerCount == 1) {
    selectPlayer(livePlayerIndices[0]);
    return;
  }

  int chosenRandomLivePlayer = random(0,livePlayerCount);
  selectPlayer(livePlayerIndices[chosenRandomLivePlayer]);
  return;
}

bool checkGameEndConditionMet() {
  int livePlayerCount = 0;
  for (int i = 0; i < PLAYER_COUNT; i++) {
    if (players[i].isAlive) {
      livePlayerCount++;
    }
  }

  return livePlayerCount == 1;
}