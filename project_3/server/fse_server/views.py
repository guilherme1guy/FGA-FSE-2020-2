from django.forms.forms import Form
from django.views.generic import TemplateView, FormView, DetailView, DeleteView
from django.views.generic.base import View
from django.urls import reverse_lazy
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

    def delete(self, request, *args: str, **kwargs):

        # get object location
        location = self.get_object().location

        r = super().delete(request, *args, **kwargs)

        # remove location if empty
        if location.device_set.count() < 1:
            location.delete()

        return r


class RegisterRequestViewSet(viewsets.ModelViewSet):

    queryset = models.DeviceRegisterRequest.objects.filter(device=None)
    serializer_class = serializers.RegisterRequestSerializer


class DeviceViewSet(viewsets.ModelViewSet):

    queryset = models.Device.objects.all()
    serializer_class = serializers.DeviceSerializer


class LocationViewSet(viewsets.ModelViewSet):

    queryset = models.Location.objects.all()
    serializer_class = serializers.LocationSerializer
