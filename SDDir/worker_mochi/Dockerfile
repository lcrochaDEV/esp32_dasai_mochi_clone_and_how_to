FROM python:3.9-slim

WORKDIR . /app

COPY requirements.txt ./

RUN pip install --no-cache-dir -r requirements.txt
RUN python3 -m venv .venv
ENV PATH="/app/.venv/bin:$PATH"

#RUN git clone https://github.com/lcrochaDEV/web-intra-python

#transfere todos os arquivos em diretórios
COPY . .

#transfere um unico arquivo do projeto
#COPY app.py . 

CMD uvicorn app:app

#EXPOSE 8000

CMD ["uvicorn", "app:app", "--host", "0.0.0.0", "--port", "8000"]


#INSTALAÇÃO python, fastapi, uvicorn

#COMMAND
#criando imagen (Building the Docker Image)
#sudo docker build -t fastapi .

#expondo portas (Running the Docker Container)
#sudo docker run -d -p 8000:8000 fastapi

#verifica conteiners criados
#sudo docker ps -as

#deletando imagens
#sudo docker rm d7f83ebfcc62
#sudo docker rmi fastapi

#craindo conteiners com docker-compose.yaml ou compose.yaml
#sudo docker-compose up -d

#remover todos os conteiners
#docker-compose down

#acessando o conteiner
#sudo docker exec -it <contener-name> /bin/bash

#verifica dado(IP) do conteiner "tupologia de rede"
#docker network inspect <conteiner>

#docker compose logs -f <conteiner>

#docker exec -it <nome_ou_id_do_container> /bin/bash