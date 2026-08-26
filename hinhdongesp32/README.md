## Base do Projeto

# 🤖 ESP32 Dasai Mochi Clone & Sistema IoT Embarcado 💾

Este repositório contém o firmware, backend e ferramentas de suporte para a criação de um **clone avançado do robô de painel Dasai Mochi**. O sistema utiliza um microcontrolador **ESP32** para a exibição de expressões faciais e animações num ecrã **OLED (SSD1306 128x64)** ou **TFT (ST7789)**, integrando comunicação via **WebSockets em tempo real**, servidor web assíncrono, persistência em **cartão MicroSD** e um ecossistema **Python/MongoDB**.

## 🌟 Destaques do Projeto

* 🎭 **Animações em Tempo Real:** Exibição de expressões robóticas e *emotes* animados suavemente (~24 FPS).
* 🌐 **Streaming via WebSockets:** Receção dinâmica de quadros hexadecimais a partir de um servidor remoto.
* 🛡️ **Arquitetura de Tripla Redundância:** Alternância automática entre WebSocket, backup em cartão MicroSD e animações na Flash interna.
* 🌐 **Servidor Web Assíncrono:** Painel de controlo HTTP integrado (`ESPAsyncWebServer`) para configuração do dispositivo em tempo real.
* 📡 **Conetividade Avançada:** Wi-Fi com suporte a reconexão automática, *failover* para rede secundária, varredura de redes (WPA2/WPA3) e *scanner* Bluetooth (Classic/BLE).
* ⏰ **Gestão Temporal & Modo Noturno:** Sincronização de relógio e horários programáveis de *Wake-on/Sleep* para desligar automaticamente o ecrã.
* 💻 **Consola CLI Interativa:** Controlo do dispositivo via Monitor Serial de forma totalmente não-bloqueante.

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

| Componente (ESP32) | Pino (ST7789/Display) | Descrição |
| :--- | :--- | :--- | 
| **GND** | GND | Terra |
| **VCC/3.3V** | VCC  | Alimentação (3.3V) |
| **D21** | MOSI (SDA) | Dados SPI |
| **D22** | SCLK (SCK) | Clock SPI |


### 🔌 Esquema de Ligação: **ESP32-C3 Super Mini** para Módulo Cartão SD

Baseado na fiação da sua imagem antiga, converta as conexões para os novos pinos físicos do C3 Super Mini:

| Pino do Módulo SD (Sua Imagem) | Cor do Fio (Imagem) | Novo Pino no ESP32-C3 Super Mini | 
| :--- | :--- | :--- |
| 3V3 | 🔴 Vermelho | 3V3
| CS  | 🟡 Amarelo | G4 (GPIO 7)
| MOSI  | 🟤 Bege / Marrom | G6 (GPIO 6)
| CLK | 🔵 Azul | G5 (GPIO 4)
| MISO  | 🟢 Verde | G3 (GPIO 5)
| GND |⚫ Preto | GND |

### 📺 Onde ligar o Display OLED SSD1306?

Como você vai usar os pinos acima para o cartão SD, o display OLED deve ser ligado obrigatoriamente nos pinos de hardware I2C do ESPC3 Super Mini:

* VCC do OLED ➡️ 3V3 do C3
* GND do OLED ➡️ GND do C3
* SDA do OLED ➡️ G2 (GPIO 8) do C3 (Nota: O LED azul nativo da placa piscará de leve junto com os dados da tela).
* SCL do OLED ➡️ G1 (GPIO 9) do C3

## 🛠️ Hardware Necessário

* **Microcontrolador:** ESP32 C3 Mini (ou ESP32 Standard / ESP8266).
* **Display:** Ecrã OLED SSD1306 128x64 (I2C) ou TFT ST7789 (SPI).
* **Armazenamento:** Leitor de Cartão MicroSD (Barramento SPI).
* **Fonte de Alimentação:** Cabo USB-C ou alimentação externa de 5V.

## 📌 Mapeamento de Pinos (Pinout ESP32 C3 Mini)

```txt
Plaintext       

       LADO ESQUERDO                         LADO DIREITO
     +-----------------+                 +-----------------+
     |  [ ] 3V3        |                 |  [ ] 5V         |
     |  |*| G4  (CS)   |                 |  |*| G3  (MISO) |
     |  |*| G5  (CLK)  |                 |  |*| G2  (SDA)  |
     |  |*| G6  (MOSI) |                 |  |*| G1  (SCL)  |
     |  [ ] G7         |                 |  [ ] G0         |
     |  [ ] TX         |                 |  [ ] RX         |
     |  [ ] GND        |                 |  [ ] GND        |
     +--------+--------+                 +--------+--------+
              |      [ Conector USB-C ]           |
```
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


### Site para criação de imagens OLED
[OLED Display Animation Maker](https://www.oledanimationmaker.com/)


### 🧩 Funcionalidades Detalhadas do Firmware
### 1. Display e Animações (Animations & all_frames.h)
* Processamento Hexadecimal: Descodifica em tempo real cadeias de carateres hexadecimais recebidas via rede e desenha a matriz de pixels diretamente na memória do ecrã OLED.

* Biblioteca Local: Contém um repositório interno de até 772 quadros monocromáticos em arrays C++ para funcionamento totalmente offline.

* Gestão Energética: Método control_oled_power() que liga ou desliga o barramento I2C do ecrã para evitar saturação e poupar energia.

* Quadros de Estado: Exibição imediata de ícones/quadros de erro caso falhe a ligação Wi-Fi (not_wifi), o servidor esteja inativo (noworker) ou ocorra um erro de payload (bugframe).

### 2. Cliente WebSocket em Tempo Real (MochiWebSocketClient)
* Coneta-se a endpoints WebSockets (ws://host:port/ws/animations) com suporte a heartbeat e reconexão automática.

* Temporizador de Contingência (Data Timeout): Monitoriza a chegada constante de pacotes. Se o fluxo de dados for interrompido por mais de 5 segundos, ativa suavemente a animação de reserva (local ou do Cartão SD) sem causar tremeluzir no ecrã.

### 3. Servidor Web Assíncrono (servidorweb.h)
* Executa um servidor HTTP na porta 80 baseado na biblioteca ESPAsyncWebServer.

* Serviço de Ficheiros do SD: Serve a interface de utilizador HTML/JS armazenada na pasta /www/ do Cartão SD.

* Endpoints REST API:

  * GET /scan: Executa a varredura e devolve um JSON com as redes Wi-Fi disponíveis, intensidade de sinal e tipo de encriptação (incluindo transição WPA3).

  * GET /status: Retorna o estado físico dos pinos GPIO e o estado da ligação MQTT.

  * POST /config_mqtt: Regista e ativa novos perfis de brokers MQTT com suporte a geração de UUID único.

  * PATCH /set_active_mqtt & DELETE /delete_mqtt: Altera ou elimina perfis de ligação MQTT armazenados no sistema de ficheiros.

  * GET /datetime: Fornece a data e hora atuais sincronizadas pelo sistema.

### 4. Gestão de Armazenamento MicroSD (SDData)
* Leitura e escrita de ficheiros de configuração em formato JSON (/data/config.json) recorrendo à biblioteca ArduinoJson.

* Leitor de Contingência (readLocalFallbackFrame): Capaz de ler quadros em formato JSON idêntico ao MongoDB diretamente do caminho /frames/wifi_frame/fallback.json no cartão SD.

* Armazenamento e recuperação de parâmetros de utilizador e preferências do modo de descanso (Wake-on/Sleep).

5. Consola de Comandos Serial CLI (Console)
Interface interativa acessível via Monitor Serial (baud rate padrão do projeto):

* HELP: Exibe o menu de comandos.

* SHOWDATA: Imprime o JSON de configurações formatado na consola.

* DELETEDATA: Remove o ficheiro de configuração do cartão SD.

* SCANWF: Despoleta uma varredura de redes sem fios e lista no terminal.

* DISPLAYON / DISPLAYOFF: Liga ou desliga manualmente o ecrã OLED.

* ANIMACAO: Força a execução em loop das animações locais.

### 6. Diagnóstico de Hardware (PhysicalAccessControl & AccessControl)
* Identificação automática do modelo de chip (ESP32 / ESP8266).

* Cálculo da memória RAM total, RAM livre (Free Heap), tamanho da memória Flash e espaço ocupado pelo programa (Sketch Size).

### 🐍 Backend Python & Ferramentas de Automação
O repositório inclui utilitários em Python localizados nos scripts auxiliares:

### 1. Worker Python (FastAPI + MongoDB Change Streams):

  * Monitoriza alterações no banco de dados MongoDB através de Change Streams.

  * Quando uma nova animação é registada ou alterada no banco de dados, o Worker envia imediatamente a sequência de frames hexadecimais via WebSocket para todos os robôs Mochi conectados.

  * Gere o envio programado de animações com base no horário do dia (modo diurno vs. modo noturno/dormir).

### 2. Gerador Automático de Quadros (Criador de frame):

  * Script de automação baseado em Selenium WebDriver.

  * Processa ficheiros GIF/imagens, interage com a ferramenta FileToCArray e gera automaticamente os ficheiros cabeçalho .h com numeração sequencial de variáveis para integração em C++.

### 🚀 Como Executar
### 1. Preparação do Cartão SD
  1. Formate o cartão MicroSD em FAT32.

  2. Crie a pasta /www/ na raiz do cartão e insira os ficheiros de interface Web (incluindo index.html e mqtt.html).

  3. (Opcional) Adicione o ficheiro de reserva em /frames/wifi_frame/fallback.json para animações offline do SD.

### 2. Compilação do Firmware
  1. Abra o projeto no VS Code com PlatformIO ou no Arduino IDE.

  2. Certifique-se de ter instaladas as seguintes bibliotecas:

      * Adafruit_GFX e Adafruit_SSD1306

      * ArduinoJson

      * WebSockets (por Markus Sattler)

      * ESPAsyncWebServer e AsyncTCP

  3. Selecione a sua placa (ex: ESP32-C3 Dev Module) e faça o upload do código.

### 3. Execução do Backend (Opcional)
  1. Certifique-se de que o MongoDB está configurado como Replica Set (necessário para Change Streams).

  2. Execute o serviço Worker em Python para transmitir as animações via WebSocket para o IP do seu ESP32.


### 🔍 Resumo da Análise Técnica e Boas Práticas

| Componente | Estado de Implementação | Vantagem Técnica |
| :--- | :--- | :--- 
| Renderização OLED | Concretizada (Animations.cpp) | Controlo otimizado com prevenção de comandos repetidos no barramento I2C. 
| Comunicação WS | Concretizada (MochiWebSocketClient.cpp) | Fallback suave para modo local se a rede falhar por mais de 5s. 
| Servidor Web | AssíncronoConcretizada (servidorweb.h) | Execução paralela sem interferir no tempo de renderização dos frames (~24 FPS).
| Armazenamento | JSONConcretizada (SDData.cpp) | Suporte a estruturas complexas e leitura direta de frames do SD. 
| Consola Serial CLI | Concretizada (Console.cpp) | Leitura contínua bufferizada de carateres sem bloquear o loop principal.

### 🤝 Créditos e Referências
Este projeto é uma adaptação e evolução do conceito original:

 * Projeto Base / Inspiração Original: huykhoong/esp32_dasai_mochi_clone_and_how_to

 * Conversão de Recursos Gráficos: Ferramenta FileToCArray.

 * Autor do Ecossistema IoT & Backend: @khoi2mai / Projeto Dasai Mochi Clone.

