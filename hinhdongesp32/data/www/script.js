// Seus dados JSON originais
let data = {
    "id": "1",
    "name": "hat_xi",
    "frames": [
        {
            "frame_name": "frame_00",
            "frame": ""
        }
        // Você pode adicionar mais frames aqui se necessário
    ],
    "wake_up_time": "",
    "sleep_time": ""
};

let area = [
    {
        "ssid": "PERIGO",
        "rssi": -44,
        "enc": "Protegida"
    },
    {
        "ssid": "EGBBS2.4",
        "rssi": -66,
        "enc": "Protegida"
    },
    {
        "ssid": "WLINKS GUIMARAES",
        "rssi": -83,
        "enc": "Protegida"
    },
    {
        "ssid": "WLINKS - Área 51",
        "rssi": -92,
        "enc": "Protegida"
    }
]

// Função para preencher o formulário com os dados JSON
window.addEventListener('load', async () => {
    document.getElementById('name').value = data.name;
    document.getElementById('wake_up_time').value = data.wake_up_time;
    document.getElementById('sleep_time').value = data.sleep_time;

    let txtarea = document.querySelector('.frame-section');
    data.frames.forEach((frameData, index) => {

        txtarea.innerHTML = `
            <div class="form-group">
                <label for="frameName_${index}">Nome do Frame ${index + 1}:</label>
                <input type="text" id="frameName_${index}" value="${frameData.frame_name}">
            </div>
            <div class="form-group">
                <label for="frameData_${index}">Dados do Frame ${index + 1} (Hex):</label>
                <textarea id="frameData_${index}" readonly>${frameData.frame}</textarea>
            </div>
        `;
    });
})

// Função para lidar com o envio do formulário (simulado)
document.getElementById('animationForm').addEventListener('submit', function(event) {
    event.preventDefault();

    // Coleta os dados do formulário para um novo objeto JSON
    let updatedData = {
        name: document.getElementById('name').value,
        wake_up_time: document.getElementById('wake_up_time').value,
        sleep_time: document.getElementById('sleep_time').value,
        frames: []
    };

    // Coleta dados dos frames
    document.querySelectorAll('.frame-section').forEach((frameDiv, index) => {
        let frameName = document.getElementById(`frameName_${index}`).value;
        let frameValue = document.getElementById(`frameData_${index}`).value; // Dados brutos (readonly)

        updatedData.frames.push({
            frame_name: frameName,
            frame: frameValue
        });
    });

    console.log("Dados a serem enviados/salvos:", updatedData);
    alert("Formulário enviado com sucesso! Verifique o console para o objeto JSON gerado.");
});

const animationDialog = document.getElementById('animationDialog');
const acultar_form = document.querySelector('.acultar_form');
const closeDialogButton = document.getElementById('closeDialogButton');
// 1. Mostrar o diálogo quando o botão "Abrir" for clicado
acultar_form.addEventListener('click', () => {
    // showModal() abre o diálogo e adiciona um backdrop (fundo escuro)
    animationDialog.showModal(); 
});

// 2. Fechar o diálogo quando o botão "Fechar" for clicado
closeDialogButton.addEventListener('click', () => {
    animationDialog.close();
});

// Chame a função quando o documento estiver pronto
document.addEventListener('DOMContentLoaded', () => {
    const switches = document.querySelectorAll('.switch');
    switches.forEach(switchElement => {
        switchElement.addEventListener('change', () => {
            const deviceId = this.id.split('-')[1]; // Ex: 'switch-wifi' -> 'wifi'
            const newState = this.checked ? 'on' : 'off';
            
            // Redireciona para a rota /toggle com os parâmetros de dispositivo e estado
            window.location.href = `/toggle?device=${deviceId}&state=${newState}`;
        });
    });
});


//DATE HOUR
const currentHourElement = document.getElementById('current_hour');
const currentDateElement = document.getElementById('current_date');

async function updateDateTime() {
    try {
        // Chama o novo endpoint que retorna a data e hora em formato JSON
        const response = await fetch('/datetime');
        
        if (response.ok) {
            const data = await response.json();
            
            // Atualiza os elementos HTML com os dados JSON
            currentHourElement.textContent = data.time;
            currentDateElement.textContent = data.date;
        } else {
            currentHourElement.textContent = "--:--:--";
            currentDateElement.textContent = "--/--/----";
            console.error("Erro ao buscar data/hora. Status:", response.status);
        }
    } catch (error) {
        console.error("Erro de rede ao buscar data/hora:", error);
        currentHourElement.textContent = "";
        currentDateElement.innerHTML = '<span class="material-symbols-outlined icon">wifi_off</span>';
    }
}
// Chama a função a cada 1 segundo (1000ms)
setInterval(updateDateTime, 1000); 
updateDateTime(); // Chama imediatamente na carga


//SCAN SCRIPT
// 1. Defina a função delay no topo do seu script
const delay = ms => new Promise(res => setTimeout(res, ms));

const scan = document.querySelector(".scan");
const item = document.querySelector('.conteiner_scan');
const conteiner = document.getElementById("lista_redes");

scan.addEventListener("click", async () => {
    item.innerHTML = "";
    conteiner.innerHTML = "<p style='padding:15px;'>Buscando redes...</p>";
    
    // Desabilitar o botão para evitar múltiplos cliques simultâneos
    scan.disabled = true;

    try {
        let escaneando = true;
        let tentativas = 0;

        while (escaneando && tentativas < 10) { // Limite de segurança de 10 tentativas (20s)
            const conexao = await fetch('/scan');
            
            if (conexao.status === 200) {
                const redes = await conexao.json();        
                conteiner.innerHTML = ""; 
                
                if (redes.length === 0) {
                    item.innerHTML = "<p style='padding:15px;'>Nenhuma rede encontrada.</p>";
                } else {
                    redes.forEach(rede => {
                        // Importante: use classes em vez de IDs repetidos
                        item.innerHTML += `
                            <form id="isProtected">
                                <div class="form-group">
                                    <input type="checkbox">
                                    <label for="nomeRede">SSID: <span>${rede.ssid}</span></label>
                                    <label for="potencia">Potência: <span>${rede.rssi} dBm</span></label>
                                    <label for="encryption">Encryption: <span>${rede.enc}</span></label>
                                </div>
                                <div id="passGroupOculto" class="oculto">
                                    <input type="password" name="password" data-ssid="${rede.ssid}" placeholder="Digite a senha" required>
                                    <button type="submit">Conectar</button>
                                </div>
                            </form>
                            <hr style="border: 0.1px solid var(--color-boder); margin: 10px 0;">
                        `;
                    });
                }
                escaneando = false; 

            } else if (conexao.status === 202) {
                tentativas++;
                await delay(2000); // Espera 2 segundos antes da próxima volta do loop
            } else {
                throw new Error("Erro no servidor");
            }
        }
    } catch (error) {
        conteiner.innerHTML = "<p style='padding:15px;'>Erro na conexão!</p>";
    } finally {
        scan.disabled = false; // Reabilita o botão
    }
});
// Função que abre/fecha a div da senha
item.addEventListener('change', (event) => {
    // 1. O checkbox que disparou o evento
    const checkbox = event.target;

    // 2. Encontra o "passGroup" que está LOGO APÓS a div pai do checkbox
    // Usamos nextElementSibling para pular espaços vazios do HTML
    let passDiv = checkbox.parentElement.nextElementSibling;

    // 3. Verificamos se o passDiv existe e se é o elemento correto
    if (passDiv && (passDiv.id === "passGroupOculto")) {
        if (checkbox.checked) {
            passDiv.classList.remove("oculto");
            passDiv.querySelector('input').focus();
        } else {
            passDiv.classList.add("oculto");
        }
    }
});

item.addEventListener('submit', async (event) => {
    event.preventDefault();
    let ssid = event.target['password'].dataset.ssid
    let passw = event.target.elements['password'].value;
    // Envia os dados para o ESP32
    const response = await fetch(`/connect?ssid=${ssid}&pass=${passw}`);
    if (response.ok) {
        alert("Comando enviado! Aguarde a conexão.");
    }
});