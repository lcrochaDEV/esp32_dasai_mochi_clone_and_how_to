# Força o terminal do Windows a usar UTF-8
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
[Console]::InputEncoding  = [System.Text.Encoding]::UTF8

# Configurações herdadas
$ENV_PATH = "env"
$DB_CONTAINER = "postgres_db"
$CACHE_CONTAINER = "memcached_server"
$LISTA_NODE = "dependencies.txt"
$DB_MONGODB = "mongoDB"

# Códigos de Cor ANSI para o terminal
$VERMELHO = "$([char]0x1b)[31m"
$VERDE    = "$([char]0x1b)[32m"
$RESET    = "$([char]0x1b)[0m"
$CINZA    = "$([char]0x1b)[90m"

# --- FUNÇÕES DE APOIO ---

# Função para verificar o status do Git nas subpastas com filtros e identificação
function Verificar-PushGit {
    Write-Host "`nVerificando repositórios Git nas subpastas...`n"
    
    $diretorios = Get-ChildItem -Directory
    
    if ($diretorios.Count -eq 0) {
        Write-Host "${VERMELHO}Nenhuma subpasta foi encontrada neste diretório: $(Get-Location)${RESET}"
        Read-Host "`nPressione [Enter] para voltar ao menu..."
        return
    }

    $encontrouGit = $false

    foreach ($dir in $diretorios) {
        $nome_pasta = $dir.Name

        # --- FILTRO: Ignora pastas de ambiente e cache ---
        if ($nome_pasta -eq "env" -or $nome_pasta -eq "env" -or $nome_pasta -eq "__pycache__" -or $nome_pasta -eq "node_modules") {
            continue
        }

        $caminhoFormatado = $dir.FullName.Replace('\', '/')
        & git -C "$caminhoFormatado" rev-parse --is-inside-work-tree 2>$null
        
        if ($LASTEXITCODE -eq 0) {
            $encontrouGit = $true
            
            & git -C "$caminhoFormatado" diff --quiet HEAD 2>$null
            if ($LASTEXITCODE -eq 0) { $COR = $VERDE } else { $COR = $VERMELHO }

            Write-Host "${COR}--- Status em: $nome_pasta ---${RESET}"
            & git -C "$caminhoFormatado" status -s
            Write-Host ""
        } else {
            # --- DETECÇÃO DE TIPO DE PROJETO ---
            $tipo_projeto = ""
            if (Test-Path (Join-Path $dir.FullName "package.json")) {
                $tipo_projeto = " [Projeto Node.js]"
            } elseif (Test-Path (Join-Path $dir.FullName "requirements.txt") -or (Test-Path (Join-Path $dir.FullName "env")) -or (Test-Path (Join-Path $dir.FullName "env")) -or (Get-ChildItem (Join-Path $dir.FullName "*.py") -ErrorAction SilentlyContinue)) {
                $tipo_projeto = " [Projeto Python]"
            }

            Write-Host "${CINZA}[Ignorado] A pasta '$nome_pasta'$tipo_projeto não é um repositório Git.${RESET}"
        }
    }

    if (-not $encontrouGit) {
        Write-Host "`n${VERMELHO}Nenhuma das subpastas encontradas é um repositório Git válido.${RESET}"
    }

    Write-Host ""
    Read-Host "Pressione [Enter] para voltar ao menu..."
}

# Função para forçar a liberação de portas
function Limpar-Portas {
    $portaDigitada = Read-Host "Digite a porta que deseja liberar (ex: 80, 8000, 3000)"
    if ([string]::IsNullOrEmpty($portaDigitada)) { $portaFinal = "8000" } else { $portaFinal = $portaDigitada }
    
    Write-Host "Limpando processos na porta $portaFinal..."
    
    $conexoes = Get-NetTCPConnection -LocalPort $portaFinal -ErrorAction SilentlyContinue
    if ($conexoes) {
        foreach ($conexao in $conexoes) {
            Stop-Process -Id $conexao.OwningProcess -Force -ErrorAction SilentlyContinue
        }
        Write-Host "Porta $portaFinal liberada com sucesso."
    } else {
        Write-Host "Nenhum processo encontrado na porta $portaFinal."
    }
    Read-Host "Pressione [Enter] para voltar ao menu..."
}

# Função para criar arquivos base na raiz
function Criar-ArquivosBase {
    if (-not (Test-Path ".gitignore")) {
        echo "Criando .gitignore..."
        "node_modules/`n__pycache__/`n$ENV_PATH/`n*.pyc`n.env`n.DS_Store`ndist/`nbuild/" | Out-File -FilePath ".gitignore" -Encoding utf8
    }
    if (-not (Test-Path ".dockerignore")) {
        echo "Criando .dockerignore..."
        "node_modules/`n$ENV_PATH/`n__pycache__/`n.git`n.gitignore" | Out-File -FilePath ".dockerignore" -Encoding utf8
    }
    if (-not (Test-Path ".env")) {
        echo "Criando .env (vazio)..."
        New-Item -Path ".env" -ItemType "file" >$null
    }
    if (-not (Test-Path "README.md")) {
        echo "Criando README.md..."
        "# Projeto`nGerenciado por script shell." | Out-File -FilePath "README.md" -Encoding utf8
    }
}

# Prepara o ambiente virtual Python Windows
function Preparar-env {
    Criar-ArquivosBase
    if (-not (Test-Path $ENV_PATH)) {
        Write-Host "Criando ambiente virtual ($ENV_PATH)..."
        python -m venv $ENV_PATH
    }
}

# Garante que o package.json exista no Node.js
function Verificar-PackageJson {
    Criar-ArquivosBase
    if (-not (Test-Path "package.json")) {
        Write-Host "Arquivo package.json não encontrado. Iniciando projeto Node.js..."
        npm init -y
    }
}

# --- MENU PYTHON ---
function Menu-Python {
    do {
        Write-Host "`n=========================================="
        Write-Host "       GERENCIADOR DE PROJETO PYTHON"
        Write-Host "=========================================="
        Write-Host "1) Iniciar Servidor (uvicorn)"
        Write-Host "2) Instalar Dependências (pip + requirements)"
        Write-Host "3) Limpar Arquivos Temporários (__pycache__)"
        Write-Host "4) Verificar/Criar Ambiente Virtual (ENV)"
        Write-Host "5) Acessar Terminal PostgreSQL (Docker)"
        Write-Host "6) Acessar Terminal Memcached (Docker)"
        Write-Host "7) Criar Arquivos Base (.ignore, .env, README)"
        Write-Host "8) Liberar Porta TCP"
        Write-Host "9) Verificar PushGit (Status Subpastas)"
        Write-Host "10) Acessar Terminal MongoDB (Docker)"
        Write-Host "11) Sair/Voltar"
        Write-Host "=========================================="
        $opcao = Read-Host "Escolha uma opção [1-11]"

        switch ($opcao) {
            "1" {
                Preparar-env
                if (-not (Test-Path "$ENV_PATH\Scripts\uvicorn.exe")) {
                    Write-Host "ERRO: uvicorn não instalado na env. Use a Opção 2."
                } else {
                    & "$ENV_PATH\Scripts\uvicorn.exe" app:app --reload
                }
            }
            "2" {
                Preparar-env
                Write-Host "Instalando uvicorn e dependências na env..."
                & "$ENV_PATH\Scripts\pip.exe" install uvicorn
                if (Test-Path "requirements.txt") {
                    & "$ENV_PATH\Scripts\pip.exe" install -r requirements.txt
                } else {
                    Write-Host "Aviso: requirements.txt não encontrado."
                }
            }
            "3" {
                Get-ChildItem -Recurse -Directory -Filter "__pycache__" | Remove-Item -Recurse -Force
                Write-Host "Cache limpo."
            }
            "4" { Preparar-env; Write-Host "Ambiente env verificado em: $ENV_PATH" }
            "5" {
                if (docker ps -q -f name=$DB_CONTAINER) {
                    Write-Host "Conectando ao PostgreSQL no container $DB_CONTAINER..."
                    docker exec -it $DB_CONTAINER psql -U postgres
                } else {
                    Write-Host "ERRO: O container '$DB_CONTAINER' não está rodando!" -ForegroundColor Red
                }
            }
            "6" {
                if (docker ps -q -f name=$CACHE_CONTAINER) {
                    Write-Host "Conectando ao Memcached no container $CACHE_CONTAINER..."
                    docker exec -it $CACHE_CONTAINER telnet localhost 11211 2>$null
                } else {
                    Write-Host "ERRO: O container '$CACHE_CONTAINER' não está rodando!" -ForegroundColor Red
                }
            }
            "7" { Criar-ArquivosBase; Write-Host "Arquivos base verificados/criados." }
            "8" { Limpar-Portas }
            "9" { Verificar-PushGit }
            "10"{
               
                if (docker ps -q -f name=$DB_MONGODB) {
                    Write-Host "Conectando ao MongoDB no container $DB_MONGODB..."
                    docker exec -it $DB_MONGODB mongosh -u admin -p admin
                } else {
                    Write-Host "ERRO: O container '$DB_MONGODB' não está rodando!" -ForegroundColor Red
                }
            }
            "11" { exit }
        }
    } while ($opcao -ne "10")
}

# --- MENU NODE.JS ---
function Menu-Node {
    do {
        Write-Host "`n=========================================="
        Write-Host "       GERENCIADOR DE PROJETO NODE.JS"
        Write-Host "=========================================="
        Write-Host "1) Iniciar App (npm start)"
        Write-Host "2) Instalar via package.json (npm install)"
        Write-Host "3) Instalar via LISTA ($LISTA_NODE)"
        Write-Host "4) Acessar Terminal PostgreSQL (Docker)"
        Write-Host "5) Acessar Terminal Memcached (Docker)"
        Write-Host "6) Criar Arquivos Base (.ignore, .env, README)"
        Write-Host "7) Liberar Porta TCP"
        Write-Host "8) Verificar PushGit (Status Subpastas)"
        Write-Host "9) Acessar Terminal MongoDB (Docker)"
        Write-Host "10) Sair/Voltar"
        Write-Host "=========================================="
        $opcao = Read-Host "Escolha uma opção [1-9]"

        switch ($opcao) {
            "1" { Verificar-PackageJson; npm start }
            "2" { Verificar-PackageJson; npm install }
            "3" {
                if (Test-Path $LISTA_NODE) {
                    Verificar-PackageJson
                    Write-Host "Instalando pacotes de $LISTA_NODE..."
                    Get-Content $LISTA_NODE | ForEach-Object { if ($_ -trim) { npm install $_ } }
                } else {
                    Write-Host "Erro: Arquivo '$LISTA_NODE' não encontrado." -ForegroundColor Red
                }
            }
            "4" {
                if (docker ps -q -f name=$DB_CONTAINER) {
                    docker exec -it $DB_CONTAINER psql -U postgres
                } else {
                    Write-Host "ERRO: O container '$DB_CONTAINER' não está rodando!" -ForegroundColor Red
                }
            }
            "5" {
                if (docker ps -q -f name=$CACHE_CONTAINER) {
                    docker exec -it $CACHE_CONTAINER telnet localhost 11211 2>$null
                } else {
                    Write-Host "ERRO: O container '$CACHE_CONTAINER' não está rodando!" -ForegroundColor Red
                }
            }
            "6" { Criar-ArquivosBase; Write-Host "Arquivos base verificados/criados." }
            "7" { Limpar-Portas }
            "8" { Verificar-PushGit }
            "9"{
               
                if (docker ps -q -f name=$DB_MONGODB) {
                    Write-Host "Conectando ao MongoDB no container $DB_MONGODB..."
                    docker exec -it $DB_MONGODB mongosh -u admin -p admin
                } else {
                    Write-Host "ERRO: O container '$DB_MONGODB' não está rodando!" -ForegroundColor Red
                }
            }
            "10" { exit }
        }
    } while ($opcao -ne "9")
}

# --- IDENTIFICAÇÃO DO AMBIENTE ---
function Identificar-Ambiente {
    if (Test-Path "package.json" -or (Test-Path $LISTA_NODE)) {
        return "node"
    } elseif (Test-Path "requirements.txt" -or (Test-Path $ENV_PATH) -or (Get-ChildItem "*.py" -ErrorAction SilentlyContinue)) {
        return "python"
    } else {
        return "hibrido"
    }
}

# --- FLUXO PRINCIPAL ---
$AMBIENTE = Identificar-Ambiente

if ($AMBIENTE -eq "node") {
    Menu-Node
} elseif ($AMBIENTE -eq "python") {
    Menu-Python
} else {
    do {
        Write-Host "`n=========================================="
        Write-Host "       AMBIENTE NÃO IDENTIFICADO"
        Write-Host "=========================================="
        Write-Host "1) Iniciar como Python"
        Write-Host "2) Iniciar como Node.js (Gera package.json)"
        Write-Host "3) Verificar PushGit (Status Subpastas)"
        Write-Host "4) Sair"
        Write-Host "=========================================="
        $escolha = Read-Host "Escolha uma opção [1-4]"

        switch ($escolha) {
            "1" { Menu-Python }
            "2" { Menu-Node }
            "3" { Verificar-PushGit }
            "4" { exit }
        }
    } while ($escolha -ne "4")
}