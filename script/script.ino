#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

#define NOTE_C4 262
#define NOTE_D4 294
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_G4 392
#define NOTE_A4 440
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_D5 587
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_G5 784
#define NOTE_A5 880
#define NOTE_B5 988

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int buzzer = 14; // D6
const int leftButton = 25;  //D2
const int rightButton = 26; //D3

const float songSpeed = 1.0;  // Adjust speed of music

int notes[] = {
    NOTE_E4, NOTE_G4, NOTE_A4, NOTE_A4, 0,
    NOTE_A4, NOTE_B4, NOTE_C5, NOTE_C5, 0,
    NOTE_C5, NOTE_D5, NOTE_B4, NOTE_B4, 0,
    NOTE_A4, NOTE_G4, NOTE_A4, 0
};

int durations[] = {
    125, 125, 250, 125, 125,
    125, 125, 250, 125, 125,
    125, 125, 250, 125, 125,
    125, 125, 375, 125
};

const int totalNotes = sizeof(notes) / sizeof(int);

const int yLine = 20;
int gameOver;
int score;

struct Arrow {
  int x;
  int y;
  int direction;
  int playerReacted;
  bool active;
};

const int maxArrows = 5;
Arrow arrows[maxArrows];

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  pinMode(buzzer, OUTPUT);
  pinMode(leftButton, INPUT);
  pinMode(rightButton, INPUT);
  randomSeed(analogRead(0));

  gameOver = 0;
  score = 0;
  for (int i = 0; i < maxArrows; i++) {
    arrows[i] = {0, 0, 0, 0, false};
  }
}

void drawArrow(int x, int y, int direction) {
  if (direction == 1) {
    display.fillTriangle(x, y, x, y + 10, x + 8, y + 5, WHITE);
    display.fillRect(x - 5, y + 3, 6, 4, WHITE);
  } else {
    display.fillTriangle(x, y + 5, x + 8, y, x + 8, y + 10, WHITE);
    display.fillRect(x + 7, y + 3, 6, 4, WHITE);
  }
}

void loop() {
  int leftButtonState = digitalRead(leftButton);
  int rightButtonState = digitalRead(rightButton);
  
  if(gameOver == 1){ // print game over message and wait for restart

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(35, 30);
    display.println("GAME OVER");
    display.setCursor(35, 40);
    display.print("SCORE: ");
    display.println(score);
    display.display();

    if(leftButtonState && rightButtonState){
      gameOver = 0;
      score = 0;
      for (int i = 0; i < maxArrows; i++) {
        arrows[i] = {0, 0, 0, 0, false};
      }
    }

    delay(400);

  }else{

    static int noteIndex = 0;

    // Play the next note
    if (noteIndex < totalNotes) {
      int currentNote = notes[noteIndex];
      int wait = durations[noteIndex] / songSpeed;

      if (currentNote != 0) {
        tone(buzzer, currentNote, wait);
      } else {
        noTone(buzzer);
      }
      delay(wait);
      
      noteIndex++;
    } else {
      noteIndex = 0; // Restart song when finished
    }

    // Update Arrows
    for (int i = 0; i < maxArrows; i++) {
      if (!arrows[i].active) {
        if (random(0, 100) < 5) { // we spawn a new arrow with 5% chance
          arrows[i] = {random(10, SCREEN_WIDTH - 10), SCREEN_HEIGHT, random(0, 2), 0, true};
        }
      } else {
        arrows[i].y -= 2; // Move up!!
        if (arrows[i].y < 0) {
          arrows[i].active = false; // Reset when out of screen
          if(arrows[i].playerReacted == 0){
            gameOver = 1;
          }else{
            score += 1;
          }
        }
      }
    }

    // Draw Arrows
    display.clearDisplay();
    for (int i = 0; i < maxArrows; i++) {
      if (arrows[i].active) {
        drawArrow(arrows[i].x, arrows[i].y, arrows[i].direction);
        if(arrows[i].y <= yLine + 10 && arrows[i].y >= yLine - 10){ //arrow near the line
          if((arrows[i].direction == 1 && rightButtonState == 1) || (arrows[i].direction == 0 && leftButtonState == 1)){//and correct button is pressed
            arrows[i].playerReacted = 1;
          }
        }
      }
    }
    display.drawLine(0, yLine, 127, yLine, WHITE);
    display.display();
    delay(50);
  }
}
