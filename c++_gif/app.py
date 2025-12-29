from selenium import webdriver
from selenium.webdriver.common.by import By
import os
import glob
import time
from pathlib import Path # Importe Path

def varrerDados():
	try:
		#options = webdriver.ChromeOptions()
		#options.add_argument("--headless=new")
		#driver = webdriver.Chrome(options=options)
		driver = webdriver.Firefox()
		driver.get("https://notisrac.github.io/FileToCArray/")
		driver.maximize_window() #ABRE COM A JENALA FULL
		driver.implicitly_wait(5)
		# SELEÇÃO DE IMAGENS

		# --- Parte para encontrar os arquivos GIF ---
		# Define o diretório onde suas imagens estão. '.' significa o diretório atual.
		diretorio_imagens = './bloggif_frames_gif' 
		total_img = len(os.listdir(diretorio_imagens))
		
		print(f'Total Imagens: {total_img}')
		# Encontra todos os arquivos .gif e .GIF no diretório
		padrao = os.path.join(diretorio_imagens, '*.gif')
		lista_gifs = glob.glob(padrao)
		lista_gifs.extend(glob.glob(os.path.join(diretorio_imagens, '*.GIF')))

		if not lista_gifs:
			print("Nenhum arquivo GIF encontrado para upload.")
			return # Sai da função se não houver arquivos

		for x in range(total_img):
			# Pega o primeiro GIF encontrado para o exemplo de upload
			primeiro_gif = lista_gifs[x] 
			print(f"Tentando fazer upload do arquivo: {primeiro_gif}")

			# --- Parte Selenium para upload ---
			# Localiza o elemento de input de arquivo
			file_input_element = driver.find_element(By.XPATH, "//input[@id='inFileInput']")

			# Envia o caminho completo do arquivo para o elemento de input (upload)
			file_input_element.send_keys(os.path.abspath(primeiro_gif))

			print("Arquivo enviado com sucesso para o campo de upload.")
			# MENU
			driver.find_element(By.XPATH, "//option[@value='DEC']").click() #Code format
			#driver.find_element(By.XPATH, "//option[@value=1]").click() #Code format
			driver.find_element(By.XPATH, "//input[@id='txtResizeX']").clear() # Boa prática limpar antes
			driver.find_element(By.XPATH, "//input[@id='txtResizeX']").send_keys("128")
			driver.find_element(By.XPATH, "//input[@id='txtResizeY']").clear()
			driver.find_element(By.XPATH, "//input[@id='txtResizeY']").send_keys("64")
			driver.find_element(By.XPATH, "//*[@id='cbPaletteMod']/option[8]").click() #Palette mod
			print("Valores X e Y inseridos.")
			# --- MODIFICAÇÃO AQUI ---
			checkbox_unsigned = driver.find_element(By.XPATH, "//input[@id='cbUnsigned']") # unsigned
			if not checkbox_unsigned.is_selected():
				checkbox_unsigned.click() # unsigned (marca se ainda não estiver marcado)
			# -----------------------	
			driver.find_element(By.XPATH, "//option[@value='char']").click() #options
			driver.find_element(By.XPATH, "//input[@id='btnConvert']").click() #options
			time.sleep(2)
			textarea_element = driver.find_element(By.XPATH, "//textarea[@id='txtResult']") #click
			#driver.find_element(By.XPATH, "//input[@id='btnSaveFile']").click() #options

			# Obtém o valor (value) do elemento
			conteudo_h = textarea_element.get_attribute('value')
			# 1. Defina e crie a pasta de destino para os arquivos .h
			pasta_destino = Path('./frame')
			pasta_destino.mkdir(exist_ok=True) # Cria a pasta se ela não existir
			# Define o nome do arquivo de saída (ex: frame_0.h, frame_1.h)
			output_filename = f"frame_{x + 1:03d}.h"
			output_filepath = pasta_destino / output_filename

			# Abre o arquivo em modo de escrita ('w') com codificação UTF-8 e escreve o conteúdo
			with open(output_filepath, 'w', encoding='utf-8') as f:
				f.write(conteudo_h)
				pass

			print(f"Conteúdo do frame {x + 1:03d} salvo em {output_filepath}\n")

			time.sleep(2)	
	except:
		print(f'Sem acesso ao Site')
	finally:
		driver.quit()
		pass
varrerDados()