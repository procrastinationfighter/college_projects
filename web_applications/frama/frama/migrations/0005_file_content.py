# Generated by Django 3.2 on 2021-04-29 16:39

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('frama', '0004_alter_file_directory'),
    ]

    operations = [
        migrations.AddField(
            model_name='file',
            name='content',
            field=models.FileField(default='test', upload_to='uploads/'),
            preserve_default=False,
        ),
    ]