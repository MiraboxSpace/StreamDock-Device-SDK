import asyncio
from datetime import time
import json
import websockets
from websockets.exceptions import ConnectionClosed

is_send = False


async def client():
    uri = "ws://localhost:9002"
    global is_send
    path = ""
    while True:  
        try:
            print(f"Connecting {uri}...")
            async with websockets.connect(uri) as websocket:
                print("Connected!")

                # Enter the continuous communication loop
                while True:
                    try:
                        # Receive server messages
                        response = await asyncio.wait_for(websocket.recv(), timeout=20)
                        print(f"Received server message: {response}")
                        response = json.loads(response)
                        if response["event"] == "deviceDidConnect":
                            path = response["path"]
                            json_data: dict = {}
                            json_data["event"] = "read"
                            json_data["path"] = path
                            json_data["payload"] = {}
                            await websocket.send(json.dumps(json_data))
                        if not is_send:
                            json_data: dict = {}
                            json_data["event"] = "setKeyImg"
                            json_data["path"] = path
                            json_data["payload"] = {
                                "imagePath": "D:/code/sdks/Websocket-SDK/img/button_test.jpg",
                                "keyIndex": 1,
                            }
                            for i in range(1, 7):
                                json_data["payload"]["keyIndex"] = i
                                await websocket.send(json.dumps(json_data))
                                # await asyncio.sleep(1)

                            json_data["event"] = "getFirmVersion"
                            json_data["payload"] = {}
                            await websocket.send(json.dumps(json_data))

                            json_data["event"] = "setBrightness"
                            json_data["payload"] = {"brightness": 100}
                            await websocket.send(json.dumps(json_data))

                            # json_data["event"] = "setBackgroundImg"
                            # json_data["payload"] = {
                            #     "imagePath": "D:/code/sdks/Websocket-SDK/img/backgroud_test.png",
                            # }
                            # await websocket.send(json.dumps(json_data))

                            json_data["event"] = "setLEDBrightness"
                            json_data["payload"] = {"brightness": 100}
                            await websocket.send(json.dumps(json_data))
                            
                            json_data["event"] = "setLEDColor"
                            json_data["payload"] = {"r": 255, "g": 0, "b": 0}
                            await websocket.send(json.dumps(json_data))
                            
                            
                            # K1 Pro
                            # json_data["event"] = "setKeyboardLightingEffects"
                            # json_data["payload"] = {"effect": 2}
                            # await websocket.send(json.dumps(json_data))
                            
                            
                            # json_data["event"] = "setKeyboardLightingSpeed"
                            # json_data["speed"] = {"effect": 6}
                            # await websocket.send(json.dumps(json_data))
                            
                            # # json_data["event"] = "setKeyboardRGBBacklight"
                            # # json_data["payload"] ={"r": 255, "g": 0, "b": 0}
                            # # await websocket.send(json.dumps(json_data))
                            
                            # json_data["event"] = "setKeyboardBacklightBrightness"
                            # json_data["payload"] ={"brightness": 6}
                            # await websocket.send(json.dumps(json_data))
                            
                            is_send = True
                            await asyncio.sleep(1)
                        # await websocket.send("ping")

                    except asyncio.TimeoutError:
                        # Timeout handling, can send a heartbeat packet
                        print("Receive timeout, trying to send heartbeat...")
                        await websocket.ping()

        except (ConnectionClosed, OSError) as e:
            print(f"Connection closed ({e}), will try to reconnect in 5 seconds...")
            await asyncio.sleep(5)
        except Exception as e:
            print(f"Unknown error occurred: {e}")
            break


if __name__ == "__main__":
    try:
        asyncio.run(client())
    except KeyboardInterrupt:
        print("Client stopped by user")
