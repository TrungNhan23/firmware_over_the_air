from django.http import JsonResponse
from django.shortcuts import render,redirect,get_object_or_404
from rest_framework.views import APIView
from rest_framework.response import Response
from rest_framework import status
from drf_yasg.utils import swagger_auto_schema
from drf_yasg import openapi
from django.contrib.auth.decorators import login_required,permission_required
from django.contrib.auth import login,authenticate
from django.utils.decorators import method_decorator
import requests
from json import dumps 
import json
from datetime import datetime, timezone
from django.db import models
from django.views.decorators.csrf import csrf_exempt
from django.core.files.storage import FileSystemStorage
from django.conf import settings

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
            return render(request,"home_admin.html")
        else:
            return render(request,"home_admin.html")

@method_decorator(login_required,name="dispatch")
class upload_hex_file(APIView):
    @swagger_auto_schema(
        manual_parameters=[
            openapi.Parameter('project_name', openapi.IN_QUERY, description="Project name", type=openapi.TYPE_STRING),
            openapi.Parameter('PC_setup', openapi.IN_QUERY, description="Name PC setup", type=openapi.TYPE_STRING),
        ],
        operation_summary="Get execution plan view with parameters."
    )
    def get(self,request):
        """
        Get execution plan view info
        """
        project = request.GET.get('project')
        PCsetup = request.GET.get('PCsetup')
        data = {
            'project' : str(project),
            'PCsetup' : str(PCsetup),
            'username': str(request.user)
        }
        dataJSON = dumps(data) 
        # print(project, PCsetup)
        return render(request,"execution_plan.html", {'data': dataJSON})
    

def upload_hex(request):
    if request.method == "POST" and request.FILES.get("hex_file"):
        hex_file = request.FILES["hex_file"]
        
        # Save file using Django's default storage
        fs = FileSystemStorage(location=settings.MEDIA_ROOT)
        filename = fs.save(hex_file.name, hex_file)
        file_url = fs.url(filename)
        
        return render(request, "upload.html", {"file_url": file_url, "filename": hex_file.name})
    return render(request, "upload.html")
