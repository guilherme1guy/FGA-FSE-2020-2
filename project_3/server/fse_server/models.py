from fse_server import csv_logger, mqtt
from django.db import models
from django.core import validators
from django.db.models.deletion import CASCADE, RESTRICT
from django.utils import timezone
from django.utils import timezone
from django.db import OperationalError


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

    def _get_device_list(self):
        q = self.devices.all()
        all_devices = list(q)
        device_count = q.count()

        return all_devices, device_count

    @property
    def temperature(self):
        temperature = 0

        device_list, device_count = self._get_device_list()
        temperature = sum(list(map(lambda x: x.last_temperature, device_list)))

        return temperature / device_count

    @property
    def humidity(self):
        humidity = 0

        device_list, device_count = self._get_device_list()
        humidity = sum(list(map(lambda x: x.last_humidity, device_list)))

        return humidity / device_count


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

    def _save_sensor_value(self, field, value):

        if value < 0:
            # ignore negative values, since they are an error in
            # the sensor reading
            return

        setattr(self, field, value)
        self.last_update = timezone.now()

        # sometimes mqtt starts before django properly configured
        # the database, we can ignore this error since the device
        # will send new data soon
        try:
            self.save()
        except OperationalError:
            pass  # do nothing

    def set_temperature(self, value):
        self._save_sensor_value("last_temperature", value)

    def set_humidity(self, value):
        self._save_sensor_value("last_humidity", value)


class DeviceInput(models.Model):
    name = models.CharField(verbose_name="Input Name", max_length=16)
    last_state = models.IntegerField(verbose_name="State", default=0)
    gpio_id = models.IntegerField(verbose_name="GPIO Port")

    device = models.ForeignKey(Device, related_name="inputs", on_delete=CASCADE)

    alarm_activated = models.BooleanField(default=False)

    def update_state(self, value):

        # check if an alarm should be raised
        for alarm in self.alarms.all():
            if alarm.should_raise(value):

                if not self.alarm_activated:
                    csv_logger.CsvLogger.log("alarm", f"Alarm active for {str(self)}")

                self.alarm_activated = True
                break

        self.last_state = value
        self.save()

    def __str__(self) -> str:
        return f"{self.device.location.name}/{self.device.id}#gpio_{self.gpio_id}"

    class Meta:
        unique_together = [["gpio_id", "device"]]


class DeviceOutput(models.Model):
    name = models.CharField(verbose_name="Output Name", max_length=16)
    last_state = models.IntegerField(verbose_name="State", default=0)
    gpio_id = models.IntegerField(verbose_name="GPIO Port")

    device = models.ForeignKey(Device, related_name="outputs", on_delete=CASCADE)

    def update_state(self, state):

        if state == True:
            integer_state = 1
        else:
            integer_state = 0

        self.last_state = state
        self.save()

        mqtt.send_gpio_output_to_device(self.device.id, self.gpio_id, integer_state)
        csv_logger.CsvLogger.log("command", f"Set {str(self)} state to {state}")

    def __str__(self) -> str:
        return f"{self.device.location.name}/{self.device.id}#gpio_{self.gpio_id}"

    class Meta:
        unique_together = [["gpio_id", "device"]]


class Alarm(models.Model):
    class ComparsionMode(models.IntegerChoices):
        EQUAL = 0, "EQUAL TO"
        GREATER = 1, "GREATER THAN"
        GREATER_OR_EQUAL = 2, "GREATER OR EQUAL THAN"
        LOWER = 3, "LOWER THAN"
        LOWER_OR_EQUAL = 4, "LOWER OR EQUAL THAN"

        @classmethod
        def operate(cls, a: int, op: int, b: int) -> bool:

            if op == cls.EQUAL:
                return a == b

            if op == cls.GREATER:
                return a > b

            if op == cls.GREATER_OR_EQUAL:
                return a >= b

            if op == cls.LOWER:
                return a < b

            if op == cls.LOWER_OR_EQUAL:
                return a <= b

            raise ValueError("op is invalid")

    target = models.ForeignKey(DeviceInput, on_delete=CASCADE, related_name="alarms")
    target_value = models.IntegerField(verbose_name="Target Value", default=0)
    mode = models.IntegerField(choices=ComparsionMode.choices)

    def should_raise(self, new_value):
        return self.ComparsionMode.operate(new_value, self.mode, self.target_value)

    def __str__(self) -> str:
        return f"gpio_{self.target} is {self.ComparsionMode.labels[self.mode]} {self.target_value}"
