import socket
from playsound import playsound

HOST = "0.0.0.0"
PORT = 1337

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    while True:
        conn, addr = s.accept()
        with conn:
            while True:
                data = conn.recv(1024)
                if not data:
                    break
                if (data.decode().strip() == "scream"):
                    print("Scream")
                    playsound('./play.mp3')

