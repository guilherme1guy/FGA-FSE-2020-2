# Generated by Django 3.2.2 on 2021-05-13 14:58

import django.core.validators
from django.db import migrations, models
import django.db.models.deletion


class Migration(migrations.Migration):

    initial = True

    dependencies = [
    ]

    operations = [
        migrations.CreateModel(
            name='Device',
            fields=[
                ('id', models.CharField(max_length=12, primary_key=True, serialize=False, verbose_name='Mac Address')),
                ('is_dht_present', models.BooleanField()),
                ('device_confirmed', models.BooleanField(default=False)),
                ('last_temperature', models.IntegerField(verbose_name='Temperature')),
                ('last_humidity', models.IntegerField(verbose_name='Humidity')),
                ('last_update', models.DateTimeField()),
            ],
        ),
        migrations.CreateModel(
            name='Location',
            fields=[
                ('name', models.CharField(max_length=16, primary_key=True, serialize=False, validators=[django.core.validators.RegexValidator(inverse_match=True, regex='[A-Z]+|\\W+|\\d+')], verbose_name='Location name')),
            ],
        ),
        migrations.CreateModel(
            name='DeviceRegisterRequest',
            fields=[
                ('id', models.CharField(max_length=12, primary_key=True, serialize=False, verbose_name='Mac Address')),
                ('is_dht_present', models.BooleanField()),
                ('io_info', models.JSONField()),
                ('device', models.OneToOneField(blank=True, null=True, on_delete=django.db.models.deletion.CASCADE, to='fse_server.device')),
            ],
        ),
        migrations.AddField(
            model_name='device',
            name='location',
            field=models.ForeignKey(on_delete=django.db.models.deletion.RESTRICT, to='fse_server.location'),
        ),
        migrations.CreateModel(
            name='DeviceOutput',
            fields=[
                ('id', models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('name', models.CharField(max_length=16, verbose_name='Output Name')),
                ('last_state', models.BooleanField(default=False, verbose_name='State')),
                ('gpio_id', models.IntegerField(verbose_name='GPIO Port')),
                ('device', models.ForeignKey(on_delete=django.db.models.deletion.CASCADE, related_name='outputs', to='fse_server.device')),
            ],
            options={
                'unique_together': {('gpio_id', 'device')},
            },
        ),
        migrations.CreateModel(
            name='DeviceInput',
            fields=[
                ('id', models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('name', models.CharField(max_length=16, verbose_name='Input Name')),
                ('last_state', models.BooleanField(default=False, verbose_name='State')),
                ('gpio_id', models.IntegerField(verbose_name='GPIO Port')),
                ('device', models.ForeignKey(on_delete=django.db.models.deletion.CASCADE, related_name='inputs', to='fse_server.device')),
            ],
            options={
                'unique_together': {('gpio_id', 'device')},
            },
        ),
    ]