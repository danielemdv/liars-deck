#include "pitches.h"

// Player state vars
const int playerCount = 4; // Total players playing
int activePlayerCount = 4; // Players that are alive
byte activePlayers = 0b1111; // Byte with lower 4 bits representing which players are still alive
byte playersTurn = 0; // not used, should delete
int playerLed[] = {2,3,4,5};
int playerChambers[playerCount][6]; // Represent each player's chamber
int currentHighlightedPlayer = -1; // State for player selection
int playerChambersIndexes[playerCount] = {};

// Buzzer
int buzzerPin = 6;

//  Button
int greenButtonPin = 7;
int redButtonPin = 8;

// Potentiometer
int knobPin = A0;
int previousKnobRead = 0;
boolean knobReadSet = false;
int knobSensitivity = 50;


// Selection input type
enum SelectionMode: int {
  Knob,
  Button
};

SelectionMode currentSelectionMode = Knob; // Change this depending on secondary input type

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
    pinMode(playerLed[i], OUTPUT);
  }
  pinMode(buzzerPin, OUTPUT);
  pinMode(greenButtonPin, INPUT_PULLUP);
  pinMode(redButtonPin, INPUT_PULLUP);
  pinMode(knobPin, INPUT);
}

void loop() {

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
    digitalWrite(playerLed[i], LOW);
  }

  int repetitions = 5 * seconds;

  // Turn player LED on and Off for a period of time. (~ 3 seconds)
  playSound();
  for (int i = 0; i < repetitions; i++) {
    digitalWrite(playerLed[player], HIGH);
    delay(100);
    digitalWrite(playerLed[player], LOW);
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
  playersTurn = random(0,playerCount);
  Serial.println("Starting player is Player " + String(playersTurn));
  highlightPlayerAnimation(playersTurn, 3);
  Serial.println("Finish Player Indication");
}

void doGameRunning(int playerCount, byte activePlayers) {
  Serial.println("Performing GameRunning");
  // Turn on all LEDs for active players.
  for (int i = 0; i < playerCount; i++) {
    digitalWrite(playerLed[i], LOW); // turn off first
    if (checkIfPlayerActive(i, activePlayers)) {
      digitalWrite(playerLed[i], HIGH); // turn on active player
    }
  }

  readButtonPress(redButtonPin); // will stay here until button is pressed.

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
      if (digitalRead(redButtonPin) == LOW) {
        doPlayerSelection_updateHighlightedPlayer(1); // Select clockwise active player
        Serial.println("Player selection updated via button");
      }
    }

    if (digitalRead(greenButtonPin) == LOW) {
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
  digitalWrite(playerLed[currentHighlightedPlayer], highlightFlashOn ? HIGH : LOW);
  return !highlightFlashOn;
}

void doPlayerSelection_updateHighlightedPlayer(int direction) {
  turnPlayerLedsOff(); // Makes selecting easier to see
  if (direction > 0) {
    currentHighlightedPlayer = getNextActivePlayerClockwise();
  } else {
    currentHighlightedPlayer = getNextActivePlayerAntiClockwise();
  }
  // digitalWrite(playerLed[currentHighlightedPlayer], HIGH);
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
    digitalWrite(playerLed[i], LOW);
  }
}

void turnPlayerLedsOn() {
  for (int i = 0; i < playerCount; i++) {
    digitalWrite(playerLed[i], HIGH);
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