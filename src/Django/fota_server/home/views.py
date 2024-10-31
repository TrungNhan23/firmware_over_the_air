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
