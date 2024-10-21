#include "pitches.h"



// START ====== DATA TYPES ====================

struct Player {
public:
  bool isAlive;
  int* chamber;
  int clickIndex;
  int ledPin;

  // Default constructor
  Player() : isAlive(false), chamber(nullptr), clickIndex(-1), ledPin(-1) {
      // Optionally, you can initialize chamber to null or skip allocation here
  }

// custom constructor that randomizes chamber.
  Player(int ledPinIn){

    isAlive = true;
    clickIndex = 0;
    ledPin = ledPinIn;

    chamber = new int[6];

    for (int i = 0; i < 6; i++) {
      chamber[i] = 0;
    }
    chamber[random(0,6)] = 1;
  }

  // Pretty prints the player's info and full chamber status
  void printInfo() {
    Serial.print("Player:: ");
    Serial.print(" ledPin: ");
    Serial.print(ledPin);
    Serial.print("; isAlive: ");
    Serial.print(isAlive);
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
  int state;
  int pin;
  int playerIndex;

  // Default constructor
  PlayerLed() : state(-1), pin(-1), playerIndex(-1) { }
  
  // Custom constructor
  PlayerLed(int pinIn, int playerIndexIn) {
    state = HIGH;
    pin = pinIn;
    playerIndex = playerIndexIn;
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

    return *this;
  }

};



// END  ======  DATA TYPES ====================


// START ====== GLOBAL STUFF ====================

#define PLAYER_COUNT 4
byte startingPlayer = -1;
Player* players = new Player[PLAYER_COUNT];

// LEDs
const int playerLedPins[] = {2,3,4,5}; // Need to add more pins if more players wanted.
PlayerLed* playerLeds = new PlayerLed[PLAYER_COUNT];
const int ledBlinkDuration = 300;
const int ledIntervalDuration = 500;

// Buzzer
const int buzzerPin = 6;

//  Button
const int buttonAPin = 7;
const int buttonBPin = 8;
const int buttonInterval = 300; // number of millisecs between button readings

// Potentiometer
const int knobPin = A0;


// Time
unsigned long currentMillis = 0;    // stores the value of millis() in each iteration of loop()

// END ====== GLOBAL STUFF ====================


// Player state vars
const int playerCount = 4; // Total players playing
int activePlayerCount = 4; // Players that are alive
byte activePlayers = 0b1111; // Byte with lower 4 bits representing which players are still alive
int playerChambers[playerCount][6]; // Represent each player's chamber
int currentHighlightedPlayer = -1; // State for player selection
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
enum GameState: int {
  FirstSetup,
  PlayerIndication,
  GameRunning,
  PlayerSelection, // <-
  PlayerTrigger,
  SecondarySetup,
  GameOverWinner
};

// byte currentGameState = GameState::PlayerIndication;
GameState currentGameState = FirstSetup;

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(A5));

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

  newSetup();
}

void newSetup() {
  // Serial.begin(9600);
  // randomSeed(analogRead(A5));

  // Pin setups
  Serial.println("(NEW) Setting up game for 4 players");

  // Create the players.
  for (int i = 0; i < PLAYER_COUNT; i++) {
    players[i] = Player(playerLedPins[i]);
    players[i].printInfo();
    playerLeds[i] = PlayerLed(playerLedPins[i], i);
    playerLeds[i].printInfo();
  }

  // currentGameState = PlayerIndication; // go straight to indication
}

void loop() {

  currentMillis = millis();   // capture the latest value of millis()

  // Daniel TODO: Read all inputs here.

  switch (currentGameState) {
    case FirstSetup:
        // Serial.println("State is FirstSetup");
        doFirstSetup(playerCount, activePlayers, playerChambers);
        currentGameState = PlayerIndication;
      break;
    case PlayerIndication:
        // Serial.println("State is PlayerIndication");
        doPlayerIndication();
        currentGameState = GameRunning;
      break;
    case GameRunning:
        Serial.println("State is GameRunning");
        doGameRunning(playerCount, activePlayers);
        // If input is received to select a player, change state
        currentGameState = PlayerSelection;
      break;
    case PlayerSelection:
        Serial.println("State is PlayerSelection");
        doPlayerSelection();
        currentGameState = PlayerTrigger;
      break;
    case PlayerTrigger:
        Serial.println("State is PlayerTrigger");
        doPlayerTrigger();
        currentGameState = GameRunning;
      break;
    default:
        Serial.println("State is defaulting switch block");
      break;
  }

  delay(50);
}

void readAllInputs() {
  // Read button A


  // Read button B


}

void loadPlayerChambers(int playerChambers[][6], int players) {
  for (int i = 0; i < players; i++) {
    // Load bullet at a random place en each chamber
    playerChambers[i][random(0,6)] = 1;
  }
}

void printPlayerChambers(int playerChambers[][6], int players) {
  for (int i = 0; i < players; i++) {
    for (int j = 0; j < 6; j++) {
      Serial.print(playerChambers[i][j]);
    }
    Serial.println();
  }
}

void printActivePlayers(byte activePlayers, int playerCount) {
  for (int i = 0; i < playerCount; i++) {
    boolean isPlayerActive = checkIfPlayerActive(i,activePlayers);
    String playerString = "Player " + String(i);
    String activeString = isPlayerActive ?       
      " is active" :
      " is inactive";
    Serial.println(playerString + activeString);
  }
}

boolean checkIfPlayerActive(int queriedPlayer, byte activePlayers) {
  byte playerMask = 1 << queriedPlayer;
  boolean isPlayerActive = playerMask & activePlayers;
  return isPlayerActive;
}

void playSound() {
  // tone(buzzerPin, NOTE_G5, 500); // Muting for now...
}

void highlightPlayerAnimation(int player, int seconds) {
  // Turn all LEDs off.
  for (int i = 0; i < 4; i++) {
    digitalWrite(playerLedPins[i], LOW);
  }

  int repetitions = 5 * seconds;

  // Turn player LED on and Off for a period of time. (~ 3 seconds)
  playSound();
  for (int i = 0; i < repetitions; i++) {
    digitalWrite(playerLedPins[player], HIGH);
    delay(100);
    digitalWrite(playerLedPins[player], LOW);
    delay(100);
  }
}

void doFirstSetup(int playerCount, byte activePlayers, int playerChambers[][6]) {
  Serial.println("Performing First Setup");
  Serial.println("Loading bullets in chamber");
  // Load player chambers
  loadPlayerChambers(playerChambers, playerCount);
  // print to check.
  printPlayerChambers(playerChambers, playerCount);
  // print active players
  printActivePlayers(activePlayers, playerCount);
  Serial.println("Finish First Setup");
}

void doPlayerIndication() {
  Serial.println("Performing Player Indication");
    // Select starting player.
  startingPlayer = random(0,playerCount);
  Serial.println("Starting player is Player " + String(startingPlayer));
  highlightPlayerAnimation(startingPlayer, 3);
  Serial.println("Finish Player Indication");
}

void doGameRunning(int playerCount, byte activePlayers) {
  Serial.println("Performing GameRunning");
  // Turn on all LEDs for active players.
  for (int i = 0; i < playerCount; i++) {
    digitalWrite(playerLedPins[i], LOW); // turn off first
    if (checkIfPlayerActive(i, activePlayers)) {
      digitalWrite(playerLedPins[i], HIGH); // turn on active player
    }
  }

  readButtonPress(buttonBPin); // will stay here until button is pressed.

  Serial.println("Finish GameRunning");
}

void doPlayerSelection() {
  boolean highlightFlashOn = false;

  // Select a player to highlight
  doPlayerSelection_selectAPlayer();

  while (true) {

    // Flash currently highlighted player
    highlightFlashOn = doPlayerSelection_flashHighlightedPlayer(highlightFlashOn);

    // MISSING: Show highlighted player triggers and chamber size on some sort of display.

    if (currentSelectionMode == Knob) {
      // Read knob direction and change player selection
      int knobDirection = readKnobDirection();
      Serial.print("knobDirection: ");
      Serial.print(knobDirection);
      Serial.print(" - currentHighlightedPlayer: ");
      if (knobDirection != 0) {
        doPlayerSelection_updateHighlightedPlayer(knobDirection);
      }
      Serial.println(currentHighlightedPlayer);
    } else {
      if (digitalRead(buttonBPin) == LOW) {
        doPlayerSelection_updateHighlightedPlayer(1); // Select clockwise active player
        Serial.println("Player selection updated via button");
      }
    }

    if (digitalRead(buttonAPin) == LOW) {
      break;
    }
    // Read shoot button press, call logic and then exit loop if pressed
    // Note: Probably more elegant to change state and handle animation/sounds in new state.

    delay(100); //  This delay value important and can affect selection knob accuracy.
  }

  
}

void doPlayerTrigger() {
      const auto shoot = playerChambersIndexes[currentHighlightedPlayer];
      const auto result = playerChambers[currentHighlightedPlayer][shoot];
      if (result == 1)
      {
          killPlayer(currentHighlightedPlayer);
          return;
      }
      playerChambersIndexes[currentHighlightedPlayer] = shoot + 1;
}

void killPlayer(int playerIndex) {
  byte playerMask = ~(1 << playerIndex);
  activePlayers &= playerMask;
  Serial.println("ActivePlayers " + String(activePlayers));
}

void doPlayerSelection_selectAPlayer() {
  // currentHighlightedPlayer
  int randomPlayer = random(0,activePlayerCount);
  int currentCheckedPlayers = 0;
  int selectedPlayer = -1;

  for (int i = 0; i < playerCount; i++) {
    if (checkIfPlayerActive(i,activePlayers)) {
      if (randomPlayer == currentCheckedPlayers) {
        selectedPlayer = i;
        break;
      } else {
        currentCheckedPlayers++;
      }
    }
  }

  currentHighlightedPlayer = selectedPlayer;
}

boolean doPlayerSelection_flashHighlightedPlayer(boolean highlightFlashOn) {
  digitalWrite(playerLedPins[currentHighlightedPlayer], highlightFlashOn ? HIGH : LOW);
  return !highlightFlashOn;
}

void doPlayerSelection_updateHighlightedPlayer(int direction) {
  turnPlayerLedsOff(); // Makes selecting easier to see
  if (direction > 0) {
    currentHighlightedPlayer = getNextActivePlayerClockwise();
  } else {
    currentHighlightedPlayer = getNextActivePlayerAntiClockwise();
  }
  // digitalWrite(playerLedPins[currentHighlightedPlayer], HIGH);
}

int getNextActivePlayerClockwise() {
  // Look at next incremental player and so on until the first active one is found.
  // Loop around using modulo.
  for (int i = 1; i < playerCount; i++) {
    int nextClockWisePlayer = (currentHighlightedPlayer + i) % playerCount;
    if (checkIfPlayerActive(nextClockWisePlayer, activePlayers)) {
      // We found the player
      return nextClockWisePlayer;
    }
  }
  return -1; // Should not get here
}

int getNextActivePlayerAntiClockwise() {
  // Look at previous increment player and so on until the first active one is found.
  // We can add amounts of (playerCount - 1) and use modulo to effectively find the index of the previous player.
  for (int i = 1; i < playerCount; i++) {
    int nextAntiClockWisePlayer = (currentHighlightedPlayer + (i*(playerCount - 1))) % playerCount;
    if (checkIfPlayerActive(nextAntiClockWisePlayer, activePlayers)) {
      // We found the player
      return nextAntiClockWisePlayer;
    }
  }
  return -1; // Should not get here
}

void turnPlayerLedsOff() {
  for (int i = 0; i < playerCount; i++) {
    digitalWrite(playerLedPins[i], LOW);
  }
}

void turnPlayerLedsOn() {
  for (int i = 0; i < playerCount; i++) {
    digitalWrite(playerLedPins[i], HIGH);
  }
}

// This blocks process until the button is pressed.
void readButtonPress(int buttonPin) {
  while(true) {
    if (digitalRead(buttonPin) == LOW) {
      Serial.println("Button pressed, continuing.");
      break;
    }
    delay(1);
  }
}

// Return 0 if no change.
// Return 1 if clockwise.
// Return -1 if anticlockwise.
int readKnobDirection() {

  if (!knobReadSet) {
    previousKnobRead = analogRead(knobPin);
    knobReadSet = true;
    return 0;
  }

  int newRead = analogRead(knobPin);
  int delta = newRead - previousKnobRead; // delta is positive if pot is turned clockwise

  // update previous read
  previousKnobRead = newRead;

  // Report no direction if change was not above sensitivity threshold.
  if (abs(delta) < knobSensitivity) {
    return 0;
  }

  if (delta > 0) {
    return 1;
  } else {
    return -1;
  }
}