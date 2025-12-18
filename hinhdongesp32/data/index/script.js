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
// 1. Obter referências aos elementos
let isProtectedCheckbox = document.getElementById('isProtected');
let passwordGroup = document.getElementById('passwordGroup');
isProtectedCheckbox.addEventListener('change', function() {
    if (this.checked) {
        passwordGroup.classList.remove('oculto');
        // Torna o campo de senha obrigatório quando visível
        document.getElementById('senha').setAttribute('required', true); 
    } else {
        passwordGroup.classList.add('oculto');
        document.getElementById('senha').removeAttribute('required');
    }
});
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
        switchElement.addEventListener('change', function() {
            const deviceId = this.id.split('-')[1]; // Ex: 'switch-wifi' -> 'wifi'
            const newState = this.checked ? 'on' : 'off';
            
            // Redireciona para a rota /toggle com os parâmetros de dispositivo e estado
            window.location.href = `/toggle?device=${deviceId}&state=${newState}`;
        });
    });
});


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
