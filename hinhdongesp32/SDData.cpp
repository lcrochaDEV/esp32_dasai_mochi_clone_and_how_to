#include "SDData.h"

SDData::SDData(const char* filename)
  : filename(filename)
{}

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
}

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

void SDData::listDir(const char * dirname, uint8_t levels) {
     Serial.println("\n\------Dados do cartão SD-------------");
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
            Serial.printf(" FILE: %s SIZE %d\n", file.name(), file.size());
        }
        file = root.openNextFile();
    }
    /*EXEMPLOS
      // Se você quiser listar tudo o que está na raiz do cartão SD, use:
      SDData_exec.listDir("/", 1); // O segundo parâmetro é a profundidade (0 = apenas a pasta atual, 1 = entra em uma subpasta)
    */
}
/*

// --- GRAVAR (CREATE) ---
void gravarJSON() {
  File file = LittleFS.open(filename, "w");
  if (!file) return;

  StaticJsonDocument<200> doc;
  doc["usuario"] = "Admin";
  doc["versao"] = 1.0;
  doc["ativo"] = true;

  if (serializeJson(doc, file) == 0) {
    Serial.println("Falha ao gravar arquivo");
  }
  file.close();
}

// --- LER (READ) ---
void readJSON() {
  File file = LittleFS.open(filename, "r");
  if (!file) return;

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, file);
  
  if (error) {
    Serial.println("Erro na leitura");
  } else {
    const char* user = doc["usuario"];
    Serial.println(user);
  }
  file.close();
}

// --- EDITAR (UPDATE) ---
void editJSON() {
  StaticJsonDocument<200> doc;
  File file = LittleFS.open(filename, "r");
  deserializeJson(doc, file);
  file.close();

  // Modifica o valor desejado
  doc["versao"] = 2.0;

  // Salva novamente
  file = LittleFS.open(filename, "w");
  serializeJson(doc, file);
  file.close();
}

// --- DELETAR (DELETE) ---
void deleteArquivo() {
  if (LittleFS.remove(filename)) {
    Serial.println("Arquivo deletado");
  }
}
*/