#!/bin/bash

# Configurações herdadas
ENV_PATH="env"
DB_CONTAINER="postgres_db"
CACHE_CONTAINER="memcached_server"
LISTA_NODE="dependencies.txt"

# Códigos de Cor ANSI para o Git Status
VERMELHO='\033[31m' # Repositório "sujo"
VERDE='\033[32m'    # Repositório "limpo"
RESET='\033[0m'     # Resetar a cor
CINZA='\033[90m'    # Pasta ignorada

# --- FUNÇÕES DE APOIO ---
verificar_push_git() {
    echo -e "\nVerificando repositórios Git nas subpastas...\n"
    
    local diretorios=(*/)
    
    if [ ${#diretorios[@]} -eq 0 ] || [ ! -d "${diretorios[0]}" ]; then
        echo -e "${VERMELHO}Nenhuma subpasta foi encontrada neste diretório: $(pwd)${RESET}"
        read -p $'\nPressione [Enter] para voltar ao menu...'
        return
    fi

    local encontrouGit=false

    for dir in "${diretorios[@]}"; do
        local nome_pasta="${dir%/}"

        # --- FILTRO: Ignora pastas de ambiente e cache ---
        if [ "$nome_pasta" = "env" ] || [ "$nome_pasta" = "env" ] || [ "$nome_pasta" = "__pycache__" ] || [ "$nome_pasta" = "node_modules" ]; then
            continue
        fi

        if git -C "$dir" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
            encontrouGit=true
            
            if git -C "$dir" diff --quiet HEAD 2>/dev/null; then
                COR="$VERDE"
            else
                COR="$VERMELHO"
            fi

            echo -e "${COR}--- Status em: $nome_pasta ---${RESET}"
            git -C "$dir" status -s
            echo ""
        else
            # --- DETECÇÃO DE TIPO DE PROJETO ---
            local tipo_projeto=""
            
            if [ -f "${dir}package.json" ]; then
                tipo_projeto=" [Projeto Node.js]"
            elif [ -f "${dir}requirements.txt" ] || [ -d "${dir}env" ] || [ -d "${dir}env" ] || ls "${dir}"*.py >/dev/null 2>&1; then
                tipo_projeto=" [Projeto Python]"
            fi

            echo -e "${CINZA}[Ignorado] A pasta '$nome_pasta'$tipo_projeto não é um repositório Git.${RESET}"
        fi
    done

    if [ "$encontrouGit" = false ]; then
        echo -e "\n${VERMELHO}Nenhuma das subpastas encontradas é um repositório Git válido.${RESET}"
    fi

    echo ""
    read -p "Pressione [Enter] para voltar ao menu..."
}

limpar_portas() {
    read -p "Digite a porta que deseja liberar (ex: 80, 8000, 3000): " PORTA_DIGITADA
    PORTA_FINAL=${PORTA_DIGITADA:-8000}
    echo "Limpando processos na porta $PORTA_FINAL..."
    
    if [ $EUID -ne 0 ] && command -v sudo >/dev/null 2>&1; then
        sudo fuser -k "$PORTA_FINAL/tcp" && echo "Porta $PORTA_FINAL liberada." || echo "Nenhum processo encontrado na porta $PORTA_FINAL."
    else
        fuser -k "$PORTA_FINAL/tcp" && echo "Porta $PORTA_FINAL liberada." || echo "Nenhum processo encontrado na porta $PORTA_FINAL."
    fi
    read -p "Pressione [Enter] para voltar ao menu..."
}

criar_arquivos_base() {
    if [ ! -f ".gitignore" ]; then
        echo "Criando .gitignore..."
        cat <<EOF > .gitignore
node_modules/
__pycache__/
$ENV_PATH/
*.pyc
.env
.DS_Store
dist/
build/
EOF
    fi

    if [ ! -f ".dockerignore" ]; then
        echo "Criando .dockerignore..."
        cat <<EOF > .dockerignore
node_modules/
$ENV_PATH/
__pycache__/
.git
.gitignore
EOF
    fi

    if [ ! -f ".env" ]; then
        echo "Criando .env (vazio)..."
        touch .env
    fi

    if [ ! -f "README.md" ]; then
        echo "Criando README.md..."
        cat <<EOF > README.md
# Projeto
Gerenciado por script shell.
EOF
    fi
}

preparar_env() {
    criar_arquivos_base
    if [ ! -d "$ENV_PATH" ]; then
        echo "Criando ambiente virtual ($ENV_PATH)..."
        python3 -m env $ENV_PATH
    fi
}

verificar_package_json() {
    criar_arquivos_base
    if [ ! -f "package.json" ]; then
        echo "Arquivo package.json não encontrado. Iniciando projeto Node.js..."
        npm init -y
    fi
}

# --- MENU PYTHON ---
menu_python() {
    while true; do
        echo -e "\n=========================================="
        echo "       GERENCIADOR DE PROJETO PYTHON"
        echo "=========================================="
        echo "1) Iniciar Servidor (uvicorn)"
        echo "2) Instalar Dependências (pip + requirements)"
        echo "3) Limpar Arquivos Temporários (__pycache__)"
        echo "4) Verificar/Criar Ambiente Virtual (ENV)"
        echo "5) Acessar Terminal PostgreSQL (Docker)"
        echo "6) Acessar Terminal Memcached (Docker)"
        echo "7) Criar Arquivos Base (.ignore, .env, README)"
        echo "8) Liberar Porta TCP"
        echo "9) Verificar PushGit (Status Subpastas)"
        echo "10) Sair/Voltar"
        echo "=========================================="
        read -p "Escolha uma opção [1-10]: " OPCAO

        case $OPCAO in
            1)
                preparar_env
                if [ ! -f "$ENV_PATH/bin/uvicorn" ]; then
                    echo "ERRO: uvicorn não instalado na env. Use a Opção 2."
                else
                    $ENV_PATH/bin/uvicorn app:app --reload
                fi
                ;;
            2)
                preparar_env
                echo "Instalando uvicorn e dependências na env..."
                $ENV_PATH/bin/pip install uvicorn
                if [ -f "requirements.txt" ]; then
                    $ENV_PATH/bin/pip install -r requirements.txt
                else
                    echo "Aviso: requirements.txt não encontrado."
                fi
                ;;
            3)
                find . -type d -name "__pycache__" -exec rm -rf {} +
                echo "Cache limpo."
                ;;
            4) preparar_env; echo "Ambiente env verificado em: $ENV_PATH" ;;
            5)
                if [ "$(docker ps -q -f name=$DB_CONTAINER)" ]; then
                    echo "Conectando ao PostgreSQL no container $DB_CONTAINER..."
                    docker exec -it $DB_CONTAINER psql -U postgres
                else
                    echo "ERRO: O container '$DB_CONTAINER' não está rodando!"
                fi
                ;;
            6)
                if [ "$(docker ps -q -f name=$CACHE_CONTAINER)" ]; then
                    echo "Conectando ao Memcached no container $CACHE_CONTAINER..."
                    docker exec -it $CACHE_CONTAINER telnet localhost 11211 2>/dev/null || \
                    docker exec -it $CACHE_CONTAINER sh -c "apt-get update && apt-get install -y telnet && telnet localhost 11211"
                else
                    echo "ERRO: O container '$CACHE_CONTAINER' não está rodando!"
                fi
                ;;            
            7) criar_arquivos_base; echo "Arquivos base verificados/criados." ;;
            8) limpar_portas ;;
            9) verificar_push_git ;;
            10) exit 0 ;;
            *) echo "Opção inválida." ;;
        esac
    done
}

# --- MENU NODE.JS ---
menu_node() {
    while true; do
        echo -e "\n=========================================="
        echo "       GERENCIADOR DE PROJETO NODE.JS"
        echo "=========================================="
        echo "1) Iniciar App (npm start)"
        echo "2) Instalar via package.json (npm install)"
        echo "3) Instalar via LISTA ($LISTA_NODE)"
        echo "4) Acessar Terminal PostgreSQL (Docker)"
        echo "5) Acessar Terminal Memcached (Docker)"
        echo "6) Criar Arquivos Base (.ignore, .env, README)"
        echo "7) Liberar Porta TCP"
        echo "8) Verificar PushGit (Status Subpastas)"
        echo "9) Sair/Voltar"
        echo "=========================================="
        read -p "Escolha uma opção [1-9]: " opcao
        
        case $opcao in
            1) verificar_package_json; npm start ;;
            2) verificar_package_json; npm install ;;
            3) 
                if [ -f "$LISTA_NODE" ]; then
                    verificar_package_json
                    echo "Instalando pacotes de $LISTA_NODE..."
                    xargs npm install < "$LISTA_NODE"
                else
                    echo "Erro: Arquivo '$LISTA_NODE' não encontrado."
                fi
                ;;
            4)
                if [ "$(docker ps -q -f name=$DB_CONTAINER)" ]; then
                    docker exec -it $DB_CONTAINER psql -U postgres
                else
                    echo "ERRO: O container '$DB_CONTAINER' não está rodando!"
                fi
                ;;
            5)
                if [ "$(docker ps -q -f name=$CACHE_CONTAINER)" ]; then
                    docker exec -it $CACHE_CONTAINER telnet localhost 11211 2>/dev/null || \
                    docker exec -it $CACHE_CONTAINER sh -c "apt-get update && apt-get install -y telnet && telnet localhost 11211"
                else
                    echo "ERRO: O container '$CACHE_CONTAINER' não está rodando!"
                fi
                ;;
            6) criar_arquivos_base; echo "Arquivos base verificados/criados." ;;
            7) limpar_portas ;;
            8) verificar_push_git ;;
            9) exit 0 ;;
            *) echo "Opção inválida!" ;;
        esac
    done
}

# --- IDENTIFICAÇÃO DO AMBIENTE ---
identificar_ambiente() {
    if [ -f "package.json" ] || [ -f "$LISTA_NODE" ]; then
        echo "node"
    elif [ -f "requirements.txt" ] || [ -d "$ENV_PATH" ] || ls *.py >/dev/null 2>&1; then
        echo "python"
    else
        echo "hibrido"
    fi
}

# --- FLUXO PRINCIPAL ---
AMBIENTE=$(identificar_ambiente)

if [ "$AMBIENTE" == "node" ]; then
    menu_node
elif [ "$AMBIENTE" == "python" ]; then
    menu_python
else
    while true; do
        echo -e "\n=========================================="
        echo "       AMBIENTE NÃO IDENTIFICADO"
        echo "=========================================="
        echo "1) Iniciar como Python"
        echo "2) Iniciar como Node.js (Gera package.json)"
        echo "3) Verificar PushGit (Status Subpastas)"
        echo "4) Sair"
        echo "=========================================="
        read -p "Escolha uma opção [1-4]: " escolha
        
        case $escolha in
            1) menu_python ;;
            2) menu_node ;;
            3) verificar_push_git ;;
            4) exit 0 ;;
            *) echo "Opção inválida!" ;;
        esac
    done
fi