from django.urls import path,include
from django.contrib import admin
from . import views

urlpatterns = [
    path('', views.homepage.as_view(), name='home'),
    path("upload/", views.upload_hex, name="upload_hex"),
]