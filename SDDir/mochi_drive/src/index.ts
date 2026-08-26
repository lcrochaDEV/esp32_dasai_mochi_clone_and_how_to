import express, { Request, Response } from "express";
import 'dotenv/config';
import cors from 'cors';
import rotasMochi from "../src/roteamento/rotas";

const HOST = process.env.SERVER_IP || 'localhost';
const PORT = process.env.SERVER_PORT ? Number(process.env.SERVER_PORT) : 3001;

// Inicialização do App
const app = express();

// 1. Configura os middlewares de tamanho PRIMEIRO
app.use(express.json({ limit: '50mb' }));
app.use(express.urlencoded({ limit: '50mb', extended: true }));

// Middlewares
app.use(express.json());
app.use(cors({
  origin: '*' 
}));

interface IReqRes{
  req: Request,
  res: Response
}

// Rota raíz de teste (padrão correto do Express)
app.get('/', ({ req, res }: IReqRes) => res.send('Página Home - API Mochi Dasai'));

// Integração das rotas do CRUD (Substituindo o connectDataBase pelo roteador do Express)
// Agora, quando acessar http://IP:PORT/home, ele chamará o GET, POST, etc. lá do seu controller
app.use("/api", rotasMochi);

// Inicialização do Servidor
app.listen(PORT, HOST, () => console.log(`🚀 Servidor executando na URL http://${HOST}:${PORT}`));