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
});

// Formulário de Configuração de Animação
document.getElementById('animationForm').addEventListener('submit', function(event) {
    event.preventDefault();

    let updatedData = {
        name: document.getElementById('name').value,
        wake_up_time: document.getElementById('wake_up_time').value,
        sleep_time: document.getElementById('sleep_time').value,
        frames: []
    };

    document.querySelectorAll('.frame-section').forEach((frameDiv, index) => {
        let frameName = document.getElementById(`frameName_${index}`).value;
        let frameValue = document.getElementById(`frameData_${index}`).value;

        updatedData.frames.push({
            frame_name: frameName,
            frame: frameValue
        });
    });

    console.log("Dados a serem enviados/salvos:", updatedData);
    alert("Formulário enviado com sucesso!");
});

// Controle do Modal/Dialog
const animationDialog = document.getElementById('animationDialog');
const acultar_form = document.querySelector('.acultar_form');
const closeDialogButton = document.getElementById('closeDialogButton');

acultar_form.addEventListener('click', () => {
    animationDialog.showModal(); 
});

closeDialogButton.addEventListener('click', () => {
    animationDialog.close();
});

// CORREÇÃO BUG 1: Switches com arrow function corrigida usando event.target
document.addEventListener('DOMContentLoaded', () => {
    const switches = document.querySelectorAll('.switch');
    switches.forEach(switchElement => {
        switchElement.addEventListener('change', (event) => {
            const target = event.target; 
            const deviceId = target.id.split('-')[1]; // 'switch-wifi' -> 'wifi'
            const newState = target.checked ? 'on' : 'off';
            
            window.location.href = `/toggle?device=${deviceId}&state=${newState}`;
        });
    });
});

// RELÓGIO (DATE & HOUR)
const currentHourElement = document.getElementById('current_hour');
const currentDateElement = document.getElementById('current_date');

async function updateDateTime() {
    try {
        const response = await fetch('/datetime');
        if (response.ok) {
            const data = await response.json();
            currentHourElement.textContent = data.time;
            currentDateElement.textContent = data.date;
        } else {
            currentHourElement.textContent = "--:--:--";
            currentDateElement.textContent = "--/--/----";
        }
    } catch (error) {
        console.error("Erro de rede ao buscar data/hora:", error);
        currentHourElement.textContent = "";
        currentDateElement.innerHTML = '<span class="material-symbols-outlined icon">wifi_off</span>';
    }
}
setInterval(updateDateTime, 1000); 
updateDateTime();

// Auxiliar de atraso (Delay)
const delay = ms => new Promise(res => setTimeout(res, ms));

// SCAN DE REDES WI-FI
const scan = document.querySelector(".scan");
const item = document.querySelector('.conteiner_scan');
const conteiner = document.getElementById("lista_redes");

scan.addEventListener("click", async () => {
    item.innerHTML = "";
    conteiner.innerHTML = "<p style='padding:15px;'>Buscando redes...</p>";
    scan.disabled = true;

    try {
        let escaneando = true;
        let tentativas = 0;

        while (escaneando && tentativas < 10) {
            const conexao = await fetch('/scan');
            
            if (conexao.status === 200) {
                const redes = await conexao.json();        
                conteiner.innerHTML = ""; 
                
                if (redes.length === 0) {
                    item.innerHTML = "<p style='padding:15px;'>Nenhuma rede encontrada.</p>";
                } else {
                    redes.forEach(rede => {
                        item.innerHTML += `
                            <form class="wifi-connect-form">
                                <div class="form-group">
                                    <input type="checkbox" class="show-pass-check">
                                    <label>SSID: <span>${rede.ssid}</span></label>
                                    <label>Potência: <span>${rede.rssi} dBm</span></label>
                                    <label>Encryption: <span>${rede.enc}</span></label>
                                </div>
                                <div class="passGroupOculto oculto">
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
                await delay(2000);
            } else {
                throw new Error("Erro no servidor");
            }
        }
    } catch (error) {
        conteiner.innerHTML = "<p style='padding:15px;'>Erro na conexão!</p>";
    } finally {
        scan.disabled = false;
    }
});

// SCAN ESP-NOW (CORREÇÃO BUG 2: Seletor consertado para minúsculo '.conteiner_esp-now')
const espnow = document.querySelector(".esp-now");
const itemEspnow = document.querySelector('.conteiner_esp-now');
const conteiner_Espnow = document.getElementById("lista_esp-now");

espnow.addEventListener("click", async () => {
    itemEspnow.innerHTML = "";
    conteiner_Espnow.innerHTML = "<p style='padding:15px;'>Buscando dispositivos ESP-NOW...</p>";
    espnow.disabled = true;

    try {
        let escaneando = true;
        let tentativas = 0;
        while (escaneando && tentativas < 10) {
            let dataRequest = await ConnectJson.connectJsonUrlJson('/espnow');

            if (dataRequest && dataRequest.length >= 0) {
                conteiner_Espnow.innerHTML = ""; 
                if (dataRequest.length === 0) {
                    itemEspnow.innerHTML = "<p style='padding:15px;'>Nenhum dispositivo encontrado.</p>";
                } else {
                    dataRequest.forEach(dev => {
                        itemEspnow.innerHTML += `
                            <form class="esp-connect-form">
                                <div class="form-group">
                                    <label>SSID: <span>${dev.ssid || 'Desconhecido'}</span></label>
                                    <label>MAC: <span>${dev.mac}</span></label>
                                </div>
                            </form>
                            <hr style="border: 0.1px solid var(--color-boder); margin: 10px 0;">
                        `;
                    });
                }
                escaneando = false;
            } else if (dataRequest && dataRequest.status === 202) {
                tentativas++;
                await delay(2000);
            } else {
                throw new Error("Erro no servidor");
            }
        }
    } catch (error) {
        conteiner_Espnow.innerHTML = "<p style='padding:15px;'>Erro na conexão ESP-NOW!</p>";
    } finally {
        espnow.disabled = false;
    }
});

// Evento dinâmico para abrir a senha das redes injetadas via HTML
item.addEventListener('change', (event) => {
    const checkbox = event.target;
    if (checkbox.type === 'checkbox') {
        let passDiv = checkbox.parentElement.nextElementSibling;
        if (passDiv && passDiv.classList.contains("passGroupOculto")) {
            if (checkbox.checked) {
                passDiv.classList.remove("oculto");
                passDiv.querySelector('input').focus();
            } else {
                passDiv.classList.add("oculto");
            }
        }
    }
});

// Envio das credenciais Wi-Fi para o ESP32
item.addEventListener('submit', async (event) => {
    event.preventDefault();
    const form = event.target;
    const passwordInput = form.querySelector('input[type="password"]');
    
    if (passwordInput) {
        let ssid = passwordInput.dataset.ssid;
        let passw = passwordInput.value;
        
        const response = await fetch(`/connect?ssid=${encodeURIComponent(ssid)}&pass=${encodeURIComponent(passw)}`);
        if (response.ok) {
            alert(`Comando enviado! Tentando conectar ao SSID: ${ssid}`);
        }
    }
});

const wakeonHtmlForm = document.getElementById('wakeonHtmlForm');
const webWakeUpInput = document.getElementById('web_wake_up_time');
const webSleepInput = document.getElementById('web_sleep_time');

// 1. Carrega as configurações salvas no SD assim que o painel abrir
window.addEventListener('DOMContentLoaded', async () => {
    try {
        const response = await fetch('/get-config');
        if (response.ok) {
            const config = await response.json();
            if (config.wake_up_time) webWakeUpInput.value = config.wake_up_time;
            if (config.sleep_time) webSleepInput.value = config.sleep_time;
        }
    } catch (err) {
        console.error("Erro ao carregar configurações do SD:", err);
    }
});

// 2. Envia as atualizações via POST dinâmico para salvamento e alteração imediata
wakeonHtmlForm.addEventListener('submit', async (event) => {
    event.preventDefault();
    
    const payload = {
        name: data.name || "Mochi C3", // Aproveita o objeto de dados existente
        wake_up_time: webWakeUpInput.value,
        sleep_time: webSleepInput.value
    };

    try {
        const response = await fetch('/save-config', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(payload)
        });

        if (response.ok) {
            alert("Configurações de Wakeon salvas com sucesso no Cartão SD!");
        } else {
            alert("Erro ao salvar dados no servidor.");
        }
    } catch (error) {
        console.error("Erro na requisição HTTP POST:", error);
        alert("Falha de comunicação com o ESP32.");
    }
});