#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

U8G2_ST7920_128X64_F_HW_SPI u8g2(U8G2_R0, /* CS=*/ 15); // SCK 14 = E, MOSI 13 = R/W, CS 15 = RS

// Game parameters
const int groundLevel = 50;
const int catWidth = 15;
const int catHeight = 10;
const int obstacleWidth = 8;
const int obstacleHeight = 10;
const int cloudWidth = 14;
const int cloudHeight = 6;
const int cloud2Width = 18;
const int cloud2Height = 10;
const int jumpHeight = 20;
const int gravity = 2;
const int jumpSpeed = 10.8;
const int obstacleSpeed = 5;
const int cloudSpeed = 2;

// Cat properties
int catX = 15;
int catY = groundLevel;
int catVelocityY = 0;
bool isJumping = false;

// Obstacle properties
int obstacleX = 128;
int obstacleY = 50;

// Cloud properties
int cloudX = 128;
int cloudY = 12;
int cloud2X = 78;
int cloud2Y = 8;

// Button and Buzzer pins
const int jumpButtonPin = 12;
const int buzzerPin = 2;

// Game state
bool gameOver = false;
int score = 0;

// 'SpriteKucing1', 15x10px
const unsigned char bitmap_SpriteKucing1 [] PROGMEM = {
    0x01, 0x14, 0x01, 0x1c, 0x01, 0x3e, 0xfe, 0x2b, 0xfc, 0x3f, 0xfc, 0x1f, 0xfc, 0x0f, 0x04, 0x05, 
    0x02, 0x0a, 0x01, 0x14
};
// 'SpriteKucing2', 15x10px
const unsigned char bitmap_SpriteKucing2 [] PROGMEM = {
    0x01, 0x14, 0x01, 0x1c, 0x01, 0x3e, 0xfe, 0x2b, 0xfc, 0x3f, 0xfc, 0x1f, 0xfc, 0x0f, 0x04, 0x05, 
    0x04, 0x05, 0x04, 0x05
};
// 'SpriteKucing3', 15x10px
const unsigned char bitmap_SpriteKucing3 [] PROGMEM = {
    0x01, 0x14, 0x01, 0x1c, 0x01, 0x3e, 0xfe, 0x2b, 0xfc, 0x3f, 0xfc, 0x1f, 0xfc, 0x0f, 0x04, 0x05, 
    0x88, 0x02, 0x50, 0x01
};

// Array of all bitmaps for convenience.
const int bitmap_allArray_LEN = 3;
const unsigned char* bitmap_allArray[3] = {
    bitmap_SpriteKucing1,
    bitmap_SpriteKucing2,
    bitmap_SpriteKucing3
};

// 'Awan', 14x6px
const unsigned char epd_bitmap_Awan [] PROGMEM = {
    0x30, 0x00, 0x38, 0x00, 0x7c, 0x0c, 0xfe, 0x1f, 0xff, 0x3f, 0xff, 0x3f
};

// 'Awan2', 18x10px
const unsigned char epd_bitmap_Awan2 [] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x3c, 0x00, 0x18, 0x7e, 0x00, 0x1c, 
    0xff, 0x00, 0x3c, 0xff, 0x00, 0xfe, 0xff, 0x01, 0xfe, 0xff, 0x01, 0xff, 0xff, 0x03
};

// 'TongSampah', 8x10px
const unsigned char epd_bitmap_TongSampah [] PROGMEM = {
    0x7e, 0xc3, 0x87, 0xfd, 0xa5, 0xad, 0xa9, 0xa9, 0xeb, 0x7e
};

int currentFrame = 0;
unsigned long lastFrameChange = 0;
const int frameInterval = 100;

void setup() {
  u8g2.begin();
  u8g2.setContrast(255);

  pinMode(jumpButtonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);  // Ensure the buzzer is off initially
}

void loop() {
  if (digitalRead(jumpButtonPin) == LOW && !isJumping && !gameOver) {
    catVelocityY = -jumpSpeed;
    isJumping = true;
  }

  catY += catVelocityY;
  catVelocityY += gravity;

  if (catY >= groundLevel) {
    catY = groundLevel;
    isJumping = false;
  }

  obstacleX -= obstacleSpeed;

  if (obstacleX < -obstacleWidth) {
    obstacleX = 128;
    score += 1;
  }

  cloudX -= cloudSpeed;
  cloud2X -= cloudSpeed;

  if (cloudX < -cloudWidth) {
    cloudX = 128;
  }

  if (cloud2X < -cloudWidth) {
    cloud2X = 128;
  }

  if (obstacleX < catX + catWidth && obstacleX + obstacleWidth > catX && catY + catHeight > groundLevel - obstacleHeight) {
    gameOver = true;
    tone(buzzerPin, 1000, 500);  // Play buzzer sound at 1000 Hz for 500 ms
  }

  if (millis() - lastFrameChange > frameInterval) {
    currentFrame = (currentFrame + 1) % 3;
    lastFrameChange = millis();
  }

  u8g2.firstPage();
  do {
    if (gameOver) {
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(34, 25, "Game Over!");
      u8g2.setCursor(41, 35);
      u8g2.print("Score: ");
      u8g2.print(score);
      u8g2.drawStr(5, 45, "Press RST to Restart");
    } else {
      u8g2.drawLine(0, groundLevel + catHeight, 128, groundLevel + catHeight);
      u8g2.drawXBMP(catX, catY, catWidth, catHeight, bitmap_allArray[currentFrame]);
      u8g2.drawXBMP(obstacleX, obstacleY, obstacleWidth, obstacleHeight, epd_bitmap_TongSampah);
      u8g2.drawXBMP(cloudX, cloudY, cloudWidth, cloudHeight, epd_bitmap_Awan);
      u8g2.drawXBMP(cloud2X, cloud2Y, cloud2Width, cloud2Height, epd_bitmap_Awan2);

      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.setCursor(0, 7);
      u8g2.print("Score: ");
      u8g2.print(score);
    }
  } while (u8g2.nextPage());

  if (gameOver) {
    delay(2000);
    catY = groundLevel;
    obstacleX = 128;
    cloudX = 128;
    cloud2X = 78;
    gameOver = false;
    isJumping = false;
    catVelocityY = 0;
    score = 0;
    noTone(buzzerPin);  // Stop buzzer sound
  }

  delay(90);
}
