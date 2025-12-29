#include "SDData.h"

SDData::SDData(const char* filename)
  : filename(filename)
{}

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
    Serial.println("Erro: Arquivo não encontrado no SD.");
    return false;
  }

  DeserializationError error = deserializeJson(doc, file);
    
  if (error) {
      Serial.print("Erro na leitura do JSON: ");
      Serial.println(error.c_str());
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
  StaticJsonDocument<200> doc;
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

// --- DELETAR (DELETE) ---
void SDData::deleteArquivo() {
    // 1. Verificamos se o arquivo existe antes de tentar deletar
    if (!SD.exists(filename)) {
        Serial.printf("Erro: O arquivo '%s' não existe para ser deletado.\n", filename);
        return;
    }

    // 2. Tentamos remover e avisamos o resultado
    if (SD.remove(filename)) {
        Serial.println("Arquivo deletado com sucesso do SD.");
    } else {
        Serial.println("Falha ao tentar deletar o arquivo. Verifique se o SD está bloqueado.");
    }
}

/*EXMPLO DP ARQUIVO .json
  {
    "usuario": "Admin",
    "versao": 1.0,
    "ativo": true
  }
*/
