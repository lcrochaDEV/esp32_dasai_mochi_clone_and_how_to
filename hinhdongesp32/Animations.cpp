/*
 * OLED Animation Advanced Example (24FPS)
 * Tạo bởi OLED Frame Generator
 * @khoi2mai
 * 
 * Số lượng frame: 772
 * Kích thước: 128x64 pixels
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "all_frames.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Cấu hình animation
#define FRAME_DELAY 42  // 24FPS

#include "Animations.h"

Animations::Animations(){}

void Animations::helloWordMochi(String consoleText){
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Falha no display"));
    for(;;);
  }

  display.clearDisplay();
  display.setTextSize(1);      
  display.setTextColor(SSD1306_WHITE); 
  display.setCursor(0, 10);      
  display.println("Ola Mundo!"); // println pula para a próxima linha
  
  display.setTextSize(1);        // Aumenta o tamanho
  display.setCursor(0, 30);      // Move o cursor para baixo
  display.print(consoleText);    // Escreve sem pular linha
  display.print("\nD.I.Y");        // Escreve um número

  // --- O Passo Final ---
  // Nada aparece até você chamar este comando:
  display.display();
  delay(5000);
}

void Animations::animationsLoop(){
  static unsigned long lastFrameTime = 0;
  static int currentFrame = 0;
  unsigned long currentTime = millis();
  if (currentTime - lastFrameTime >= FRAME_DELAY) {
    lastFrameTime = currentTime;
    display.clearDisplay();
    display.drawBitmap(0, 0, frames[currentFrame], FRAME_WIDTH, FRAME_HEIGHT, SSD1306_WHITE);
    display.display();
    currentFrame = (currentFrame + 1) % TOTAL_FRAMES;
  }
}

void Animations::setFrameData(const unsigned char* frameData[]){
  static unsigned long lastFrameTime = 0;
  static int currentFrame = 0;
  unsigned long currentTime = millis();
  if (currentTime - lastFrameTime >= FRAME_DELAY) {
    lastFrameTime = currentTime;
    display.clearDisplay();
    display.drawBitmap(0, 0, frameData[currentFrame], FRAME_WIDTH, FRAME_HEIGHT, SSD1306_WHITE);
    display.display();
    currentFrame = (currentFrame + 1) % TOTAL_FRAMES;
  }
}

void Animations::not_wifi(){
  setFrameData(not_connet);
}

void Animations::control_oled_power(bool enable) {
    if (enable) {
        // LIGA o display (sai do modo de suspensão)
        display.ssd1306_command(SSD1306_DISPLAYON);
        Serial.println("Display OLED LIGADO.");
    } else {
        // DESLIGA o display (entra em modo de suspensão)
        // O comando SSD1306_DISPLAYOFF coloca o controlador do display em sleep mode.
        display.ssd1306_command(SSD1306_DISPLAYOFF);
        // Opcional: Limpar a tela antes de desligar completamente
        display.clearDisplay();
        display.display();
        Serial.println("Display OLED DESLIGADO (Sleep Mode).");
    }
}
