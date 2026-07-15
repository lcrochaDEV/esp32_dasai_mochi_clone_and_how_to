import asyncio
import logging
from motor.motor_asyncio import AsyncIOMotorClient
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.middleware.cors import CORSMiddleware
from fastapi import FastAPI, Query
# Configuração de Logs
logging.basicConfig(level=logging.INFO, format="%(asctime)s - %(levelname)s - %(message)s")
logger = logging.getLogger(__name__)

app = FastAPI(title="Mochi Animation Worker v2.1")

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
MONGO_URI = "mongodb://admin:admin@192.168.1.252:27017/?authSource=admin"
DB_NAME = "MochiDB"
COLLECTION_NAME = "mochi_animations"

client = AsyncIOMotorClient(MONGO_URI)
db = client[DB_NAME]
collection = db[COLLECTION_NAME]

# Gerenciador de conexões WebSocket ativas
active_connections: set[WebSocket] = set()

# Categoria padrão do sistema. Vai rodar aleatoriamente entre as cadastradas com este type.
current_interaction_type = "animation" 

# -------------------------------------------------------------------------
# NOVO ENDPOINT PARA MUDAR A CATEGORIA DINAMICAMENTE
# -------------------------------------------------------------------------
@app.post("/set-category")
async def set_category(category: str = Query(..., description="O nome do 'type' da animação desejada")):
    """
    Endpoint HTTP para alterar a categoria atual de animações em tempo real.
    Exemplo de chamada: POST http://localhost:8000/set-category?category=happy
    """
    global current_interaction_type
    
    # Validação simples: checa se existe pelo menos uma animação com esse tipo no banco
    exists = await collection.find_one({"type": category})
    if not exists:
        raise HTTPException(
            status_code=404, 
            detail=f"Categoria '{category}' não encontrada ou não possui nenhuma animação cadastrada."
        )
    
    # Atualiza a variável global sem parar o Worker
    old_category = current_interaction_type
    current_interaction_type = category
    
    logger.info(f"Categoria alterada via API: '{old_category}' -> '{current_interaction_type}'")
    return {
        "status": "sucesso",
        "categoria_anterior": old_category,
        "nova_categoria": current_interaction_type,
        "mensagem": "A troca ocorrerá assim que a animação atual terminar."
    }

# Endpoint auxiliar para você consultar qual categoria está ativa no momento
@app.get("/current-category")
def get_current_category():
    global current_interaction_type
    return {"current_interaction_type": current_interaction_type}

# -------------------------------------------------------------------------
# LOOP PRINCIPAL DO WORKER (REPRODUTOR ALEATÓRIO)1
# -------------------------------------------------------------------------
async def animation_streamer_loop():
    """
    Loop perpétuo que busca animações aleatórias baseadas na categoria (type) atual
    e transmite seus frames para o hardware conectado.
    """
    global current_interaction_type
    logger.info("Worker de Streaming de Animações Aleatórias iniciado!")

    while True:
        # Se não houver nenhum hardware conectado, espera um pouco e pula o ciclo
        if not active_connections:
            await asyncio.sleep(1)
            continue

        try:
            # 1. Pipeline de Agregação para buscar 1 animação aleatória filtrada pelo 'type' atual
            pipeline = [
                {"$match": {"type": current_interaction_type}},
                {"$sample": {"size": 1}}
            ]
            
            cursor = collection.aggregate(pipeline)
            animations = await cursor.to_list(length=1)

            if not animations:
                logger.warning(f"Nenhuma animação encontrada para a categoria: '{current_interaction_type}'")
                await asyncio.sleep(2)
                continue

            animation = animations[0]
            frames = animation.get("frames", [])

            if not frames:
                logger.warning(f"A animação '{animation.get('name')}' foi sorteada mas não possui frames válidos.")
                await asyncio.sleep(1)
                continue

            logger.info(f"[Sorteada] Transmitindo: '{animation.get('name')}' (Categoria: {current_interaction_type}) - {len(frames)} frames")

            # 2. Varre os frames da animação sorteada
            for frame in frames:
                # Se o hardware desconectar no meio da transmissão, interrompe imediatamente
                if not active_connections:
                    break

                hex_frame_data = str(frame.get("data", "")).strip()
                
                if hex_frame_data:
                    send_tasks = [
                        connection.send_text(hex_frame_data) 
                        for connection in active_connections
                    ]
                    await asyncio.gather(*send_tasks, return_exceptions=True)

                # Controle de Frame Rate (~24 FPS)
                await asyncio.sleep(0.080)

            # Pequeno intervalo opcional entre o fim de uma animação e o sorteio da próxima
            await asyncio.sleep(0.1)

        except Exception as e:
            logger.error(f"Erro crítico no loop do Worker: {e}")
            await asyncio.sleep(1)


@app.get("/")
def methodGet():
   return {"status": "API Mochi v2 está operacional e rodando em modo aleatório"}

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
            data = await websocket.receive_text()
            logger.info(f"Mensagem recebida do Mochi: {data}")
            
            # Dinâmico: Se o Mochi ou uma rota de controle enviar "change_type:happy",
            # o loop passará a sortear aleatoriamente apenas animações do tipo "happy"
            if "change_type:" in data:
                global current_interaction_type
                novo_tipo = data.split(":")[1].strip()
                current_interaction_type = novo_tipo
                logger.info(f"Categoria de animação alterada para: {current_interaction_type}")

    except WebSocketDisconnect:
        active_connections.remove(websocket)
        logger.warning(f"Mochi Robot desconectou do canal WebSocket. Clientes restantes: {len(active_connections)}")
    except Exception as e:
        if websocket in active_connections:
            active_connections.remove(websocket)
        logger.error(f"Erro na conexão WebSocket: {e}")

# -------------------------------------------------------------------------
# GATILHO DE INICIALIZAÇÃO
# -------------------------------------------------------------------------
@app.on_event("startup")
async def startup_event():
    asyncio.create_task(animation_streamer_loop())


'''
# Bash
curl -X POST "http://localhost:8000/set-category?category=happy"
'''