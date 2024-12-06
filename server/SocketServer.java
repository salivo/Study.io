import java.io.*;
import java.net.*;

public class SocketServer {

    public static void main(String[] args) {
        int port = 12345; // Define the port the server will listen on

        try (ServerSocket serverSocket = new ServerSocket(port)) {
            System.out.println("Server is listening on port " + port);

            while (true) {
                // Accept a client connection
                Socket socket = serverSocket.accept();
                System.out.println("New client connected");

                // Handle the client in a separate thread
                new ClientHandler(socket).start();
            }
        } catch (IOException ex) {
            System.out.println("Server exception: " + ex.getMessage());
            ex.printStackTrace();
        }
    }
}

class ClientHandler extends Thread {

    private Socket socket;

    public void ProcessDataFromClient(String data) {
        System.out.println("Received: " + data);
    }

    public ClientHandler(Socket socket) {
        this.socket = socket;
    }

    @Override
    public void run() {
        try (
            InputStream input = socket.getInputStream();
            BufferedReader reader = new BufferedReader(
                new InputStreamReader(input)
            );
            OutputStream output = socket.getOutputStream();
            PrintWriter writer = new PrintWriter(output, true)
        ) {
            String text;

            // Read messages from the client
            while ((text = reader.readLine()) != null) {
                ProcessDataFromClient(text);
                // Echo the message back to the client
                writer.println("Server: " + text);

                // Break if the client sends "bye"
                if ("bye".equalsIgnoreCase(text)) {
                    System.out.println("Client disconnected");
                    break;
                }
            }
        } catch (IOException ex) {
            System.out.println("Server exception: " + ex.getMessage());
            ex.printStackTrace();
        } finally {
            try {
                socket.close();
            } catch (IOException ex) {
                System.out.println("Error closing socket: " + ex.getMessage());
            }
        }
    }
}
