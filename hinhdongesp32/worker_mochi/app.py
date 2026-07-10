# worker.py

import asyncio
import logging
from motor.motor_asyncio import AsyncIOMotorClient
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.middleware.cors import CORSMiddleware

# Configuração de Logs para monitorizares o streaming no terminal
logging.basicConfig(level=logging.INFO, format="%(asctime)s - %(levelname)s - %(message)s")
logger = logging.getLogger(__name__)

app = FastAPI(title="Mochi Animation Worker")

origins = [
    "http://localhost",
    "http://localhost:8000",
    "*"
]
app.add_middleware(
    CORSMiddleware,
    allow_origins=origins,
    allow_credentials=True,
    allow_methods=["POST", "GET"],
    allow_headers=["*"],
    max_age=3600,
)

# -------------------------------------------------------------------------
# CONFIGURAÇÕES DO BANCO DE DADOS (MONGODB)
# -------------------------------------------------------------------------
# Se usares o MongoDB Atlas ou autenticação, altera a URI abaixo
MONGO_URI = "mongodb://admin:admin@192.168.1.252:27017/?authSource=admin"
DB_NAME = "MochiDB"
COLLECTION_NAME = "mochi_animations"

client = AsyncIOMotorClient(MONGO_URI)
db = client[DB_NAME]
collection = db[COLLECTION_NAME]

# Gerenciador de conexões WebSocket ativas (Neste caso, o teu ESP32 Mochi)
active_connections: set[WebSocket] = set()

# Variável de controle para sabermos qual o tipo de interação atual que deve rodar
current_interaction_type = "connection_status" 

# -------------------------------------------------------------------------
# LOOP PRINCIPAL DO WORKER (REPRODUTOR DE ANIMAÇÕES)
# -------------------------------------------------------------------------
async def animation_streamer_loop():
    """
    Loop assíncrono perpétuo que dita o ritmo (FPS) e envia as strings hexadecimais
    dos frames uma a uma diretamente para a tela física do Mochi.
    """
    global current_interaction_type
    logger.info("Worker de Streaming de Animações iniciado com sucesso!")

    while True:
        # Se não houver nenhum hardware conectado, espera um pouco e pula o ciclo
        if not active_connections:
            await asyncio.sleep(1)
            continue

        try:
            # 1. Procura no MongoDB a animação ativa com base no tipo de interação atual
            animation = await collection.find_one({"type": current_interaction_type})

            if not animation or "frames" not in animation or len(animation["frames"]) == 0:
                logger.warning(f"Nenhuma animação cadastrada para o tipo: '{current_interaction_type}'")
                await asyncio.sleep(2)
                continue

            logger.info(f"Transmitindo a animação: '{animation.get('name')}' ({len(animation['frames'])} frames)")

            # 2. Varre os frames da animação passados um a um
            for frame in animation["frames"]:
                # Se todos os clientes desconectarem no meio da animação, interrompe
                if not active_connections:
                    break

                hex_frame_data = str(frame.get("data", "")).strip()
                
                if hex_frame_data:
                    # Cria uma lista de tarefas para disparar o frame para todos os dispositivos conectados
                    # Enviamos a string hex PURA, poupando a RAM e processamento do ESP32!
                    send_tasks = [
                        connection.send_text(hex_frame_data) 
                        for connection in active_connections
                    ]
                    await asyncio.gather(*send_tasks, return_exceptions=True)

                # 3. CONTROLE DE FRAME RATE (FPS) NO SERVIDOR
                # 0.042 segundos de delay garante uma taxa cravada em aproximadamente ~24 FPS na tela física
                await asyncio.sleep(0.042)

        except Exception as e:
            logger.error(f"Erro crítico no loop do Worker: {e}")
            await asyncio.sleep(1)


@app.get("/")
def methodGet():
   return {"API Mochi está operacional"}

# -------------------------------------------------------------------------
# ENDPOINT WEBSOCKET PARA O ESP32 SE CONECTAR
# -------------------------------------------------------------------------
@app.websocket("/ws/animations")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()
    active_connections.add(websocket)
    logger.info(f"Mochi Robot conectado ao canal WebSocket! Clientes ativos: {len(active_connections)}")
    
    try:
        while True:
            # Mantém a conexão aberta escutando mensagens do Mochi (caso queiras enviar telemetria)
            data = await websocket.receive_text()
            logger.info(f"Mensagem recebida do Mochi: {data}")
            
            # Exemplo: Se o Mochi mandar um comando de texto, podes mudar a animação dinamicamente
            # if "change_state:" in data:
            #     global current_interaction_type
            #     current_interaction_type = data.split(":")[1]

    except WebSocketDisconnect:
        active_connections.remove(websocket)
        logger.warning(f"Mochi Robot desconectou do canal WebSocket. Clientes restantes: {len(active_connections)}")
    except Exception as e:
        if websocket in active_connections:
            active_connections.remove(websocket)
        logger.error(f"Erro na conexão WebSocket: {e}")

# -------------------------------------------------------------------------
# GATILHO DE INICIALIZAÇÃO DO TRABALHO EM SEGUNDO PLANO
# -------------------------------------------------------------------------
@app.on_event("startup")
async def startup_event():
    # Registra o Loop do Worker para rodar de forma assíncrona em background,
    # permitindo que o servidor WebSocket gerencie conexões sem travar.
    asyncio.create_task(animation_streamer_loop())


# Para rodar este arquivo, instala as dependências:
# pip install fastapi uvicorn motor pymongo
#
# E executa no terminal:
# uvicorn worker:app --host 0.0.0.0 --port 8000 --reload