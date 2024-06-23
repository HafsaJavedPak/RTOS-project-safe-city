import socket
import sys

def error(msg):
    print(f"Error: {msg}")
    sys.exit(1)

def main():
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} <hostname> <port>")
        sys.exit(1)

    hostname = sys.argv[1]
    portno = int(sys.argv[2])

    try:
        # Initialize socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    except socket.error as err:
        error(f"Error opening socket: {err}")

    try:
        # Resolve hostname
        server_ip = socket.gethostbyname(hostname)
    except socket.error as err:
        error(f"Error, no such host: {err}")

    server_addr = (server_ip, portno)

    try:
        # Connect to server
        sock.connect(server_addr)
    except socket.error as err:
        error(f"Error connecting: {err}")

    while True:
        try:
            # Enter priority
            buffer = input("Enter priority (0-3): ")
            sock.sendall(buffer.encode())

            if buffer.startswith("Bye"):
                break
        except socket.error as err:
            error(f"Error writing to socket: {err}")

    sock.close()

if __name__ == "__main__":
    main()
