import asyncio
import websockets
import json

# Handle WebSocket server communication
async def socketwork(websocket):
    try:
        # Create a loop for sending data
        send_task = asyncio.create_task(send_positions(websocket))

        # Listen for incoming messages from the client
        async for message in websocket:
            print(f"Received message from client: {message}")

    except websockets.exceptions.ConnectionClosed as e:
        print("Connection closed", e)
    finally:
        # Cancel the sending task if the connection is closed
        send_task.cancel()


# Function for sending position data
async def send_positions(websocket):
    try:
        while True:
            for i in range(10):
                position = {"x": i * 10, "y": i * 10}  # Example movement logic
                await websocket.send(json.dumps(position))  # Send data as JSON string
                await asyncio.sleep(0.033)  # Send data at ~30 FPS
    except asyncio.CancelledError:
        print("Sending task cancelled")


# Main server loop
async def main():
    async with websockets.serve(socketwork, "localhost", 8765):
        print("WebSocket server is running on ws://localhost:8765")
        await asyncio.Future()  # Run server forever


# Run the server
asyncio.run(main())
