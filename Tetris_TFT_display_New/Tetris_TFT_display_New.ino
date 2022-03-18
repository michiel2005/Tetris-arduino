///////////////////////////////////////////////////////////////////////////////////////////
//Not yet working or implemented list://///////////////////////////////////////////////////
//.implement a joystick control////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
#include <TFT.h>
#include <SPI.h>

#define cs 10
#define dc 9
#define rst 8
#define joyX A0
#define joyY A1

TFT TFTscreen = TFT(cs, dc, rst);

//Variables for the stats
int level = 0;
char levelC[17];
char previousLevelC[17];
int previousLevelI;
char scoreC[33];
char previousScoreC[33];
char HighScoreC[33];
long previousScoreI;
char madeLinesC[17];
char previousMadeLinesC[17];
int previousmadeLinesI;
String levelS;
long score = 0;
int totalLinesMade = 0;
long HighScore = 0;

//variables for updating the screen
bool changed = true;
bool newStats = true;

//variables for the playboard
const int8_t playRows = 16;
const int8_t playCols = 10;
const byte multiplication = 7;
const byte rowOffsetBoard = 49;
const byte colOffsetBoard = 59;
byte landed[playRows][playCols];
byte DisplayTetronominos[playRows][playCols];

//variables for the gameplay
byte currenttetronomino[5][5];
byte rotateTetronomino[5][5];
byte nextTetronomino[5][5];
int8_t lineMadeNumber[playRows];
int8_t rowOffset = -2;
int8_t colOffset = 3;
byte tetronominoType = 0;
byte nextTetronominoType;
byte rotation = 0;
bool gameOvert = false;
byte gameOverOffset = 0;

//variables for timing
unsigned long lastTick = 0;
unsigned long lastButtonTick = 0;
unsigned long fallinterval = 700;
unsigned long buttonDebouceInterfal = 200;
unsigned long momentNow;
unsigned long lastButtonPress;

void setup() {
  for (int x = 0; x < playRows; x++) {
    lineMadeNumber[x] = -1;
  }
  Serial.begin(9600);
  newSeed();

  nextTetronominoType = random(1, 8);

  TFTscreen.stroke(100, 100, 100);

  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);

  TFTscreen.begin();
  TFTscreen.background(0, 0, 0);

  newTetronomino();

  TFTscreen.setRotation(0);
  TFTscreen.setTextSize(1);
  TFTscreen.text("level", 70, 130);
  TFTscreen.text("score", 70, 110);
  TFTscreen.text("Lines made", 70, 90);
  TFTscreen.setTextSize(1);
  TFTscreen.text("Highscore:", 5, 20);
  TFTscreen.text(ltoa(HighScore, HighScoreC, 10), 65, 20);
}

void loop() {

  showStatsScreen();

  UpdateScreen();

  byte nextInputMove;
  nextInputMove = readButtons();

  momentNow = millis();
  lastButtonPress = millis();

  if (nextInputMove != 0) {
    if ( ( momentNow - lastTick) >= fallinterval) {
      lastTick = millis();
      moveDown();
    }

    if ( ( lastButtonPress - lastButtonTick) >= buttonDebouceInterfal) {
      lastButtonTick = millis();

      if (nextInputMove == 1) {
        momentNow = millis();
        moveDown();
      }
      else if (nextInputMove == 2) {
        momentNow = millis();
        MoveLeft();
      }
      else if (nextInputMove == 3) {
        momentNow = millis();
        MoveRight();
      }
      else if (nextInputMove == 4) {
        momentNow = millis();
        Rotate();
      }
    }
  }
  else if ( ( momentNow - lastTick) >= fallinterval) {
    lastTick = millis();
    moveDown();
  }
}

void moveDown() {
  checkNewLines();
  if (!detectCollisions(1)) {
    rowOffset ++;
    changed = true;
  }
  else {
    CurrentGetsLanded();
    newTetronomino();
  }
}

void  MoveLeft() {
  if (!detectCollisions(2)) {
    colOffset ++;
    changed = true;
  }
}
void  MoveRight() {
  if (!detectCollisions(3)) {
    colOffset --;
    changed = true;
  }
}
void  Rotate() {
  checkNewLines();
  // Manages the rotation of each piece
  //checks if pieces are not turned into the wall.

  if (tetronominoType != 1) {
    if (tetronominoType == 2) {
      if (rotation == 0) {
        if (4 + rowOffset < playRows && 2 + rowOffset > 0 && landed[1 + rowOffset][1 + colOffset] == 0 && landed[3 + rowOffset][1 + colOffset] == 0 && landed[4 + rowOffset][1 + colOffset] == 0) {
          currenttetronomino[2][0] = 0;
          currenttetronomino[2][2] = 0;
          currenttetronomino[2][3] = 0;
          currenttetronomino[1][1] = 2;
          currenttetronomino[3][1] = 2;
          currenttetronomino[4][1] = 2;
          rotation++;
        }
      }
      else if (rotation == 1) {
        if (3 + colOffset < playCols && 1 + colOffset > 0  && landed[3 + rowOffset][0 + colOffset] == 0 && landed[3 + rowOffset][2 + colOffset] == 0 && landed[3 + rowOffset][3 + colOffset] == 0) {
          currenttetronomino[1][1] = 0;
          currenttetronomino[2][1] = 0;
          currenttetronomino[4][1] = 0;
          currenttetronomino[3][0] = 2;
          currenttetronomino[3][2] = 2;
          currenttetronomino[3][3] = 2;
          rotation++;
        }
      }
      else if (rotation == 2) {
        if (4 + rowOffset < playRows && landed[1 + rowOffset][2 + colOffset] == 0 && landed[2 + rowOffset][2 + colOffset] == 0 && landed[4 + rowOffset][2 + colOffset] == 0) {
          currenttetronomino[3][0] = 0;
          currenttetronomino[3][1] = 0;
          currenttetronomino[3][3] = 0;
          currenttetronomino[1][2] = 2;
          currenttetronomino[2][2] = 2;
          currenttetronomino[4][2] = 2;
          rotation++;
        }
      }
      else if (rotation == 3) {
        if (3 + colOffset < playCols && 1 + colOffset > 0 && landed[2 + rowOffset][0 + colOffset] == 0 && landed[2 + rowOffset][1 + colOffset] == 0 && landed[2 + rowOffset][3 + colOffset] == 0) {
          currenttetronomino[1][2] = 0;
          currenttetronomino[3][2] = 0;
          currenttetronomino[4][2] = 0;
          currenttetronomino[2][0] = 2;
          currenttetronomino[2][1] = 2;
          currenttetronomino[2][3] = 2;
          rotation = 0;
        }
      }
    }
    else {
      if (!detectCollisions(4)) {

        //Rotates the tetronomino's.
        //lowest row is not fully functional
        //that is 1s, 2s and 3s
        bool rotateCollision = false;
        if (currenttetronomino[2][3] != 0 && (rowOffset + 3 > playRows - 2 || colOffset + 1 < 0)) {
          rotateCollision = true;
        }
        else if (currenttetronomino[3][3] != 0 && (rowOffset + 3 > playRows - 2)) {
          rotateCollision = true;
        }
        else if (currenttetronomino[4][3] != 0 && (rowOffset + 3 > playRows - 2 || colOffset + 1 > playCols - 3)) {
          rotateCollision = true;
        }

        else if (currenttetronomino[4][2] != 0 && (colOffset + 1 > playCols - 3)) {
          rotateCollision = true;
        }
        else if (currenttetronomino[4][1] != 0 && (colOffset + 1 > playCols - 3)) {
          rotateCollision = true;
        }
        else if (currenttetronomino[2][2] != 0 && (colOffset + 1 < 0)) {
          rotateCollision = true;
        }
        else if (currenttetronomino[2][1] != 0 && (colOffset + 1 < 0)) {
          rotateCollision = true;
        }


        if (rotateCollision == false) {

          currenttetronomino[2][1] = rotateTetronomino[2][3];
          currenttetronomino[2][2] = rotateTetronomino[3][3];
          currenttetronomino[2][3] = rotateTetronomino[4][3];
          currenttetronomino[3][1] = rotateTetronomino[2][2];
          currenttetronomino[3][2] = rotateTetronomino[3][2];
          currenttetronomino[3][3] = rotateTetronomino[4][2];
          currenttetronomino[4][1] = rotateTetronomino[2][1];
          currenttetronomino[4][2] = rotateTetronomino[3][1];
          currenttetronomino[4][3] = rotateTetronomino[4][1];
        }
      }
      changed = true;
    }
    changed = true;
  }
}

bool detectCollisions(byte dir) {
  bool isColliding = false;

  if (dir == 1) {
    byte lowestPoint = 0;

    for (int8_t row = 0; row < 5; row++) {
      for (int8_t col = 0; col < 5; col++) {
        if (currenttetronomino[row][col] != 0) {
          lowestPoint = row;
        }
      }
    }

    if (lowestPoint + rowOffset + 1 == playRows) {
      return true;
    }

    for (int8_t row = 0; row < 5; row++) {
      for (int8_t col = 0; col < 5; col++) {
        if (currenttetronomino[row][col] != 0 && landed[row + rowOffset + 1][col + colOffset] != 0) {
          return true;
        }
      }
    }
  }

  else if (dir == 2) {
    byte mostLeftPoint = 0;

    for (int8_t col = 0; col < 5; col++) {
      for (int8_t row = 0; row < 5; row++) {
        if (currenttetronomino[row][col] != 0) {
          mostLeftPoint = col;
        }
      }
    }

    if (mostLeftPoint + colOffset + 1 == playCols) {
      return true;
    }

    for (int8_t col = 0; col < 5; col++) {
      for (int8_t row = 0; row < 5; row++) {
        if (currenttetronomino[row][col] != 0 && landed[row + rowOffset][col + colOffset + 1] != 0) {
          return true;
        }
      }
    }
  }

  else if (dir == 3) {
    byte mostRightPoint = 0;

    for (int8_t col = 4; col >= 0; col--) {
      for (int8_t row = 0; row < 5; row++) {
        if (currenttetronomino[row][col] != 0) {
          mostRightPoint = col;
        }
      }
    }

    if (mostRightPoint + colOffset == 0) {
      return true;
    }

    for (int8_t col = 4; col >= 0; col--) {
      for (byte row = 0; row < 5; row++) {
        if (currenttetronomino[row][col] != 0 && landed[row + rowOffset][col + colOffset - 1] != 0) {
          return true;
        }
      }
    }
  }

  else if (dir == 4) {
    for (int8_t row = 0; row < 5; row++) {
      for (int8_t col = 0; col < 5; col++) {
        rotateTetronomino[row][col] = currenttetronomino[row][col];
      }
    }

    if (rotateTetronomino[2][3] != 0 && (landed[2 + rowOffset][1 + colOffset] != 0)) {
      return true;
    }
    else if (rotateTetronomino[3][3] != 0 && (landed[2 + rowOffset][2 + colOffset] != 0)) {
      return true;
    }
    else if (rotateTetronomino[4][3] != 0 && (landed[2 + rowOffset][3 + colOffset] != 0)) {
      return true;
    }
    else if (rotateTetronomino[2][2] != 0 && (landed[3 + rowOffset][1 + colOffset] != 0)) {
      return true;
    }
    else if (rotateTetronomino[3][2] != 0 && (landed[3 + rowOffset][2 + colOffset] != 0)) {
      return true;
    }
    else if (rotateTetronomino[4][2] != 0 && (landed[3 + rowOffset][3 + colOffset] != 0)) {
      return true;
    }
    else if (rotateTetronomino[2][1] != 0 && (landed[4 + rowOffset][1 + colOffset] != 0)) {
      return true;
    }
    else if (rotateTetronomino[3][1] != 0 && (landed[4 + rowOffset][2 + colOffset] != 0)) {
      return true;
    }
    else if (rotateTetronomino[4][1] != 0 && (landed[4 + rowOffset][3 + colOffset] != 0)) {
      return true;
    }
    else {
      return false;
    }
  }
  return false;
}

void newTetronomino() {
  checkNewLines();
  //this function select a random tetris piece and spawns it in the currenttetronomino array

  rotation = 0;
  tetronominoType  = nextTetronominoType;
  //tetronominoType = 4;

  if (tetronominoType == 2) {
    for (int8_t x = 3; x < 7; x++) {
      if (landed[0][x] != 0) {
        gameOver();
      }
    }
  }
  else {
    byte j;
    byte k;
    byte l;
    byte m;
    if (tetronominoType == 1) {
      j = 4;
      k = 6;
      l = 4;
      m = 6;
    }
    if (tetronominoType == 3) {
      j = 4;
      k = 6;
      l = 5;
      m = 7;
    }
    if (tetronominoType == 4) {
      j = 5;
      k = 7;
      l = 4;
      m = 6;
    }
    if (tetronominoType == 5) {
      j = 4;
      k = 5;
      l = 4;
      m = 7;
    }
    if (tetronominoType == 6) {
      j = 6;
      k = 7;
      l = 4;
      m = 7;
    }
    if (tetronominoType == 7) {
      j = 5;
      k = 6;
      l = 4;
      m = 7;
    }
    for (int8_t x = j; x < k; x++) {
      if (landed[0][x] != 0) {
        gameOver();
      }
    }
    for (int8_t x = l; x < m; x++) {
      if (landed[1][x] != 0) {
        gameOvert = true;
        gameOverOffset = 1;
      }
    }
  }

  if (tetronominoType < 1 || tetronominoType > 7) {
    tetronominoType = 1;
  }
  switch (tetronominoType) {
    case 1:
      if (gameOvert == false) {
        currenttetronomino[2][1] = 1;
        currenttetronomino[2][2] = 1;
      }
      currenttetronomino[3 - gameOverOffset][1] = 1;
      currenttetronomino[3 - gameOverOffset][2] = 1;
      if (gameOvert == true) {
        gameOver();
      }
      break;
    case 2:
      currenttetronomino[2][0] = 2;
      currenttetronomino[2][1] = 2;
      currenttetronomino[2][2] = 2;
      currenttetronomino[2][3] = 2;
      break;
    case 3:
      if (gameOvert == false) {
        currenttetronomino[2][1] = 3;
        currenttetronomino[2][2] = 3;
      }
      currenttetronomino[3 - gameOverOffset][2] = 3;
      currenttetronomino[3 - gameOverOffset][3] = 3;
      if (gameOvert == true) {
        gameOver();
      }
      break;
    case 4:
      if (gameOvert == false) {
        currenttetronomino[2][2] = 4;
        currenttetronomino[2][3] = 4;
      }
      currenttetronomino[3 - gameOverOffset][1] = 4;
      currenttetronomino[3 - gameOverOffset][2] = 4;
      if (gameOvert == true) {
        gameOver();
      }
      break;
    case 5:
      if (gameOvert == false) {
        currenttetronomino[2][1] = 5;
      }
      currenttetronomino[3 - gameOverOffset][1] = 5;
      currenttetronomino[3 - gameOverOffset][2] = 5;
      currenttetronomino[3 - gameOverOffset][3] = 5;
      if (gameOvert == true) {
        gameOver();
      }
      break;
    case 6:
      if (gameOvert == false) {
        currenttetronomino[2][3] = 6;
      }
      currenttetronomino[3 - gameOverOffset][1] = 6;
      currenttetronomino[3 - gameOverOffset][2] = 6;
      currenttetronomino[3 - gameOverOffset][3] = 6;
      if (gameOvert == true) {
        gameOver();
      }
      break;
    case 7:
      if (gameOvert == false) {
        currenttetronomino[2][2] = 7;
      }
      currenttetronomino[3 - gameOverOffset][1] = 7;
      currenttetronomino[3 - gameOverOffset][2] = 7;
      currenttetronomino[3 - gameOverOffset][3] = 7;
      if (gameOvert == true) {
        gameOver();
      }
      break;
  }
  nextTetronominoType = random(1, 8);
  updateNextTetronomino();
  changed = true;
}

byte readButtons() {
  
  if (digitalRead(2) == LOW || analogRead(joyX) < 50) {
    return 1;
  }
  else if (digitalRead(3)  == LOW || analogRead(joyY) < 50) {
    return 2;
  }
  else if (digitalRead(4)  == LOW || analogRead(joyY) > 1020) {
    return 3;
  }
  else if (digitalRead(5)  == LOW || digitalRead(7)  == LOW) {
    return 4;
  }
  return 0;
}

void checkNewLines() {
  //This script is falling apart and just sucks
  byte linesCleared = 0;
  byte lowestClearedLine;
  bool lineIsMade = false;
  bool perfectclear = true;
  byte multiplier;

  for (int8_t x = 0; x < playRows; x++) {
    bool lineIsMade = true;
    for (int8_t y = 0; y < playCols; y++) {
      if (landed[x][y] == 0) {
        lineIsMade = false;
      }
    }
    if (lineIsMade == true) {
      lineMadeNumber[x] = 1;
    }
  }

  for (int8_t x = 0; x < playRows; x++) {
    if (lineMadeNumber[x] == 1) {
      lowestClearedLine = x;
      linesCleared ++;
    }
  }

  byte rowsDeleted = 0;
  for (int8_t x = playRows - 1; x > -1; x--) {
    if (lineMadeNumber[x] == 1) {
      for (int8_t j = 0; j < playCols; j++) {
        landed[x + rowsDeleted][j] = 1;
      }
      lineMadeNumber[x] = -1;
      changed = true;
      UpdateScreen();
      delay(500);
      for (int8_t j = 0; j < playCols; j++) {
        landed[x + rowsDeleted][j] = 0;
      }
      changed = true;
      UpdateScreen();
      delay(500);
      for (int8_t p = x; p > -1; p--) {
        for (int8_t q = 0; q < playCols; q++) {
          landed[p + rowsDeleted][q] = landed[p - 1 + rowsDeleted][q];
        }
      }
      for (int8_t p = 0; p < playCols; p++) {
        landed[0][p] = 0;
      }
      rowsDeleted ++;
      totalLinesMade ++;
      newStats = true;
    }
  }

  for (int8_t x = 0; x < playRows; x++) {
    for (int8_t y = 0; y < playCols; y++) {
      if (landed[x][y] != 0) {
        perfectclear = false;
      }
    }
  }

  if (perfectclear == true) {
    multiplier  = 10;
  } else {
    multiplier  = 1;
  }

  if (level == 0 || level == 1) {
    if (linesCleared == 1) {
      score = score + (100 * multiplier);
    }
    else if (linesCleared == 2) {
      score = score + (400 * multiplier);
    }
    else if (linesCleared == 3) {
      score = score + (900 * multiplier);
    }
    else if (linesCleared == 4) {
      score = score + (2000 * multiplier);
    }
  }
  else if (level == 2 || level == 3) {
    if (linesCleared == 1) {
      score = score + (200 * multiplier);
    }
    else if (linesCleared == 2) {
      score = score + (800 * multiplier);
    }
    else if (linesCleared == 3) {
      score = score + (1800 * multiplier);
    }
    else if (linesCleared == 4) {
      score = score + (4000 * multiplier);
    }
  }
  else if (level == 4 || level == 5) {
    if (linesCleared == 1) {
      score = score + (300 * multiplier);
    }
    else if (linesCleared == 2) {
      score = score + (1200 * multiplier);
    }
    else if (linesCleared == 3) {
      score = score + (2700 * multiplier);
    }
    else if (linesCleared == 4) {
      score = score + (6000 * multiplier);
    }
  }
  else if (level == 6 || level == 7) {
    if (linesCleared == 1) {
      score = score + (400 * multiplier);
    }
    else if (linesCleared == 2) {
      score = score + (1600 * multiplier);
    }
    else if (linesCleared == 3) {
      score = score + (3600 * multiplier);
    }
    else if (linesCleared == 4) {
      score = score + (8000 * multiplier);
    }
  }
  else {
    if (linesCleared == 1) {
      score = score + (500 * multiplier);
    }
    else if (linesCleared == 2) {
      score = score + (2000 * multiplier);
    }
    else if (linesCleared == 3) {
      score = score + (4500 * multiplier);
    }
    else if (linesCleared == 4) {
      score = score + (10000 * multiplier);
    }
  }
  updateLevelStat();
}

void showStatsScreen() {
  if (newStats == true) {
    TFTscreen.setRotation(0);
    TFTscreen.setTextSize(1);

    TFTscreen.stroke(0, 0, 0);
    TFTscreen.text(itoa(previousLevelI, previousLevelC, 10), 70, 140);
    TFTscreen.text(ltoa(previousScoreI, previousScoreC, 10), 70, 120);
    TFTscreen.text(itoa(previousmadeLinesI, previousMadeLinesC, 10), 70, 100);
    TFTscreen.stroke(100, 100, 100);

    TFTscreen.text(itoa(level, levelC, 10), 70, 140);
    TFTscreen.text(ltoa(score, scoreC, 10), 70, 120);
    TFTscreen.text(itoa(totalLinesMade, madeLinesC, 10), 70, 100);
    previousLevelI = level;
    previousScoreI = score;
    previousmadeLinesI = totalLinesMade;


    TFTscreen.setRotation(1);
    newStats = false;
  }
}

void updateLevelStat() {
  if (score > 1000 && score <= 3000) {
    level = 1;
    fallinterval = 650;
  }
  if (score > 3000 && score <= 6000) {
    level = 2;
    fallinterval = 600;
  }
  if (score > 6000 && score <= 10000) {
    level = 3;
    fallinterval = 550;
  }
  if (score > 10000 && score <= 20000) {
    level = 4;
    fallinterval = 500;
  }
  if (score > 20000 && score <= 35000) {
    level = 5;
    fallinterval = 450;
  }
  if (score > 35000 && score <= 70000) {
    level = 6;
    fallinterval = 400;
  }
  if (score > 70000 && score <= 10000) {
    level = 7;
    fallinterval = 350;
  }
  if (score > 100000) {
    level = 8;
    fallinterval = 300;
  }
}
void updateNextTetronomino() {
  for (int8_t x = 0; x < 5; x++) {
    for (int8_t y = 0; y < 5; y++) {
      nextTetronomino[x][y] = 0;
    }
  }
  if (nextTetronominoType == 1) {
    nextTetronomino[2][1] = 1;
    nextTetronomino[2][2] = 1;
    nextTetronomino[3][1] = 1;
    nextTetronomino[3][2] = 1;
  }
  else if (nextTetronominoType == 2) {
    nextTetronomino[2][0] = 2;
    nextTetronomino[2][1] = 2;
    nextTetronomino[2][2] = 2;
    nextTetronomino[2][3] = 2;
  }
  else if (nextTetronominoType == 3) {
    nextTetronomino[2][1] = 3;
    nextTetronomino[2][2] = 3;
    nextTetronomino[3][2] = 3;
    nextTetronomino[3][3] = 3;
  }
  else if (nextTetronominoType == 4) {
    nextTetronomino[2][2] = 4;
    nextTetronomino[2][3] = 4;
    nextTetronomino[3][1] = 4;
    nextTetronomino[3][2] = 4;
  }
  else if (nextTetronominoType == 5) {
    nextTetronomino[2][1] = 5;
    nextTetronomino[3][1] = 5;
    nextTetronomino[3][2] = 5;
    nextTetronomino[3][3] = 5;
  }
  else if (nextTetronominoType == 6) {
    nextTetronomino[2][3] = 6;
    nextTetronomino[3][1] = 6;
    nextTetronomino[3][2] = 6;
    nextTetronomino[3][3] = 6;
  }
  else if (nextTetronominoType == 7) {
    nextTetronomino[2][2] = 7;
    nextTetronomino[3][1] = 7;
    nextTetronomino[3][2] = 7;
    nextTetronomino[3][3] = 7;
  }
  for (int8_t row = 0; row < 5; row++) {
    for (int8_t col = 0; col < 5; col++) {
      if (nextTetronomino[row][col] == 0) {
        TFTscreen.fill(0, 0, 0);
        TFTscreen.rect((row  * multiplication) + rowOffsetBoard, (col * multiplication) + 13, 5, 5);
      }
      if (nextTetronomino[row][col] == 1) {
        TFTscreen.fill(0 , 239, 255);
        TFTscreen.rect((row  * multiplication) + rowOffsetBoard, (col * multiplication) + 13, 5, 5);
      }
      if (nextTetronomino[row][col] == 2) {
        TFTscreen.fill(255, 200, 25);
        TFTscreen.rect((row  * multiplication) + rowOffsetBoard, (col * multiplication) + 13, 5, 5);
      }
      if (nextTetronomino[row][col] == 3) {
        TFTscreen.fill(0, 255, 0);
        TFTscreen.rect((row  * multiplication) + rowOffsetBoard, (col * multiplication) + 13, 5, 5);
      }
      if (nextTetronomino[row][col] == 4) {
        TFTscreen.fill(0, 0, 255);
        TFTscreen.rect((row  * multiplication) + rowOffsetBoard, (col * multiplication) + 13, 5, 5);
      }
      if (nextTetronomino[row][col] == 5) {
        TFTscreen.fill(0, 150, 255);
        TFTscreen.rect((row  * multiplication) + rowOffsetBoard, (col * multiplication) + 13, 5, 5);
      }
      if (nextTetronomino[row][col] == 6) {
        TFTscreen.fill(255, 0, 0);
        TFTscreen.rect((row  * multiplication) + rowOffsetBoard, (col * multiplication) + 13, 5, 5);
      }
      if (nextTetronomino[row][col] == 7) {
        TFTscreen.fill(255, 0, 180);
        TFTscreen.rect((row  * multiplication) + rowOffsetBoard, (col * multiplication) + 13, 5, 5);
      }
    }
  }
}
void UpdateScreen() {
  if (changed == true) {
    UpdateLanded();
    UpdateCurrent();
    UpdateDisplay();
  }
  changed = false;
}

void CurrentGetsLanded() {
  for (int row = 0; row < 5; row++) {
    for (int col = 0; col < 5; col++) {
      if (currenttetronomino[row][col] != 0) {
        landed[row + rowOffset][col + colOffset] = currenttetronomino[row][col];
        currenttetronomino[row][col] = 0;
      }
    }
  }
  rowOffset = -2;
  colOffset = 3;
}

//everything to update the screen is in the next 3 functions

void UpdateLanded() {
  for (int row = 0; row < playRows; row++) {
    for (int col = 0; col < playCols; col++) {
      DisplayTetronominos[row][col] = landed[row][col];
    }
  }
}
void UpdateCurrent() {
  for (int row = 0; row < 5; row++) {
    for (int col = 0; col < 5; col++) {
      if (currenttetronomino[row][col] != 0) {
        DisplayTetronominos[row + rowOffset][col + colOffset] = currenttetronomino[row][col];
      }
    }
  }
}

void UpdateDisplay() {
  for (int row = 0; row < playRows; row++) {
    for (int col = 0; col < playCols; col++) {
      if (DisplayTetronominos[row][col] == 0) {
        TFTscreen.fill(0, 0, 0);
        TFTscreen.rect((row  * multiplication) + rowOffsetBoard, (col * multiplication) + colOffsetBoard, 5, 5);
      }
      if (DisplayTetronominos[row][col] == 1) {
        TFTscreen.fill(0 , 239, 255);
        TFTscreen.rect((row  * multiplication) + rowOffsetBoard, (col * multiplication) + colOffsetBoard, 5, 5);
      }
      if (DisplayTetronominos[row][col] == 2) {
        TFTscreen.fill(255, 200, 25);
        TFTscreen.rect((row  * multiplication) + rowOffsetBoard, (col * multiplication) + colOffsetBoard, 5, 5);
      }
      if (DisplayTetronominos[row][col] == 3) {
        TFTscreen.fill(0, 255, 0);
        TFTscreen.rect((row  * multiplication) + rowOffsetBoard, (col * multiplication) + colOffsetBoard, 5, 5);
      }
      if (DisplayTetronominos[row][col] == 4) {
        TFTscreen.fill(0, 0, 255);
        TFTscreen.rect((row  * multiplication) + rowOffsetBoard, (col * multiplication) + colOffsetBoard, 5, 5);
      }
      if (DisplayTetronominos[row][col] == 5) {
        TFTscreen.fill(0, 150, 255);
        TFTscreen.rect((row  * multiplication) + rowOffsetBoard, (col * multiplication) + colOffsetBoard, 5, 5);
      }
      if (DisplayTetronominos[row][col] == 6) {
        TFTscreen.fill(255, 0, 0);
        TFTscreen.rect((row  * multiplication) + rowOffsetBoard, (col * multiplication) + colOffsetBoard, 5, 5);
      }
      if (DisplayTetronominos[row][col] == 7) {
        TFTscreen.fill(255, 0, 180);
        TFTscreen.rect((row  * multiplication) + rowOffsetBoard, (col * multiplication) + colOffsetBoard, 5, 5);
      }
    }
  }
}

void gameOver() {
  changed = true;
  UpdateScreen();

  delay(2000);

  TFTscreen.fillScreen(0);

  TFTscreen.setRotation(0);
  TFTscreen.setTextSize(1);

  if (score < HighScore) {
    TFTscreen.text("You have failed", 0, 10);
    TFTscreen.text("The highscore was", 0, 30);
    TFTscreen.text(ltoa(HighScore, HighScoreC, 10), 0, 50);
    TFTscreen.text("Your score was", 0, 70);
    TFTscreen.text(ltoa(score, scoreC, 10), 0, 90);
  }
  else if (score > HighScore) {
    TFTscreen.text("You were great", 0, 10);
    TFTscreen.text("Your score was", 0, 30);
    TFTscreen.text(ltoa(score, scoreC, 10), 0, 50);
    TFTscreen.text("The highscore was", 0, 70);
    TFTscreen.text(ltoa(HighScore, HighScoreC, 10), 0, 90);
  }
  else if (score == HighScore) {
    TFTscreen.text("That sucks", 0, 10);
    TFTscreen.text("The highscore was", 0, 30);
    TFTscreen.text(ltoa(HighScore, HighScoreC, 10), 0, 50);
    TFTscreen.text("Your score was", 0, 70);
    TFTscreen.text(ltoa(score, scoreC, 10), 0, 90);
  }

  if (score > HighScore) {
    HighScore = score;
  }

  fallinterval = 700;
  score = 0;
  level = 0;
  totalLinesMade = 0;

  for (int8_t row = 0; row < playRows; row ++) {
    for (int8_t col = 0; col < playCols; col++) {
      landed[row][col] = 0;
      DisplayTetronominos[row][col] = 0;
    }
  }
  for (int8_t row = 0; row < 5; row ++) {
    for (int8_t col = 0; col < 5; col++) {
      currenttetronomino[row][col] = 0;
      nextTetronomino[row][col] = 0;
    }
  }

  while (digitalRead(6) == HIGH && analogRead(joyX) < 1020) {}
  
  TFTscreen.fillScreen(0);
  
  changed = true;
  newStats = true;
  gameOvert = false;
  gameOverOffset = 0;
  TFTscreen.setRotation(0);
  TFTscreen.setTextSize(1);
  TFTscreen.text("level", 70, 130);
  TFTscreen.text("score", 70, 110);
  TFTscreen.text("Lines made", 70, 90);
  TFTscreen.setTextSize(1);
  TFTscreen.text("Highscore:", 5, 20);
  TFTscreen.text(ltoa(HighScore, HighScoreC, 10), 65, 20);

  TFTscreen.setRotation(1);

  newTetronomino();

}

void newSeed() {
  int seed = (analogRead(0) + 1) * (analogRead(1) + 1) * (analogRead(2) + 1) * (analogRead(3) + 1) * millis();
  randomSeed(seed);
  random(10, 9610806);
  seed = seed * random(3336, 15679912) + analogRead(random(4)) ;
  randomSeed(seed);
  random(10, 98046);
}
