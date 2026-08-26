import { MongoClient, Collection } from "mongodb";

const DATABASE_URL = process.env.CONECT_DATABASE;

if (!DATABASE_URL) throw new Error("A variável de ambiente CONECT_DATABASE não foi definida!");

const client = new MongoClient(DATABASE_URL);

// Criamos a conexão com o banco
const db = client.db("MochiDB");

/**
 * Tipamos explicitamente a coleção para manter o autocompletar do MongoDB.
 * Substitua o 'any' dentro de Collection<any> por uma Interface do seu documento futuramente, 
 * por exemplo: Collection<IUsuario>
 */
const connectDataBase: Collection<any> = db.collection("MochiDataBase");

// Iniciamos a conexão em background de forma segura
client.connect()
  .then(() => {
    console.log("🚀 Conectado ao MongoDB com sucesso (MochiDataBase)!");
  })
  .catch((erro) => {
    console.error("❌ Erro crítico ao conectar no MongoDB:", erro);
    // Em produção, se o banco não conecta, geralmente queremos que o processo feche
    process.exit(1); 
  });

export default connectDataBase;