import asyncio
import time
import websockets
import threading
import json
import uuid

speed = 10
# Shared data structures for storing players' state and movement commands
players = {}
playersmove = {}
connected_clients = set()  # Track all connected WebSocket clients
players_lock = threading.Lock()  # Lock for thread-safe dictionary access


def calculatepos():
    """
    A background thread that updates positions of all players at regular intervals.
    This thread modifies player positions based on their current movement direction.
    """
    while True:
        time.sleep(0.033)  # ~30 FPS
        with players_lock:
            for id in players:
                # Calculate new position based on movement
                new_x = players[id]['x'] + speed * playersmove.get(id, [0, 0])[0]
                new_y = players[id]['y'] + speed * playersmove.get(id, [0, 0])[1]

                # Only apply position changes if they are different
                if new_x != players[id]['x'] or new_y != players[id]['y']:
                    players[id]['x'] = new_x
                    players[id]['y'] = new_y


async def broadcast(message):
    """
    Sends a message to all connected clients.
    """
    with players_lock:
        websockets_to_remove = []
        for client in connected_clients:
            try:
                await client.send(message)
            except Exception as e:
                print(f"Failed to send message to a client: {e}")
                websockets_to_remove.append(client)
        for ws in websockets_to_remove:
            connected_clients.remove(ws)


async def handle_client(websocket):
    """
    Handles a single WebSocket client's communication.
    Assigns a unique ID, receives movement commands, and manages disconnects.
    """
    # Assign a unique player ID
    player_id = int(uuid.uuid4().int % 1e3)
    welcome_message = json.dumps({"type": "welcome", "player_id": player_id})
    await websocket.send(welcome_message)
    print(f"Player {player_id} connected")
    # Register the player with initial starting position
    with players_lock:
        players[player_id] = {"x": 100, "y": 100}
        playersmove[player_id] = [0, 0]
        connected_clients.add(websocket)

    try:
        # Task to send position updates
        send_task = asyncio.create_task(send_positions(websocket, player_id))

        # Wait for client messages
        async for message in websocket:
            data = json.loads(message)
            with players_lock:
                # Update movement based on the received data
                playersmove[player_id] = [
                    -data.get('left', 0) + data.get('right', 0),
                    -data.get('up', 0) + data.get('down', 0),
                ]
    except websockets.exceptions.ConnectionClosed as e:
        print(f"Connection closed for player {player_id}: {e}")
    finally:
        # Notify all clients about the disconnection
        disconnect_message = json.dumps({"type": "disconnect", "player_id": player_id})
        await broadcast(disconnect_message)

        # Cancel sending task and clean up
        send_task.cancel()
        with players_lock:
            del players[player_id]
            del playersmove[player_id]
            if websocket in connected_clients:
                connected_clients.remove(websocket)
        print(f"Player {player_id} disconnected")


async def send_positions(websocket, player_id):
    """
    Sends data only when any player's position changes.
    Avoids sending redundant data.
    """
    last_sent_state = {}  # Keep track of the last state sent to the client
    try:
        while True:
            await asyncio.sleep(0.033)  # Throttle the sending rate
            with players_lock:
                # Compare the current state with last sent state
                if players != last_sent_state:
                    # Send only when there is a change
                    data = {"type": "positions", "players": players}
                    try:
                        await websocket.send(json.dumps(data))
                        last_sent_state = {player_id: {"x": players[player_id]['x'], "y": players[player_id]['y']} for player_id in players}
                    except Exception as e:
                        print(f"Failed to send data to player: {e}")
    except asyncio.CancelledError:
        print("Sending task cancelled")


async def main():
    """
    Main function that starts the WebSocket server and threads.
    """
    # Start the position calculation thread
    thread = threading.Thread(target=calculatepos, daemon=True)
    thread.start()

    # Start the WebSocket server
    async with websockets.serve(handle_client, "0.0.0.0", 8765):
        print("WebSocket server running on ws://localhost:8765")
        await asyncio.Future()  # Keep the server alive


# Run the server
if __name__ == "__main__":
    asyncio.run(main())
