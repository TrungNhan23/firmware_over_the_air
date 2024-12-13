import asyncio
import websockets
import binascii
from io import BytesIO
import os

from PIL import Image, UnidentifiedImageError

stop_flag = False
esp32_websocket = None

frame_path = r"C:\Project_UTE\DA2\src\ESP32-CAM\esp32_camera_webstream\images\image.jpg"

def is_valid_image(image_bytes):
    try:
        Image.open(BytesIO(image_bytes))
        # print("image OK")
        return True
    except UnidentifiedImageError:
        print("image invalid")
        return False


async def handle_connection(websocket):
    global stop_flag,esp32_websocket,frame_path
    while True:
        try:
            message = await websocket.recv()
            print(len(message))
            if isinstance(message,str) and "ESP32-CAM" in message:
                 esp32_websocket = websocket
                 print(f"{message} connected!")
            elif isinstance(message,bytes) and len(message) > 5000 and stop_flag == False:
                if is_valid_image(message):
                    #print(message)
                    print("enter valid")
                    with open(frame_path, "wb") as f:
                            f.write(message)
                else:
                    print("error")
            elif isinstance(message,str):
                if esp32_websocket:
                    if message == "START":
                        stop_flag = False
                    elif message == "STOP":
                        stop_flag = True
                    await esp32_websocket.send(message)
                    print(f"Command:{message} sent to esp32-cam")
                    message = ""
            # else:
            #     print("Waititng for response from esp32-cam")
        except websockets.exceptions.ConnectionClosed:
            print("Connection break!")
            break

# Function to send START/STOP commands to the ESP32-CAM
async def handle_on_off(command):
    esp32_cam_address = "ws://<ESP32-CAM-IP>:<PORT>"  # Replace with your ESP32-CAM WebSocket IP and Port

    try:
        async with websockets.connect(esp32_cam_address) as websocket:
            await websocket.send(command)
            print(f"Command '{command}' sent to ESP32-CAM.")
    except Exception as e:
        print(f"Failed to send command to ESP32-CAM: {e}")

async def main():
    # Get the absolute path of the folder where the script is located
    script_directory = os.path.dirname(os.path.abspath(__file__))

    image_directory = os.path.dirname(script_directory) + "//images"

    # Set the working directory to that folder
    os.chdir(image_directory)
    server = await websockets.serve(handle_connection, '0.0.0.0', 3001)
    await server.wait_closed()

asyncio.run(main())
