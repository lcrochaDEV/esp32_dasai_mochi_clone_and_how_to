#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "all_frames.h"
#include "Animations.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C
#define FRAME_DELAY 42

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Animations::Animations() {
    memset(_currentHexData, 0, sizeof(_currentHexData));
}

void Animations::helloWordMochi(String consoleText) {
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("Falha no display"));
        return;
    }
    display.clearDisplay();
    display.setTextSize(1);      
    display.setTextColor(SSD1306_WHITE); 
    display.setCursor(0, 10);      
    display.println("Ola Mundo!"); 
    display.setCursor(0, 30);      
    display.print(consoleText);    
    display.println("\nD.I.Y");        
    display.display();
    delay(3000); // Reduzido para evitar trava prolongada no boot
}


void Animations::setFrameData(const unsigned char* frameData[], size_t frameCount) {
    if (!_oled_active || frameCount == 0) return;

    static unsigned long lastFrameTime = 0;
    static int currentFrame = 0;
    static const unsigned char** lastAnimation = nullptr;
    
    // Reseta o quadro se a animação mudar (evita artefatos)
    if (lastAnimation != frameData) {
        currentFrame = 0;
        lastAnimation = frameData;
    }

    if (millis() - lastFrameTime >= FRAME_DELAY) {
        lastFrameTime = millis();
        display.clearDisplay();
        display.drawBitmap(0, 0, frameData[currentFrame], SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
        display.display();
        
        currentFrame = (currentFrame + 1) % frameCount;
    }
}

void Animations::defaultlocal() { setFrameData(default_local, TOTAL_FRAMES); }   // Fallback padrão
void Animations::notwifi() { setFrameData(notconnet, sizeof(notconnet)/sizeof(notconnet[0])); }
void Animations::noworker() { setFrameData(notworker, sizeof(notworker)/sizeof(notworker[0])); }
void Animations::bugframe() { setFrameData(notbug, sizeof(notbug)/sizeof(notbug[0])); }

void Animations::control_oled_power(bool enable) {
    // 🛡️ Proteção I2C: Ignora se a tela já estiver no estado desejado
    if (_oled_active == enable) return; 
    
    _oled_active = enable;
    if (enable) {
        display.ssd1306_command(SSD1306_DISPLAYON);
    } else {
        display.ssd1306_command(SSD1306_DISPLAYOFF);
        display.clearDisplay();
        display.display();
    }
}

void Animations::drawHexFrame(const char* hexData) {
    if (hexData && strlen(hexData) < sizeof(_currentHexData)) {
        strcpy(_currentHexData, hexData); 
        _newFrameAvailable = true;
    }
}

void Animations::processHexFrameLoop() {
    if (!_oled_active || !_newFrameAvailable) return;
    _newFrameAvailable = false; 

    size_t hexLength = strlen(_currentHexData); 
    
    // Tratamento de limpeza
    if (hexLength == 0) {
        display.clearDisplay();
        display.display();
        return;
    }

    uint8_t buffer[1024] = {0};
    const char* hexDataPtr = _currentHexData;

    auto charToNibble = [](char c) -> uint8_t {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return 0;
    };

    size_t bytesDisponiveis = min(hexLength / 2, (size_t)1024);

    for (size_t i = 0; i < bytesDisponiveis; i++) {
        buffer[i] = (charToNibble(hexDataPtr[i * 2]) << 4) | charToNibble(hexDataPtr[(i * 2) + 1]);
    }

    display.clearDisplay();
    display.drawBitmap(0, 0, buffer, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
    display.display();
}