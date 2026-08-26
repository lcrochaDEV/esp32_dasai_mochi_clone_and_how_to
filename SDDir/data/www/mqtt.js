document.addEventListener('DOMContentLoaded', async () => {
    try {
        //Busca os dados atuais do ESP
        const response = await fetch('/status');
        const data = await response.json();
        const profiles = data.mqtt || []; // Usando a chave 'mqtt' que definimos no ESP

        document.querySelector('.mqtt-config-table').innerHTML += mqqt_fotm(profiles);

    } catch (err) {
        console.error("Erro ao carregar perfis:", err);
    }
});

// Lógica de salvamento integrada ao seu ecossistema (MQTT/ESP32)
async function saveMqttConfig() {
    const btn = document.querySelector('.btn-save');
    const formData = {
        broker: document.getElementById('broker')?.value || "",
        port: parseInt(document.getElementById('port')?.value || 1883),
        clientId: document.getElementById('clientId')?.value || "ESP32_Nexus",
        user: document.getElementById('user')?.value || "",
        passw: document.getElementById('passw')?.value || "",
        topic: document.getElementById('baseTopic')?.value || "", 
        ssl: document.getElementById('useSsl')?.checked || false,
        qos: parseInt(document.getElementById('qos')?.value || 0) // 0, 1 ou 2
    };

    if(!formData.broker || !formData.topic) {
        alert("Broker e Tópico Base são obrigatórios!");
        return;
    }

    btn.innerText = "SINCRONIZANDO...";
    btn.disabled = true;
    
    try {
        // Envio para o seu backend/ESP32
        const response = await fetch('/config_mqtt', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(formData)
        });

        if(response.ok) {
            //alert("Configuração MQTT atualizada com sucesso!");

            // Busca os dados atualizados diretamente do ESP32 de forma silenciosa
            const statusRes = await fetch('/status');
            const statusData = await statusRes.json();
            const profiles = statusData.mqtt || [];

            // Renderiza novamente a tabela com os novos dados inseridos
            document.querySelector('.mqtt-config-table').innerHTML = mqqt_fotm(profiles);

            // Limpa o formulário após o sucesso para um novo cadastro
            document.getElementById('mqtt-form').reset();
            document.getElementById('port').value = "1883";
            document.getElementById('qos').value = "1";
        } else if(response.status === 409) {
            alert("Erro: Este IP e Tópico já estão cadastrados ou limite de 10 perfis atingido.");
        }
    } catch (err) {
        console.error("Falha ao salvar:", err);
        alert("Erro ao conectar com o servidor do ESP32.");
    } finally {
        btn.innerText = "Salvar Configuração";
        btn.disabled = false;
    }
}
async function setActiveMqtt(uuid) {
    // 1. Ativa o perfil no backend
    const response = await fetch('/set_active_mqtt', {
        method: 'PATCH',
        body: JSON.stringify({ uuid })
    });

    if (response.ok) {
        // 2. Opcional: Mostra um estado de "Conectando..." na UI
        updateStatusUI(uuid, "connecting");

        // 3. Aguarda 2 segundos (tempo médio de handshake) e verifica o status real
        setTimeout(async () => {
            const statusRes = await fetch('/status');
            const data = await statusRes.json();
            
            // 4. Procura o perfil e atualiza a coluna baseado no p.online real do ESP32
            const profile = data.mqtt.find(m => m.uuid === uuid);
            updateStatusUI(uuid, profile.online ? "online" : "offline");
        }, 2000);
    }
}
async function deleteMqtt(uuid) {
    if (!confirm("Tem certeza que deseja remover este perfil MQTT?")) return;

    try {
        const response = await fetch('/delete_mqtt', {
            method: 'DELETE',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ uuid: uuid })
        });

        if (response.ok) {
            console.log("Perfil removido com sucesso");
            // Remove a linha da tabela visualmente sem precisar dar reload
            document.querySelector(`tr[data-uuid="${uuid}"]`).remove();
            
            // Opcional: Se a tabela ficar vazia, mostrar mensagem
            const tbody = document.querySelector('.mqtt-config-table tbody');
            if (tbody.children.length === 0) {
                tbody.innerHTML = '<tr><td colspan="9">Nenhum perfil cadastrado</td></tr>';
            }
        } else {
            alert("Erro ao excluir perfil.");
        }
    } catch (err) {
        console.error("Falha na requisição DELETE:", err);
    }
}

function updateStatusUI(uuid, state) {
    const row = document.querySelector(`tr[data-uuid="${uuid}"] .status-col`);
    if (!row) return;

    if (state === "online") {
        row.innerHTML = '<span style="color:#00ff00">● Online</span>';
        document.querySelector('.check [type="checkbox"]').checked = true;
    } else if (state === "connecting") {
        row.innerHTML = '<span style="color:#ffa500">● Conectando...</span>';
    } else {
        row.innerHTML = '<span style="color:#ff4d4d">● Offline</span>';
        document.querySelector('.check [type="checkbox"]').checked = false;
    }
}
let mqqt_fotm = (profiles = []) => {
    // Gera as linhas da tabela dinamicamente
    const tableRows = profiles.map(p => `
        <tr data-uuid="${p.uuid}">
            <td>${p.clientId || 'N/A'}</td>
            <td>${p.broker}</td>
            <td>${p.port}</td>
            <td>${p.user}</td>
            <td>${p.topic}</td>
            <td>${p.ssl ? 'Sim' : 'Não'}</td>
            <td>${p.qos}</td>
            <td class="check"><input type="checkbox"${p.active ? 'checked' : ''} onchange="setActiveMqtt('${p.uuid}')"></td>
            <td class="status-col">
                <span class="status-indicator" style="color: ${p.online ? '#00ff00' : '#ff4d4d'}">
                    ${p.online ? '● Online' : '● Offline'}
                </span>
            </td>
            <td><a href="javascript:void(0)" onclick="deleteMqtt('${p.uuid}')"><span class="material-symbols-outlined" style="color: #ff4d4d;">delete</span></a></td>
        </tr>
    `).join('');
    return `       
        <thead>
            <tr>
                <th>Client</th>
                <th>Broker</th>
                <th>Port</th>
                <th>User</th>
                <th>Topic</th>
                <th>SSL</th>
                <th>QoS</th>
                <th>Active</th>
                <th>Status</th>
                <th>Ações</th>
            </tr>
        </thead>
        <tbody>
            ${tableRows || '<tr><td colspan="9">Nenhum perfil cadastrado</td></tr>'}
        </tbody>
    `
}