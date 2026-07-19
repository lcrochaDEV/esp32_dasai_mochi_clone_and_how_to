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
COLLECTION_NAME = "MochiDataBase"

client = AsyncIOMotorClient(MONGO_URI)
db = client[DB_NAME]
collection = db[COLLECTION_NAME]

# Gerenciador de conexões WebSocket ativas
active_connections: set[WebSocket] = set()

# Categoria padrão do sistema. Vai rodar aleatoriamente entre as cadastradas com este type.
current_interaction_type = "animation" 
frame_delay = 0.042  # Delay padrão (~24 FPS) em segundos

# Sistema de Filas e Interrupção Imediata
priority_queue = None
interrupt_current = False  # Flag para cortar a animação em execução na mesma hora

# -------------------------------------------------------------------------
# NOVO ENDPOINT: EXECUÇÃO IMEDIATA DE ANIMAÇÃO ESPECÍFICA
# -------------------------------------------------------------------------
@app.post("/play-now")
async def play_now(name: str = Query(..., description="Nome exato da animação que quer rodar agora")):
    """
    Endpoint HTTP para injetar uma animação específica para rodar imediatamente.
    Assim que ela terminar, o sistema volta a sortear a categoria atual de forma transparente.
    Exemplo: POST http://localhost:8000/play-now?name=piscar_olhos
    """
    global priority_queue, interrupt_current
    
    if priority_queue is None:
        logger.error("Erro: Sistema de filas 'priority_queue' não foi definido.")
        raise HTTPException(status_code=500, detail="O sistema de filas ainda não foi inicializado.")

    try:
        logger.info(f"Buscando animação imediata: '{name}' no MongoDB...")
        animation = await collection.find_one({"name": name})
        
        if not animation:
            logger.warning(f"Animação '{name}' não encontrada.")
            raise HTTPException(
                status_code=404, 
                detail=f"Animação com o nome '{name}' não foi encontrada."
            )

        if not animation.get("active", False):
            raise HTTPException(status_code=400, detail="Esta animação está cadastrada, mas está desativada.")    

        if "frames" not in animation or len(animation["frames"]) == 0:
            logger.warning(f"Animação '{name}' está sem frames.")
            raise HTTPException(
                status_code=400, 
                detail=f"A animação '{name}' existe, mas não possui frames."
            )

        # 1. Limpa a fila de prioridade antiga se houver (para garantir que toque apenas a última clicada)
        while not priority_queue.empty():
            try:
                priority_queue.get_nowait()
                priority_queue.task_done()
            except asyncio.QueueEmpty:
                break

        # 2. Adiciona a nova animação à fila
        await priority_queue.put(animation)
        
        # 3. Força a interrupção imediata da animação que está rodando agora!
        interrupt_current = True
        
        logger.info(f"Animação prioritária '{name}' adicionada. Sinal de interrupção ATIVADO!")
        
        return {
            "status": "sucesso",
            "mensagem": f"A animação '{name}' foi acionada e interromperá o frame atual imediatamente."
        }

    except HTTPException as http_err:
        raise http_err
    except Exception as e:
        logger.error(f"Erro ao processar /play-now: {e}", exc_info=True)
        raise HTTPException(
            status_code=500, 
            detail=f"Erro interno: {str(e)}"
        )
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
# NOVO ENDPOINT: ALTERAR O TEMPO DE EXIBIÇÃO (DELAY) DOS FRAMES
# -------------------------------------------------------------------------
@app.post("/set-delay")
async def set_delay(seconds: float = Query(..., description="Tempo em segundos que cada frame ficará visível. Ex: 0.042 para rápido, 1.5 para lento.")):
    """
    Endpoint HTTP para mudar a velocidade da animação em tempo real.
    Exemplo: POST http://localhost:8000/set-delay?seconds=1.5
    """
    global frame_delay
    
    if seconds <= 0:
        raise HTTPException(
            status_code=400, 
            detail="O tempo de delay por frame deve ser maior que zero segundos."
        )
        
    old_delay = frame_delay
    frame_delay = seconds
    
    logger.info(f"Velocidade alterada via API: {old_delay}s -> {frame_delay}s por frame")
    return {
        "status": "sucesso",
        "delay_anterior": f"{old_delay} segundos",
        "novo_delay": f"{frame_delay} segundos",
        "mensagem": "A nova velocidade de exibição foi aplicada instantaneamente a partir do próximo frame."
    }
# -------------------------------------------------------------------------
# LOOP PRINCIPAL DO WORKER (REPRODUTOR ALEATÓRIO)1
# -------------------------------------------------------------------------
async def animation_streamer_loop():
    global current_interaction_type, priority_queue, interrupt_current, frame_delay
    logger.info("Worker de Streaming de Animações v3.3 (Interrupção de Tela) ativo!")

    while True:
        if not active_connections:
            await asyncio.sleep(1)
            continue

        try:
            animation = None
            is_priority = False

            # 1. Verifica se há prioridade na fila
            if priority_queue is not None and not priority_queue.empty():
                animation = await priority_queue.get()
                is_priority = True
                logger.info(f"[URGENTE] Iniciando imediatamente: '{animation.get('name')}'")
                
                # VERIFICAÇÃO DE ATIVAÇÃO PARA ANIMAÇÕES PRIORITÁRIAS
                if not animation.get("active", False):
                    logger.warning(f"[URGENTE BLOCKED] A animação '{animation.get('name')}' foi solicitada, mas está inativa (active=False). Pulando.")
                    priority_queue.task_done()
                    continue
            else:
                # Se não houver prioridade, faz o sorteio normal filtrando apenas as ATIVAS (active: True)
                pipeline = [
                    {"$match": {"type": current_interaction_type, "active": True}},
                    {"$sample": {"size": 1}}
                ]
                cursor = collection.aggregate(pipeline)
                animations = await cursor.to_list(length=1)
                
                if animations:
                    animation = animations[0]
                else:
                    logger.warning(f"Nenhuma animação ativa encontrada para a categoria: '{current_interaction_type}'")
                    await asyncio.sleep(2)
                    continue

            # Reseta a flag de interrupção antes de começar a exibir os frames
            interrupt_current = False

            frames = animation.get("frames", [])
            if not frames:
                if is_priority:
                    priority_queue.task_done()
                continue

            logger.info(f"Transmitindo: '{animation.get('name')}' | Origem: {'PRIORITÁRIA' if is_priority else 'SORTEIO'}")

            for frame in frames:
                if not active_connections:
                    break

                # >>> INTERRUPÇÃO INSTANTÂNEA <<<
                if interrupt_current and not is_priority:
                    logger.info(f"Interrompendo a animação '{animation.get('name')}' no meio!")
                    
                    # ENVIAR COMANDO DE LIMPEZA OU FRAME PRETO AO ESP32
                    # Se você tiver uma string hexadecimal que representa a tela toda apagada (ex: tudo 0x00), 
                    # envie-a aqui. Caso contrário, enviar uma string vazia ou comando de reset limpa o buffer.
                    clear_tasks = [
                        connection.send_text("CLEAR") # Ou envie uma string de zeros compatível com seu ESP32
                        for connection in active_connections
                    ]
                    await asyncio.gather(*clear_tasks, return_exceptions=True)
                    break

                hex_frame_data = str(frame.get("data", "")).strip()
                
                if hex_frame_data:
                    send_tasks = [
                        connection.send_text(hex_frame_data) 
                        for connection in active_connections
                    ]
                    await asyncio.gather(*send_tasks, return_exceptions=True)

                await asyncio.sleep(frame_delay)

            # Finaliza a tarefa da fila prioritária
            if is_priority:
                priority_queue.task_done()
                interrupt_current = False 
                logger.info(f"[URGENTE] Concluída reprodução de '{animation.get('name')}'. Retornando ao fluxo padrão.")

            await asyncio.sleep(0.1)

        except Exception as e:
            logger.error(f"Erro crítico no loop do Worker: {e}", exc_info=True)
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
    global priority_queue
    # Inicializa a fila de forma segura no loop de eventos correto
    priority_queue = asyncio.Queue()
    logger.info("Fila 'priority_queue' inicializada com sucesso!")
    
    # Inicia o loop de streaming em background
    asyncio.create_task(animation_streamer_loop())


'''
# Bash
# CATEGORIA
curl -X POST "http://localhost:8000/set-category?category=happy"
# ANIMAÇÃO
curl -X POST "http://localhost:8000//play-now?name=smirk"
# TEMPO DA ANIMAÇÃO
curl -X POST "http://192.168.1.252:8003/set-delay?seconds=0.09"
# VERIFICA QUAL CATEGORIA
curl -X GET "http://192.168.1.252:8003/current-category"
'''