from datetime import timedelta
from django.db import models
from django.core import validators
from django.db.models.deletion import CASCADE, DO_NOTHING, RESTRICT
from django.utils import timezone


class Location(models.Model):
    name = models.CharField(
        verbose_name="Location name",
        max_length=16,
        primary_key=True,
        validators=[
            # raises validation error when name contains whitespace,
            # digits or upper case characters
            validators.RegexValidator(regex="[A-Z]+|\W+|\d+", inverse_match=True)
        ],
    )

    @property
    def temperature(self):
        temperature = 0

        devices = self.device_set.objects.all()
        temperature = sum(map(lambda x: x.temperature, devices)) / len(devices)

        return temperature

    @property
    def humidity(self):
        humidity = 0

        devices = self.device_set.objects.all()
        humidity = sum(map(lambda x: x.humidity, devices)) / len(devices)

        return humidity


class DeviceRegisterRequest(models.Model):
    id = models.CharField(verbose_name="Mac Address", max_length=12, primary_key=True)
    is_dht_present = models.BooleanField()

    # io_info is a representation of io present on device
    # it will be used to create the mappings for DeviceInput and DeviceOutput
    # example:
    # {
    #   input: [<GPIO_ID>, ...]
    #   output: [<GPIO_ID>, ...]
    # }

    io_info = models.JSONField()

    device = models.OneToOneField("Device", null=True, blank=True, on_delete=CASCADE)

    def __str__(self) -> str:
        return self.id


class Device(models.Model):

    id = models.CharField(verbose_name="Mac Address", max_length=12, primary_key=True)
    is_dht_present = models.BooleanField()

    last_temperature = models.IntegerField(verbose_name="Temperature", default=0)
    last_humidity = models.IntegerField(verbose_name="Humidity", default=0)

    last_update = models.DateTimeField(auto_now=True)

    location = models.ForeignKey(Location, on_delete=RESTRICT, related_name="devices")


class DeviceInput(models.Model):
    name = models.CharField(verbose_name="Input Name", max_length=16)
    last_state = models.BooleanField(verbose_name="State", default=False)
    gpio_id = models.IntegerField(verbose_name="GPIO Port")

    device = models.ForeignKey(Device, related_name="inputs", on_delete=CASCADE)

    class Meta:
        unique_together = [["gpio_id", "device"]]


class DeviceOutput(models.Model):
    name = models.CharField(verbose_name="Output Name", max_length=16)
    last_state = models.BooleanField(verbose_name="State", default=False)
    gpio_id = models.IntegerField(verbose_name="GPIO Port")

    device = models.ForeignKey(Device, related_name="outputs", on_delete=CASCADE)

    class Meta:
        unique_together = [["gpio_id", "device"]]
