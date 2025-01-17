"""fse_server URL Configuration

The `urlpatterns` list routes URLs to views. For more information please see:
    https://docs.djangoproject.com/en/3.0/topics/http/urls/
Examples:
Function views
    1. Add an import:  from my_app import views
    2. Add a URL to urlpatterns:  path('', views.home, name='home')
Class-based views
    1. Add an import:  from other_app.views import Home
    2. Add a URL to urlpatterns:  path('', Home.as_view(), name='home')
Including another URLconf
    1. Import the include() function: from django.urls import include, path
    2. Add a URL to urlpatterns:  path('blog/', include('blog.urls'))
"""
from fse_server.forms import DeviceForm
from django.conf import settings
from django.conf.urls.static import static

from django.contrib import admin
from django.urls import include, path
from rest_framework import routers

from fse_server import views


router = routers.DefaultRouter()
router.register(r"register_request", views.RegisterRequestViewSet)
router.register(r"device", views.DeviceViewSet)
router.register(r"location", views.LocationViewSet)

static_urls = static(settings.STATIC_URL, document_root=settings.STATIC_ROOT)

urlpatterns = [
    path("", views.MainView.as_view(), name="main"),
    path(
        "register/<str:id>",
        views.DeviceFormView.as_view(),
        name="register_device_form",
    ),
    path("device/<str:pk>", views.DeviceDetailView.as_view(), name="device_detail"),
    path(
        "device/~delete/<str:pk>",
        views.DeviceDeleteView.as_view(),
        name="device_delete",
    ),
    path(
        "device/set_state/<str:pk>/<int:state>",
        views.DeviceOutputChangeStateView.as_view(),
        name="device_set_out_state",
    ),
    path("alarms/", views.AlarmStateView.as_view(), name="alarm_state"),
    path("alarms/<pk>", views.AlarmListView.as_view(), name="alarm_list"),
    path("alarms/<pk>/create", views.AlarmFormView.as_view(), name="alarm_create"),
    path("alarms/~delete/<pk>", views.AlarmDeleteView.as_view(), name="alarm_delete"),
    path("admin/", admin.site.urls),
    path("api/", include("rest_framework.urls", namespace="rest_framework")),
    path("api/", include(router.urls)),
] + static_urls
