# 🍡 Mochi Drive API

Uma API RESTful robusta, performática e tipada desenvolvida em **Node.js**, **Express**, **TypeScript** e **MongoDB** para gerenciar as animações e dados do ecossistema Mochi (perfeito para integração com ESP32 e clones do Dasai Mochi).

Este projeto foi reestruturado para seguir as melhores práticas de mercado, garantindo tratamento seguro de IDs do MongoDB, roteamento limpo e isolamento de variáveis de ambiente.

---

## 🚀 Tecnologias Utilizadas

- **Node.js** & **TypeScript** (Tipagem estática e segurança em tempo de desenvolvimento)
- **Express** (Gerenciamento de rotas e requisições HTTP)
- **MongoDB Native Driver** (Conexão nativa de alta performance com o banco de dados)
- **Dotenv** (Gerenciamento seguro de credenciais via variáveis de ambiente)

---

## 📋 Arquitetura e Fluxo do Projeto

O projeto segue uma arquitetura separada por responsabilidades para facilitar a manutenção e escalabilidade:

```text
src/
├── Controladores/
│   └── ClassRequest.ts    # Centraliza a lógica do CRUD e validações do MongoDB
├── Roteamento/
│   └── rotas.ts           # Definição e mapeamento dos endpoints HTTP
├── conect_data
|   └──conectData.ts       # Inicialização e exportação do cliente MongoDB
└── index.ts               # Ponto de entrada do servidor Express
```

---

## 🛠️ Passo a Passo da Configuração do Ambiente

### 1. Configuração do Banco de Dados (MongoDB)
Para garantir que a API encontre os registros históricos com sucesso, a coleção foi padronizada no terminal (`mongosh`) utilizando os seguintes passos:

1. Acesse o banco de dados principal:
   ```bash
   use MochiDB
   ```
2. Renomeie a coleção antiga (`mochi_animations`) para o novo padrão estruturado no código (`MochiDataBase`):
   ```bash
   db.mochi_animations.renameCollection("MochiDataBase")
   ```

### 2. Variáveis de Ambiente (`.env`)
Crie um arquivo `.env` na raiz do seu projeto e insira a string de conexão do seu MongoDB de forma segura:
```env
PORT=3001
CONECT_DATABASE=mongodb://localhost:27017/MochiDB
```

---

## 💻 Estrutura dos Arquivos Principais

### Conexão Segura com o Banco (`src/conect_data/conectData.ts`)
Este arquivo gerencia o ciclo de vida da conexão, evitando o uso de *Top-Level Await* que pode travar a inicialização do app, e tipa explicitamente a coleção para manter o autocompletar do driver.

```typescript
import { MongoClient, Collection } from "mongodb";

const DATABASE_URL = process.env.CONECT_DATABASE;

if (!DATABASE_URL) throw new Error("A variável de ambiente CONECT_DATABASE não foi definida!");

const client = new MongoClient(DATABASE_URL);
const db = client.db("MochiDB");

// Tipagem explícita para o ecossistema Mochi
const connectDataBase: Collection<any> = db.collection("MochiDataBase");

client.connect()
  .then(() => {
    console.log("🚀 Conectado ao MongoDB com sucesso (MochiDataBase)!");
  })
  .catch((erro) => {
    console.error("❌ Erro crítico ao conectar no MongoDB:", erro);
    process.exit(1); 
  });

export default connectDataBase;
```

### Roteamento Inteligente (`src/roteamento/rotas.ts`)
As rotas foram separadas para evitar sobreposição ou falha na resolução dinâmica de parâmetros. O Express analisa se o ID foi fornecido de forma nativa.

```typescript
import { Router } from 'express';
import Rotas from '../Controladores/ClassRequest';

const router = Router();

// Buscar todos os registros
router.get("/", Rotas.methodGet);

// Buscar registro específico por ID
router.get("/:id", Rotas.methodGet);

// Demais rotas do CRUD
router.post("/", Rotas.methodPost);
router.patch("/:id", Rotas.methodPatch);
router.delete("/:id", Rotas.methodDelete);

export default router;
```

### Controlador Blindado (`src/Controladores/ClassRequest.ts`)
O método `GET` foi desenhado para ser dinâmico e seguro, validando se o ID fornecido possui o formato exato de 24 caracteres hexadecimais do MongoDB (`ObjectId`), prevenindo quebras do servidor em tempo de execução:

```typescript
import { Request, Response } from 'express';
import { ObjectId } from 'mongodb';
import connectDataBase from '../conectData';

class ClassRequest {
  async methodGet(req: Request, res: Response): Promise<void> {
    try {
      const id = req.params.id as string;

      // Se não houver ID na URL, retorna todos os dados da coleção
      if (!id) {
        const todosOsDados = await connectDataBase.find({}).toArray();
        res.status(200).json(todosOsDados);
        return;
      }

      // Validação estrita de formato do ObjectId do MongoDB
      if (!ObjectId.isValid(id)) {
        res.status(400).json({ error: "ID em formato inválido. Deve ter 24 caracteres hexadecimais." });
        return;
      }

      const registro = await connectDataBase.findOne({ _id: new ObjectId(id) });

      if (!registro) {
        res.status(404).json({ error: "Registro não encontrado na base de dados Mochi." });
        return;
      }

      res.status(200).json(registro);
    } catch (error) {
      console.error("Erro no método GET:", error);
      res.status(500).json({ error: "Erro interno do servidor ao processar a requisição." });
    }
  }
  
  // Os métodos methodPost, methodPatch e methodDelete seguem a mesma lógica segura...
}

export default new ClassRequest();
```

---

## 🌍 Guia de Uso dos Endpoints (API Reference)

Considerando o prefixo definido no arquivo principal (ex: `app.use("/api", rotas)`):

### 1. Listar Todos os Dados
Retorna um array com todas as animações/configurações gravadas.
*   **Método:** `GET`
*   **URL:** `http://localhost:3001/api`
*   **Resposta de Sucesso (200 OK):** `[ { "_id": "...", "nome": "Mochi Feliz", ... } ]`

### 2. Buscar por ID Específico
Busca um único documento através de seu identificador exclusivo.
*   **Método:** `GET`
*   **URL:** `http://localhost:3001/api/6a55069d0f022966c2dbbe27`
*   **Resposta (200 OK):** `{ "_id": "6a55069d0f022966c2dbbe27", "nome": "Mochi Feliz" }`
*   **Resposta ID Inválido (400 Bad Request):** `{ "error": "ID em formato inválido..." }`
*   **Resposta Não Encontrado (404 Not Found):** `{ "error": "Registro não encontrado..." }`

### 3. Criar Novo Registro
Insere novos dados ou sequências de animações na base.
*   **Método:** `POST`
*   **URL:** `http://localhost:3001/api`
*   **Headers:** `Content-Type: application/json`
*   **Body (JSON):**
    ```json
    {
      "nome": "Animação Piscando",
      "frames": [1, 2, 3, 4],
      "velocidade": 150
    }
    ```

---

## 🛡️ Benefícios da Nova Implementação

1. **Anti-Crash:** A API nunca derrubará o processo por erro de conversão de string para `ObjectId`.
2. **Performance:** Conexão nativa reutilizável mantendo o pool de conexões otimizado pelo driver do MongoDB.
3. **Previsibilidade:** Sem dados duplicados ou coleções fantasmas em segundo plano.