from Emailer.verification_email import Email
import socket

def start_emailer_server(host, port):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
        server_socket.bind((host, port))
        server_socket.listen()

        print(f"Server listening on {host}:{port}")

        while True:
            client_socket, client_address = server_socket.accept()
            with client_socket:
                print(f"Incoming connection request from {client_address} has been established.")

                while True:
                    data = client_socket.recv(1024)
                    if not data:
                        break
                    body = data.decode().split(" ")
                    print("email address : " + body[0])
                    print("verification code : " + body[1])

                    email = Email(body[0], "smtp.sendgrid.net", 587)
                    email.send_verif_code(body[1])
                    print(f"Successfully sent verification code to email address {email.dest}")

if __name__ == '__main__':
    start_emailer_server("localhost", 4200)
