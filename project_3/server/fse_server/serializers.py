from rest_framework import serializers
from fse_server import models


class RegisterRequestSerializer(serializers.ModelSerializer):
    class Meta:
        model = models.DeviceRegisterRequest
        fields = "__all__"


class DeviceSerializer(serializers.ModelSerializer):
    class Meta:
        model = models.Device
        fields = "__all__"


class LocationSerializer(serializers.ModelSerializer):

    devices = serializers.PrimaryKeyRelatedField(many=True, read_only=True)

    class Meta:
        model = models.Location
        fields = "__all__"
