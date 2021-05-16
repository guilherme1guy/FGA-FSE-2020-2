from django.urls.base import resolve
from django.views.generic import (
    TemplateView,
    FormView,
    DetailView,
    DeleteView,
    View,
    ListView,
)
from django.views.generic.base import View
from django.urls import reverse_lazy
from django.http import JsonResponse
from django.db.models import Count
from rest_framework import viewsets

from fse_server import models
from fse_server import serializers
from fse_server import forms


class MainView(TemplateView):
    template_name = "home.html"


class FormViewWithKwargs(FormView):
    def get_form_kwargs(self):
        kwargs = super().get_form_kwargs()
        kwargs.update(self.kwargs)

        return kwargs


class DeviceFormView(FormViewWithKwargs):
    form_class = forms.DeviceForm
    template_name = "form.html"
    success_url = reverse_lazy("main")

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


class AlarmListView(ListView):

    model = models.Alarm
    template_name = "alarm_list.html"

    def get_queryset(self):

        devInput_pk = self.kwargs["pk"]

        return models.Alarm.objects.filter(target__pk=devInput_pk)

    def get_context_data(self, **kwargs):
        context = super().get_context_data(**kwargs)

        context["input"] = models.DeviceInput.objects.filter(
            pk=self.kwargs["pk"]
        ).first()

        return context


class AlarmFormView(FormViewWithKwargs):

    form_class = forms.AlarmForm
    template_name = "form.html"

    def get_success_url(self):
        return reverse_lazy("alarm_list", kwargs={"pk": self.kwargs["pk"]})

    def form_valid(self, form):
        form.save()
        return super().form_valid(form)


class AlarmStateView(View):
    def get(self, request, *args, **kwargs):

        alarm_activated_objects = models.DeviceInput.objects.filter(
            alarm_activated=True
        )

        if alarm_activated_objects.count() == 0:
            return JsonResponse({"alarm_active": False})

        response = {"alarm_active": True, "alarmed_devices": []}
        for deviceInput in alarm_activated_objects:
            response["alarmed_devices"].append(
                f"{deviceInput.device.location.name}/{deviceInput.device.id}#gpio_{deviceInput.gpio_id}"
            )

        return JsonResponse(response)

    def post(self, request, *args, **kwargs):
        # disable all alarms
        q = models.DeviceInput.objects.filter(alarm_activated=True)
        for deviceInput in q:
            deviceInput.alarm_activated = False
            deviceInput.save()

        return JsonResponse({"ok": True})


class AlarmDeleteView(DeleteView):
    model = models.Alarm
    success_url = reverse_lazy("")
    template_name = "delete.html"

    def get_success_url(self) -> str:
        return reverse_lazy("alarm_list", kwargs={"pk": self.object.target.pk})
