### Base do Projeto


#### Destaques do projeto:
- Exibição de emotes animados de Mochi em tela OLED
- Utilização do gif2cpp para converter GIFs em arrays C++ leves
- Baseado em ESP32
- Executa animações suaves a 15 FPS em displays 0LED
- Ideal para expressões robóticas, assistentes inteligentes ou pura fofura

#### Integação de Funcionalidades adicionais:
- sistema de Horas
- Time de desligamneto da Tela
- Conexão EIFI
- Comandos via console
- Servidor Web


# 🎌 ESP32-DASAI-MOCHI-CLONE & HOW-TO 💾

Este repositório contém o código e as instruções necessárias para replicar o projeto **"Dasai Mochi"** de **huykhoong** (um clone do clássico "Dasai Mochi"). O projeto utiliza um **ESP32 C3 Mini** para exibir vídeo em um display ST7789, com os dados de vídeo armazenados em um cartão SD.

## 🌟 Visão Geral do Projeto

Este projeto é uma demonstração de como utilizar um **ESP32** para reprodução de vídeo simples e eficiente em um display TFT de baixo custo. O vídeo é convertido em um formato otimizado e reproduzido quadro a quadro a partir de um cartão MicroSD.

### 🎥 O que é "Dasai Mochi"?

"Dasai Mochi" é uma homenagem ou clone do famoso vídeo "Bad Apple!!" executado em hardware limitado. A otimização se concentra em altas taxas de quadros e cores limitadas para garantir uma execução suave no ESP32.

## 🛠️ Hardware Necessário

Para replicar este projeto, você precisará dos seguintes componentes:

* **Microcontrolador:** ESP32 C3 Mini (qualquer variante, como ESP32 Dev Module, Wemos D1 R32, etc.)
* **Display:** Módulo Display OLED com SSD1360 , de 0,96 polegadas ou 1,3 polegadas.
##### Armazenamento
* **Armazenamento:** Módulo Leitor de Cartão MicroSD
* **Cartão SD:** Um cartão MicroSD formatado (FAT32) para armazenar os dados do vídeo.
##### Touch
* ***TTP223** Sensor de toque (caso deseje funcionalidade de toque adicional)
##### Audio

* **Conectores:** Jumpers (Fio Du Pont)
* ***MAX98357*** Se você não estiver usando um alto-falante com buzzer para reproduzir música, prepare uma combinação do circuito amplificador MAX98357 com um mini alto-falante de 8 ohms.



## ⚙️ Configuração de Hardware e Fiação (Wiring)

A fiação exata pode variar dependendo do seu módulo display e leitor de cartão SD. A seguir, está uma fiação comum para o display ST7789 e o módulo SD (SPI).

| Componente (ESP32) | Pino (ST7789/Display) | Pino (Leitor SD) | Descrição |
| :--- | :--- | :--- | :--- |
| **GND** | GND | GND | Terra |
| **VCC/3.3V** | VCC | VCC | Alimentação (3.3V) |
| **D21** | MOSI (SDA) | MOSI | Dados SPI |
| **D22** | SCLK (SCK) | SCK | Clock SPI |


## 🚀 Como Executar

1.  Insira o cartão MicroSD (com o arquivo de dados de vídeo na raiz) no leitor.
2.  Conecte o ESP32 ao computador (ou fonte de alimentação).
3.  O ESP32 deve inicializar, montar o sistema de arquivos do SD, localizar o arquivo de vídeo e iniciar a reprodução no display ST7789.

## 🤝 Créditos

Este projeto é um clone/adaptação.

* **Original/Inspiração:** [huykhoong/esp32_dasai_mochi_clone_and_how_to](https://github.com/huykhoong/esp32_dasai_mochi_clone_and_how_to)
* **Bibliotecas:** TFT\_eSPI (pela excelente performance gráfica) e a biblioteca SD.

## 📄 Licença

Este projeto é de código aberto e está licenciado sob a Licença **MIT** (ou outra licença que o autor original tenha especificado).