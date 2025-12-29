from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
import os
import glob
import time
import re
import json

def varrerDados():
    array_global_animacoes = [] # Array de Arrays final
    driver = webdriver.Firefox()

    try:
        # URL com protocolo explícito para evitar InvalidArgumentError
        driver.get("https://notisrac.github.io/FileToCArray/")
        driver.maximize_window()
        wait = WebDriverWait(driver, 10)

        diretorio_imagens = './bloggif_frames_gif' 
        # Ordenar os arquivos garante a sequência correta da animação
        lista_gifs = sorted(glob.glob(os.path.join(diretorio_imagens, '*.[pP][nN][gG]')))

        if not lista_gifs:
            print("Nenhum arquivo encontrado.")
            return

        for x, caminho_arquivo in enumerate(lista_gifs):
            # 1. Limpa o resultado anterior via JS para evitar ler dados duplicados
            driver.execute_script("document.getElementById('txtResult').value = '';")
            
            # 2. Localiza e limpa o input de arquivo (alguns drivers concatenam paths se não resetar)
            file_input = wait.until(EC.presence_of_element_located((By.ID, "inFileInput")))
            file_input.send_keys(os.path.abspath(caminho_arquivo))

            # 3. Configurações (Apenas no primeiro frame para otimizar)
            if x == 0:
                driver.find_element(By.XPATH, "//option[@value='DEC']").click()
                driver.find_element(By.XPATH, "//*[@id='cbPaletteMod']/option[8]").click()
                
                txtX = driver.find_element(By.ID, "txtResizeX")
                txtX.clear()
                txtX.send_keys("128")
                
                txtY = driver.find_element(By.ID, "txtResizeY")
                txtY.clear()
                txtY.send_keys("64")

                checkbox_unsigned = driver.find_element(By.ID, "cbUnsigned")
                if not checkbox_unsigned.is_selected():
                    checkbox_unsigned.click()

                driver.find_element(By.XPATH, "//option[@value='char']").click()

            # 4. Converte e aguarda
            driver.find_element(By.ID, "btnConvert").click()
            time.sleep(1.5) 

            # 5. Extração e transformação em Array
            textarea = driver.find_element(By.ID, "txtResult")
            conteudo_h = textarea.get_attribute('value')

            # Regex para capturar tudo entre chaves { }
            match = re.search(r'\{(.*?)\}', conteudo_h, re.DOTALL)
            if match:
                # Remove quebras de linha e limpa espaços
                dados_limpos = match.group(1).replace('\n', '').replace('\r', '')
                # Transforma a string em uma lista de inteiros real
                lista_numeros = [int(n.strip()) for n in dados_limpos.split(',') if n.strip().isdigit()]
                
                # Adiciona o array do frame ao array global
                array_global_animacoes.append(lista_numeros)
                print(f"Frame {x+1}/{len(lista_gifs)} processado.")

        # 6. Exportação para JSON
        if array_global_animacoes:
            with open("animacao.json", 'w', encoding='utf-8') as f:
                # Salva como array de arrays: [[...],[...]]
                json.dump(array_global_animacoes, f)
            print(f"\nSucesso! Arquivo 'animacao.json' gerado com {len(array_global_animacoes)} frames.")

    except Exception as e:
        print(f'Erro durante a execução: {e}')
    finally:
        driver.quit()

if __name__ == "__main__":
    varrerDados()
