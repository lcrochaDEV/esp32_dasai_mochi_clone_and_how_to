[PORTUGUêS]

# 🌐 Smart Screen Controller (ESP32 / ESP8266)

Este projeto consiste em um firmware robusto para controle temporizado de tela, integrando recursos de conectividade via Web Server, sincronização de tempo real via NTP e persistência de dados em Cartão SD utilizando formato JSON.

## 🚀 Funcionalidades Principal

### ⏲️ Gestão de Tela e Tempo

* **Controle de Ciclo:** Liga e desliga a tela automaticamente com base em tempos pré-definidos.
* **Relógio NTP:** Sincronização automática de data e hora via internet para garantir precisão no agendamento das tarefas.

### 📶 Conectividade e Rede

* **Wi-Fi Scanner:** Varredura em tempo real de redes Wi-Fi próximas para facilitar a conexão.
* **Access Point (AP):** Caso não encontre uma rede conhecida, o dispositivo cria sua própria rede Wi-Fi para configuração inicial.
* **Web Server:** Interface intuitiva hospedada no dispositivo para ajustes de parâmetros sem necessidade de recompilar o código.

### 🛠️ Configuração e Armazenamento

* **Console Serial:** Interface de linha de comando para monitoramento e configurações avançadas via USB.
* **Leitor de Cartão SD:** Suporte a hardware para leitura e escrita de logs ou arquivos de sistema.
* **Persistência JSON:** Todas as configurações são salvas em arquivos `.json`, permitindo fácil edição e leitura estruturada de dados.

---

## 🛠️ Tecnologias Utilizadas

* **C++ (Arduino Framework)**
* **ArduinoJson:** Para manipulação de arquivos de configuração.
* **ESPAsyncWebServer:** Para a interface de usuário fluida.
* **NTPClient:** Sincronização de horário.
* **SPI/SD Library:** Comunicação com o módulo de cartão SD.

---

## 📋 Como utilizar

1. **Conexão Inicial:** Ao ligar pela primeira vez, o dispositivo iniciará em modo **Access Point**. Conecte-se à rede gerada por ele.
2. **Configuração Web:** Acesse o endereço IP padrão (geralmente `192.168.4.1`) no seu navegador para configurar o Wi-Fi local e os tempos de tela.
3. **Monitoramento:** Utilize o Serial Monitor (baud rate 115200) para visualizar o status do sistema e o log das operações.
4. **Cartão SD:** Certifique-se de que o cartão SD esteja formatado em FAT32 para que as configurações JSON sejam lidas corretamente.

---

## 📁 Estrutura de Arquivos (SD Card)

O sistema busca e salva as configurações na seguinte estrutura:

```text
/
├── config.json      # Parâmetros de rede e temporizadores
├── logs.txt         # Histórico de atividades (opcional)
└── www/             # Arquivos da página Web (HTML/CSS)

```

---

## 🤝 Contribuição

1. Faça um **Fork** do projeto.
2. Crie uma **Branch** para sua feature (`git checkout -b feature/nova-feature`).
3. Dê um **Commit** nas suas alterações (`git commit -m 'Adicionando nova feature'`).
4. Dê um **Push** na sua Branch (`git push origin feature/nova-feature`).
5. Abra um **Pull Request**.

---

**Gostaria que eu detalhasse a estrutura do arquivo `config.json` ou que criasse um exemplo do código para a leitura do SD?**