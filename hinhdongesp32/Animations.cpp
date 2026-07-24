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
  display.println("\nD.I.Y");        // Escreve um número

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


// INTERAÇÃO DE LOCAL DE RESPOSTAS
void Animations::not_wifi(){ 
  setFrameData(not_connet);
}

void Animations::noworker(){ 
  setFrameData(not_worker);
}

void Animations::bugframe(){ 
  setFrameData(not_bug);
}

// COMANDO LIGA E DESLIGA TELA.
void Animations::control_oled_power(bool enable) {
  _oled_active = enable;
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


// WORKER TRANSMISSION
/*
  OS METODOS A BAIXO SÃO REFERENTES AO SERVIDOR WORKER
*/
void Animations::drawHexFrame(const char* hexData) {
if (hexData != nullptr && strlen(hexData) < sizeof(_currentHexData)) {
        strcpy(_currentHexData, hexData); // Copia segura sem alocar memória nova
        _newFrameAvailable = true;
    }
}

void Animations::processHexFrameLoop() {
    // 🛑 Se a tela estiver desligada, ignora os frames do WebSocket para não acender a tela!
    if (!_oled_active) return;
    // 1. Só processa se houver um frame novo vindo do WebSocket
    if (!_newFrameAvailable) return;
    _newFrameAvailable = false; 

    size_t hexLength = strlen(_currentHexData); 
    if (hexLength == 0) return;

    // Garante que usaremos o espaço exato de 1024 bytes (2048 caracteres hex)
    uint8_t buffer[1024];
    memset(buffer, 0, sizeof(buffer)); // Zera a tela inteira (fundo preto)

    const char* hexDataPtr = _currentHexData;

    // Lambda para converter caracteres hexadecimais em numeração real
    auto charToNibble = [](char c) -> uint8_t {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return 0;
    };

    // Determina quantos bytes foram recebidos de fato (limita ao tamanho físico da tela)
    size_t bytesDisponiveis = hexLength / 2;
    if (bytesDisponiveis > 1024) bytesDisponiveis = 1024;

    // 2. Reconstrói o mapa de bits exatamente na ordem sequencial horizontal do .h
    for (size_t i = 0; i < bytesDisponiveis; i++) {
        char highChar = hexDataPtr[i * 2];
        char lowChar  = hexDataPtr[(i * 2) + 1];

        uint8_t highNibble = charToNibble(highChar);
        uint8_t lowNibble  = charToNibble(lowChar);

        // Mescla os nibbles para recriar o byte original (ex: 0x01, 0xFF)
        buffer[i] = (highNibble << 4) | lowNibble;
    }

    // 3. Atualiza e renderiza a tela OLED de forma horizontal (128x64)
    display.clearDisplay();
    
    // Passamos o buffer linear reconstruído diretamente para a função drawBitmap da Adafruit.
    // Como a string do Mongo seguiu o padrão de linhas do seu .h, o alinhamento ficará perfeito.
    display.drawBitmap(0, 0, buffer, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
    
    display.display();
}