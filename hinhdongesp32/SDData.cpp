#include "SDData.h"

SDData::SDData(const char* filename) {
    // Se passar nullptr, assume o caminho padrão do projeto
    this->filename = (filename != nullptr) ? filename : "/data/config.json";
}

// Starta o SD
void SDData::sdbegin() {
  if(!SD.begin()){
    Serial.println("Falha ao montar o Cartão SD!");
    return;
  }
  Serial.println("Cartao SD inicializado com sucesso!");
    // Opcional: imprime informacoes basicas sobre o cartao
    uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
      Serial.println("Nenhum cartao SD encontrado ou tipo desconhecido.");
    } else {
      Serial.print("Tipo de cartao SD: ");
      if (cardType == CARD_MMC) Serial.println("MMC");
      else if (cardType == CARD_SD) Serial.println("SDSC");
      else if (cardType == CARD_SDHC) Serial.println("SDHC");
      else Serial.println("DESCONHECIDO");

      uint64_t cardSize = SD.cardSize() / (1024 * 1024);
      Serial.printf("Tamanho do cartao: %lluMB\n", cardSize);
    }
    // Garante de forma automática que o diretório padrão exista no SD
    createFolder("/data");
}

// Cria diretórios no cartão SD
bool SDData::createFolder(const char * path) {
    if (SD.exists(path)) {
        Serial.printf("A pasta '%s' já existe.\n", path);
        return true;
    }
    if (SD.mkdir(path)) {
        Serial.printf("Pasta '%s' criada com sucesso!\n", path);
        return true;
    } else {
        Serial.printf("Falha ao criar a pasta '%s'.\n", path);
        return false;
    }
    /*EXEMPLOS
      // SDData_exec.createFolder("/config"); // Criando uma pasta chamada 'config'
      // SDData_exec.createFolder("/assets/icons"); // Criando uma subpasta (mkdir cria todo o caminho se necessário)
    */
}

// Lista diretórios e arquivos no cartão SD
void SDData::listDir(const char * dirname, uint8_t levels) {
    Serial.println("\n------Dados do cartão SD-------------");
    Serial.printf("Listando diretório: %s\n", dirname);

    File root = SD.open(dirname);
    if (!root) {
        Serial.println("Falha ao abrir o diretório");
        return;
    }
    if (!root.isDirectory()) {
        Serial.println("O caminho fornecido não é um diretório");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            Serial.printf("  DIR : %s", file.name());
            if (levels) {
                // Chama recursivamente para listar subpastas
                listDir(file.path(), levels - 1);
            }
        } else {
            Serial.printf(" FILE: %s SIZE %d\n\n", file.name(), file.size());
        }
        file = root.openNextFile();
    }
    /*EXEMPLOS
      // Se você quiser listar tudo o que está na raiz do cartão SD, use:
      SDData_exec.listDir("/", 1); // O segundo parâmetro é a profundidade (0 = apenas a pasta atual, 1 = entra em uma subpasta)
    */
}

// --- GRAVAR (CREATE) ---
void SDData::createJSON(JsonDocument& doc) {
  File file = SD.open(filename, "w");
  if (!file) return;

  if (serializeJson(doc, file) == 0) {
    Serial.println("Falha ao escrever JSON no SD");
  } else {
    Serial.println("Falha ao gravar arquivo");
  }
  file.close();

  /*EXEMPLO
    JsonDocument doc; 
    doc["ssid"] = "MinhaRede";
    doc["senha"] = "12345678";
    doc["ip_estatico"] = true;

    meuSD.gravarJSON(doc); // A classe salva sem questionar os campos

    JsonDocument doc;
    doc["temp"] = 25.4;
    doc["umid"] = 60;
    doc["sensor_id"] = "DHT22_Externo";

    meuSD.gravarJSON(doc); // O mesmo método agora salvou dados diferentes
  */
}

// --- LER (READ) ---
bool SDData::readJSON(JsonDocument& doc) {
  File file = SD.open(filename, "r");
    if (!file) {
        Serial.println("⚠️ Arquivo não encontrado no SD.");
        return false;
    }

    DeserializationError error = deserializeJson(doc, file);
    file.close();
      
    if (error) {
        Serial.printf("❌ Erro na leitura do JSON: %s\n", error.c_str());
        return false;
    }
    return true; // Sucesso!
    /*EXEMPLO
      //TODOS
      void carregarConfiguracoes() {
        JsonDocument doc;
        if (meuSD.readJSON(doc)) {
            // Percorre todos os pares Chave:Valor dentro do JSON
            JsonObject obj = doc.as<JsonObject>();
            for (JsonPair p : obj) {
                Serial.print("Chave encontrada: ");
                Serial.print(p.key().c_str());
                Serial.print(" | Valor: ");
                Serial.println(p.value().as<const char*>()); // Converte o valor para string
            }
        }
      }
      //COM CHAVES
      void carregarConfiguracoes() {
          JsonDocument config;
          if (meuSD.readJSON(config)) {
              // Se eu souber o que tem lá, uso direto:
              const char* nome = config["usuario"] | "Padrao"; // O '|' define um valor padrão se não existir
              int nivel = config["nivel_acesso"] | 0;
              
              Serial.printf("Bem-vindo %s, seu nível é %d\n", nome, nivel);
          }
      }
    */
}

// --- EDITAR (UPDATE) ---
void SDData::updateJSON(const char* chave, const char* novoValor) {
  JsonDocument doc;
  File file = SD.open(filename, "r");
  if (file) { 
    deserializeJson(doc, file); 
    file.close(); 
  }
  // Modifica o valor desejado
  doc[chave] = novoValor;

  // Salva novamente
  file = SD.open(filename, "w");
  if (file) {
    if (serializeJson(doc, file) == 0) {
        Serial.println("Falha ao atualizar arquivo no SD");
    }
    file.close();
    Serial.printf("Campo '%s' atualizado com sucesso!\n", chave);
  } else {
      Serial.println("Erro ao abrir arquivo para salvar atualizacao");
  }

    /*EXEMPLO
      // Atualiza a versão
      meuSD.updateJSON("versao", "3.0");

      // Atualiza o usuário
      meuSD.updateJSON("usuario", "Joao_Silva");

      // Adiciona um campo que nem existia antes!
      meuSD.updateJSON("ultimo_acesso", "2025-12-27");
    */
}

// --- LISTAR/IMPRIMIR DADOS FORMATADOS NO CONSOLE ---
void SDData::printJSON() {
    if (!SD.exists(filename)) {
        Serial.printf("[SDData] Erro: O arquivo '%s' não existe para análise.\n", filename);
        return;
    }

    File file = SD.open(filename, FILE_READ);
    if (!file) {
        Serial.printf("[SDData] Erro ao abrir o arquivo '%s' para leitura.\n", filename);
        return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.printf("[SDData] Erro ao processar o JSON para o console: %s\n", error.c_str());
        return;
    }

    Serial.println("\n=============================================");
    Serial.printf("🔍 ANÁLISE DE DADOS DO ARQUIVO: %s\n", filename);
    Serial.println("=============================================");
    
    // Imprime o JSON de forma "bonita" e identada no Monitor Serial
    serializeJsonPretty(doc, Serial);
    
    Serial.println("\n=============================================\n");
}

// --- ESCRITA / EDIÇÃO (Salva a estrutura com o objeto aninhado) ---
bool SDData::saveWakeonSettings(const String& name, const String& wakeon, const String& sleep) {
    File file = SD.open(filename, FILE_WRITE); // FILE_WRITE sobrescreve limpando o arquivo antigo
    if (!file) {
        Serial.println("[SDData] Erro ao abrir arquivo para escrita.");
        return false;
    }

    JsonDocument doc;
    doc["id"] = "1";
    doc["name"] = name;

    // Cria o objeto aninhado 'wakeonMode' conforme solicitado
    JsonObject wakeonMode = doc["wakeonMode"].to<JsonObject>();
    wakeonMode["wakeon"] = wakeon;
    wakeonMode["sleep"] = sleep;

    // Serializa direto no arquivo de forma eficiente
    if (serializeJson(doc, file) == 0) {
        Serial.println("[SDData] Falha ao serializar JSON.");
        file.close();
        return false;
    }

    file.close();
    Serial.println("[SDData] Configurações de wakeonMode salvas com sucesso!");
    return true;
}

// --- LEITURA / RECUPERAÇÃO (Extrai os dados de dentro de wakeonMode) ---
bool SDData::loadWakeonSettings(String& outName, String& outWakeon, String& outSleep) {
    if (!SD.exists(filename)) {
        Serial.println("[SDData] Arquivo de configuração não existe.");
        return false;
    }

    File file = SD.open(filename, FILE_READ);
    if (!file) return false;

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.printf("[SDData] Erro ao processar JSON: %s\n", error.c_str());
        return false;
    }

    // Recupera o nome principal
    outName = doc["name"] | "Mochi Robot";

    // Acessa de forma segura o objeto interno wakeonMode
    JsonObject wakeonMode = doc["wakeonMode"];
    outWakeon = wakeonMode["wakeon"] | "07:00"; // Fallback caso a chave não exista
    outSleep  = wakeonMode["sleep"]  | "22:00";

    return true;
}

// --- DELETAR (DELETE) ---
void SDData::deleteArquivo() {
    if (SD.exists(filename)) {
        if (SD.remove(filename)) {
            Serial.printf("[SDData] Arquivo '%s' deletado com sucesso do SD.\n", filename);
        } else {
            Serial.printf("[SDData] Falha crítica ao tentar deletar '%s'.\n", filename);
        }
    } else {
        Serial.printf("[SDData] Operação cancelada: '%s' não existe.\n", filename);
    }
}


// Metodo para ler uma animação local de contingência (ex: /data/fallback.json) gravada no mesmo formato do MongoDB.
// Implemente este método no final do seu SDData.cpp

bool SDData::readLocalFallbackFrame(int index, String& outHexData) {
    // Abre o arquivo no caminho exato solicitado
    File file = SD.open("/frames/wifi_frame/fallback.json", "r");
    if (!file) {
        Serial.println("[SDData] Erro: /frames/wifi_frame/fallback.json nao encontrado!");
        return false;
    }

    // Aloca um tamanho seguro para o parser processar o documento do arquivo
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close(); // Fecha o arquivo assim que ler para liberar recursos do barramento SPI

    if (error) {
        Serial.printf("[SDData] Erro ao processar o JSON de Fallback: %s\n", error.c_str());
        return false;
    }

    // Verifica se a estrutura esperada do MongoDB (que clonamos para o SD) existe
    if (!doc["frames"].is<JsonArray>()) {
        Serial.println("[SDData] Erro: Chave 'frames' invalida no arquivo fallback.");
        return false;
    }

    JsonArray framesArray = doc["frames"].as<JsonArray>();
    
    // Procura o frame que bate com o 'frame_index' solicitado pelo loop
    for (JsonObject frame : framesArray) {
        int currentIdx = frame["frame_index"] | -1;
        if (currentIdx == index) {
            const char* hexStr = frame["data"] | nullptr;
            if (hexStr) {
                outHexData = String(hexStr);
                return true;
            }
        }
    }

    // Se chegou até aqui, o índice solicitado não foi achado no JSON (ex: fim da animação)
    return false; 
}