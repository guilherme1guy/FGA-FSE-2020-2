import json
from os import name

from django import forms
from django.core.validators import MaxLengthValidator

from fse_server.models import Device, DeviceRegisterRequest, Location


class DeviceForm(forms.ModelForm):

    location = forms.CharField(
        max_length=16, help_text="If the location is not registered it will be created"
    )

    def __init__(self, *args, **kwargs):

        register_request_id = kwargs.pop("id")
        self.register_request = DeviceRegisterRequest.objects.filter(
            id=register_request_id
        ).first()

        super(DeviceForm, self).__init__(*args, **kwargs)

        self.fields["id"].initial = self.register_request.id
        self.fields["id"].disabled = True

        self.fields["is_dht_present"].initial = self.register_request.is_dht_present
        self.fields["is_dht_present"].disabled = True
        self.fields["is_dht_present"].widget = forms.HiddenInput()

        io_info = json.loads(self.register_request.io_info)
        for input in io_info["input"]:
            self.fields[f"io_input_{input}"] = forms.CharField(
                max_length=16,
                help_text="Insert a name for this Input",
                label=f"Input #{input}",
            )
            self.fields[f"io_input_{input}"].required = True

        for output in io_info["output"]:
            self.fields[f"io_output_{output}"] = forms.CharField(
                max_length=16,
                help_text="Insert a name for this Output",
                label=f"Output #{output}",
            )
            self.fields[f"io_output_{output}"].required = True

    def clean_location(self):
        location = self.cleaned_data["location"]

        search = Location.objects.filter(name=location)

        if search.count() < 1:
            location = Location(name=location)
            location.save()
        else:
            location = search.first()

        return location

    class Meta:
        model = Device
        fields = ["id", "is_dht_present", "location"]
