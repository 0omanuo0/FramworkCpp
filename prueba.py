import requests
import threading
import time


def post_request(url, data):
	while True:
		try:
			respuesta = requests.post(url, data=data)
			break
		except requests.exceptions.RequestException as e:
			time.sleep(0.1)  # Esperar 1 segundo antes de intentar nuevamente

n = 300
url = "http://localhost:9443/dashboard"
data = {'fname': 'manu', 'fid': '123'}

# Variables para el cálculo de peticiones por segundo
contador = 0
tiempo_inicio = time.time()

for x in range(n):
	t = threading.Thread(target=post_request, args=(url, data))
	t.start()
	contador += 1

# Esperar a que finalicen todas las solicitudes
while threading.active_count() > 1:
	time.sleep(0.1)

tiempo_transcurrido = time.time() - tiempo_inicio
peticiones_por_segundo = contador / tiempo_transcurrido

print("Peticiones por segundo:", peticiones_por_segundo)