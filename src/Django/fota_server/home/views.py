from django.http import JsonResponse, StreamingHttpResponse
from django.shortcuts import render,redirect,get_object_or_404
from rest_framework.views import APIView
from rest_framework.response import Response
from rest_framework import status
from drf_yasg.utils import swagger_auto_schema
from drf_yasg import openapi
from django.contrib.auth.decorators import login_required,permission_required
from django.contrib.auth import login,authenticate
from django.utils.decorators import method_decorator
from django.contrib import messages
import requests
from json import dumps 
import json
from datetime import datetime, timezone
from django.db import models
from django.views.decorators.csrf import csrf_exempt
from django.core.files.storage import FileSystemStorage
from django.conf import settings
from django.http import HttpResponse
from .models import HexFile,UserID
from django.views.generic import TemplateView
import socket
from PIL import Image
from io import BytesIO
import os
import asyncio
import time
import websockets
import hashlib

# Create your views here.

@method_decorator(login_required,name="dispatch")
class homepage(APIView):
# Create your views here.

    @swagger_auto_schema(
        manual_parameters=[
            openapi.Parameter('message', openapi.IN_QUERY, description="Message to be echoed back", type=openapi.TYPE_STRING),
            openapi.Parameter('name', openapi.IN_QUERY, description="Name of the sender", type=openapi.TYPE_STRING),
        ],
        operation_summary="Get a simple hello message with parameters."
    )
    def get(self,request):
        """
        Get home data info
        """
        if self.request.user.is_superuser:
            return render(request,"hw_selection.html")
        else:
            return render(request,"hw_selection.html")
@method_decorator(login_required,name="dispatch")
class fota_page(APIView):
# Create your views here.
    @swagger_auto_schema(
        manual_parameters=[
            openapi.Parameter('message', openapi.IN_QUERY, description="Message to be echoed back", type=openapi.TYPE_STRING),
            openapi.Parameter('name', openapi.IN_QUERY, description="Name of the sender", type=openapi.TYPE_STRING),
        ],
        operation_summary="Get a simple hello message with parameters."
    )
    def get(self,request):
        """
        Get home data info
        """
        user_name = request.user
        return render(request,"home_admin.html",{'user_name':user_name})
        # else:
        #     return redirect('fotapage')

@method_decorator(login_required,name="dispatch")
class upload_hex_file(APIView):
    @swagger_auto_schema(
        manual_parameters=[
            openapi.Parameter('hex_file', openapi.IN_QUERY, description="the hex file", type=openapi.TYPE_STRING),
        ],
        operation_summary="Get execution plan view with parameters."
    )
    def post(self,request):
        if request.FILES.get("hex_file"):
            hex_file = request.FILES["hex_file"]
            user_id = request.user

            file_content = hex_file.read()
        
        #Save the user directly to the database through UserID model
            user, created  = UserID.objects.get_or_create(user_id=user_id)

        # Save the file content directly to the database
            uploaded_file = HexFile.objects.create(
                file_name=hex_file.name,
                file_data=file_content,
                user_id= user
            )
        
            # Save file using Django's default storage
            fs = FileSystemStorage(location=settings.MEDIA_ROOT)
            filename = fs.save(hex_file.name, hex_file)
            file_url = fs.url(filename)
            
            return JsonResponse({"file_url": file_url, "filename": hex_file.name})
        return JsonResponse({"error": "Invalid request"}, status=400)


@method_decorator(login_required,name="dispatch")
class download_hex_file(APIView):
    @swagger_auto_schema(
        manual_parameters=[
            openapi.Parameter('hex_file', openapi.IN_QUERY, description="hex file", type=openapi.TYPE_STRING),
        ],
        operation_summary="Get execution plan view with parameters."
    )
    def get(self,request):
        try:
            user = request.user
            user_id = get_object_or_404(UserID, user_id=user)
            uploaded_files = HexFile.objects.filter(user_id=user_id).order_by('-uploaded_at')[:5]
            if uploaded_files.exists():
                files_data = [{'file_name': file.file_name,'file_url': file.id} for file in uploaded_files]
                if len(files_data) < 5:
                    number_none = 5 - len(files_data)
                    for none_file in range(0,number_none):
                        files_data.append({'file_name': "",'file_url': None})
                print(files_data)
                return JsonResponse({'files': files_data})
            return None
        except HexFile.DoesNotExist:
            return HttpResponse("File not found", status=404)


@method_decorator(login_required,name="dispatch")
class select_hardware(APIView):
    @swagger_auto_schema(
        manual_parameters=[
            openapi.Parameter('hardware_ip', openapi.IN_QUERY, description="The ip of hardware", type=openapi.TYPE_STRING),
        ],
        operation_summary="Get execution plan view with parameters."
    )
    def get(self,request):
        return redirect("fota_page")
        # print("reached here")
        # return render(request,"home_admin.html")

@method_decorator(login_required,name="dispatch")
class pump_message(APIView):
    @swagger_auto_schema(
        manual_parameters=[
            openapi.Parameter('message', openapi.IN_QUERY, description="The message you want to pump", type=openapi.TYPE_STRING),
            openapi.Parameter('type', openapi.IN_QUERY, description="The message type you want to pump", type=openapi.TYPE_STRING),
        ],
        operation_summary="Get execution plan view with parameters."
    )
    def get(self,request):
        message_text = request.GET.get('message')
        message_type = request.GET.get('type')
        print(message_text)
        print(message_type)
        if message_type == "success":
            messages.success(request, message_text)
        elif message_type == "error":
            messages.error(request,message_text)
        elif message_type == "warning":
            messages.warning(request,message_text)
        return JsonResponse({'files': ""})

@method_decorator(login_required,name="dispatch")
class stream_video(APIView):
    @swagger_auto_schema(
        manual_parameters=[
            openapi.Parameter('message', openapi.IN_QUERY, description="The message you want to pump", type=openapi.TYPE_STRING),
            openapi.Parameter('type', openapi.IN_QUERY, description="The message type you want to pump", type=openapi.TYPE_STRING),
        ],
        operation_summary="Get execution plan view with parameters."
    )
    def calculate_image_hash(self,image_bytes):
        """Calculate a hash of the image bytes using SHA256."""
        return hashlib.sha256(image_bytes).hexdigest()

    def get(self,request):
        def get_image():
            frame_path = r"C:\Project_UTE\DA2\src\ESP32-CAM\esp32_camera_webstream\images\image.jpg"
            frame_close_path = r"C:\Project_UTE\DA2\src\ESP32-CAM\esp32_camera_webstream\python\placeholder.jpg"
            previous_hash = None
            while True:
                try:
                    if os.path.exists(frame_path) and os.path.getsize(frame_path) > 5000 and os.path.exists(frame_close_path):
                        with open(frame_path, "rb") as f:
                            image_bytes = f.read()
                        
                        # Validate the image
                        try:
                            image = Image.open(BytesIO(image_bytes))
                            image.verify()  # Check if it is a valid image
                            image = Image.open(BytesIO(image_bytes))  # Reopen to load after verify
                        except Exception as e:
                            print(f"Invalid image data: {e}")
                            continue  # Skip the invalid frame

                        current_hash = self.calculate_image_hash(image_bytes) #Get the hash of the current frame
                        # print(f"Current hash:{current_hash} and {previous_hash}")
                        if current_hash != previous_hash: #Check if the frame has been changed -> Display to the web
                            previous_hash = current_hash

                            # Convert to bytes and yield
                            img_io = BytesIO()
                            image.save(img_io, 'JPEG')
                            img_io.seek(0)
                            img_bytes = img_io.read()
                            yield (b'--frame\r\n'
                                b'Content-Type: image/jpeg\r\n\r\n' + img_bytes + b'\r\n')
                        else:
                            with open(frame_close_path, "rb") as f:
                                image_bytes = f.read()
                                 # Validate the image
                                try:
                                    image = Image.open(BytesIO(image_bytes))
                                    image.verify()  # Check if it is a valid image
                                    image = Image.open(BytesIO(image_bytes))  # Reopen to load after verify
                                except Exception as e:
                                    print(f"Invalid image data: {e}")
                                    continue  # Skip the invalid frame
                                img_io = BytesIO()
                                image.save(img_io, 'JPEG')
                                img_io.seek(0)
                                img_bytes = img_io.read()
                                yield (b'--frame\r\n'
                                    b'Content-Type: image/jpeg\r\n\r\n' + img_bytes + b'\r\n')
                    else:
                        print("Frame file is not valid yet or does not exist!")

                except Exception as e:
                    print("encountered an exception: ")
                    print(e)
                finally:
                    # Add a small delay to avoid rapid looping
                    time.sleep(0.4)

        return StreamingHttpResponse(
            get_image(),
            content_type="multipart/x-mixed-replace; boundary=frame"
        )

@method_decorator(login_required,name="dispatch")
class toggle_camera(APIView):
    @swagger_auto_schema(
        manual_parameters=[
            openapi.Parameter('action', openapi.IN_QUERY, description="The action (START/STOP) to the esp32-cam", type=openapi.TYPE_STRING),
        ],
        operation_summary="Get execution plan view with parameters."
    )
    def post(self,request):
        rawdata = request.data.dict()
        action = rawdata["action"]
        try:
            print(f"Action is:{action}")
            asyncio.run(self.send_message_to_websocket(action))
            return JsonResponse({"success":True,"message":f"Camera {action} sent!"})
        except Exception as e:
            return JsonResponse({"success":False,"message":f"Error:{e}"})
        
    
    async def send_message_to_websocket(self,message):
        ## getting the hostname by socket.gethostname() method
        hostname = socket.gethostname()
        ## getting the IP address using socket.gethostbyname() method
        ip_address = socket.gethostbyname(hostname)
        websocket_url = f"ws://{ip_address}:3001" #replace websocket url
        try:
            async with websockets.connect(websocket_url) as websocket:
                await websocket.send(message)
                return True
        except Exception as e:
            print(f"Failed to send message to websocket:{e}")
            return False
            

def download_file(request,file_id):
    file = get_object_or_404(HexFile, id=file_id) 
    response = HttpResponse(file.file_data, content_type='application/octet-stream') 
    response['Content-Disposition'] = f'attachment; filename="{file.file_name}"' 
    return response




def upload_hex(request):
    if request.method == "POST" and request.FILES.get("hex_file"):
        hex_file = request.FILES["hex_file"]
        
        # Save file using Django's default storage
        fs = FileSystemStorage(location=settings.MEDIA_ROOT)
        filename = fs.save(hex_file.name, hex_file)
        file_url = fs.url(filename)
        
        return render(request, "upload.html", {"file_url": file_url, "filename": hex_file.name})
    return render(request, "upload.html")

