from django.contrib import admin
from fse_server.models import (
    Device,
    DeviceInput,
    DeviceOutput,
    DeviceRegisterRequest,
    Location,
)


admin.site.register(Device)
admin.site.register(DeviceInput)
admin.site.register(DeviceOutput)
admin.site.register(DeviceRegisterRequest)
admin.site.register(Location)
