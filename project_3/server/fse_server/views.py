from django.forms.forms import Form
from django.views.generic import TemplateView, FormView, DetailView, DeleteView
from django.views.generic.base import View
from django.urls import reverse_lazy
from django.db.models import Count
from rest_framework import viewsets

from fse_server import models
from fse_server import serializers
from fse_server import forms


class MainView(TemplateView):
    template_name = "home.html"


class DeviceFormView(FormView):
    form_class = forms.DeviceForm
    template_name = "form.html"
    success_url = reverse_lazy("main")

    def get_form_kwargs(self):
        kwargs = super().get_form_kwargs()
        kwargs.update(self.kwargs)

        return kwargs

    def form_valid(self, form):

        instance = form.save()

        register_request = models.DeviceRegisterRequest.objects.filter(
            id=instance.id
        ).first()
        register_request.device = instance
        register_request.save()

        INPUT_PREFIX = "io_input_"
        OUTPUT_PREFIX = "io_output_"

        for key, value in form.cleaned_data.items():

            if key.startswith(INPUT_PREFIX):

                models.DeviceInput(
                    name=value, gpio_id=key.replace(INPUT_PREFIX, ""), device=instance
                ).save()

            elif key.startswith(OUTPUT_PREFIX):

                models.DeviceOutput(
                    name=value, gpio_id=key.replace(OUTPUT_PREFIX, ""), device=instance
                ).save()

        return super().form_valid(form)


class DeviceDetailView(DetailView):
    model = models.Device
    template_name = "device_detail.html"


class DeviceDeleteView(DeleteView):
    model = models.Device
    success_url = reverse_lazy("main")
    template_name = "delete.html"

    def delete(self, request, *args: str, **kwargs):

        response = super().delete(request, *args, **kwargs)

        # remove location if empty
        location_query = models.Location.objects.annotate(
            device_count=Count("devices")
        ).filter(device_count=0)

        for location in location_query:
            location.delete()

        return response


class RegisterRequestViewSet(viewsets.ModelViewSet):

    queryset = models.DeviceRegisterRequest.objects.filter(device=None).order_by("id")
    serializer_class = serializers.RegisterRequestSerializer


class DeviceViewSet(viewsets.ModelViewSet):

    queryset = models.Device.objects.all().order_by("id")
    serializer_class = serializers.DeviceSerializer


class LocationViewSet(viewsets.ModelViewSet):

    queryset = models.Location.objects.all().order_by("name")
    serializer_class = serializers.LocationSerializer
