from django.db import models
from django.contrib.auth.models import User as modelsUser
from django.utils import timezone

from datetime import datetime

LINUX_FILE_NAME_LIMIT = 255
DESC_LIMIT = 1000

class Entity(models.Model):
    is_valid = models.BooleanField(default=True)
    last_updated = models.DateTimeField(default=timezone.now)
    class Meta:
        abstract = True

class User(modelsUser, Entity): pass


class Directory(Entity):
    name = models.CharField(max_length=LINUX_FILE_NAME_LIMIT)
    description = models.CharField(max_length=DESC_LIMIT, blank=True, default='')
    creation_date = models.DateTimeField('date created', default=timezone.now)

    owner = models.ForeignKey(User, on_delete=models.CASCADE)
    parent_dir = models.ForeignKey('self', null=True, blank=True, on_delete=models.CASCADE, related_name='child_dirs')

    is_available = models.BooleanField(default=True)

    class Meta:
        constraints = [
            models.UniqueConstraint(fields=['name', 'parent_dir'], name='unique directory name')
        ]

    def __str__ (self):
        if self.parent_dir:
            return self.parent_dir.__str__() + '/' + self.name;
        else:
            return self.name;



class File(Entity):
    name = models.CharField(max_length=LINUX_FILE_NAME_LIMIT)
    description = models.CharField(max_length=DESC_LIMIT, blank=True, default='')
    creation_date = models.DateTimeField('date created', default=timezone.now)
    content = models.FileField(upload_to='uploads/')

    owner = models.ForeignKey(User, on_delete=models.CASCADE)
    directory = models.ForeignKey(Directory, on_delete=models.CASCADE, related_name='files')

    is_available = models.BooleanField(default=True)

    class Meta:
        constraints = [
            models.UniqueConstraint(fields=['name', 'directory'], name='unique file name')
        ]

    def __str__ (self):
        return self.directory.__str__() + '/' + self.name;


class FileSection(Entity):
    name = models.CharField(max_length=LINUX_FILE_NAME_LIMIT, blank=True, default='')
    description = models.CharField(max_length=DESC_LIMIT, blank=True, default='')
    creation_date = models.DateTimeField('date created', default=timezone.now)

    parent_section = models.ForeignKey('self', null=True, on_delete=models.CASCADE)
    file = models.ForeignKey(File, on_delete=models.CASCADE)

    class SectionCategory(models.TextChoices):
        PROCEDURE = 'PROC'
        PROPERTY = 'PROP'
        LEMMA = 'LEM'
        ASSERTION = 'ASSERT'
        INVARIANT = "INVAR"
        PRECONDITION = "PRECOND"
        POSTCONDITION = "POSTCOND"
    
    section_cat = models.CharField(max_length=10, choices=SectionCategory.choices)

    class SectionStatus(models.TextChoices):
        PROVED = 'PRV'
        INVALID = 'INV'
        COUNTEREXAMPLE = "CNTREX"
        UNCHECKED = 'UNCH'

    section_stat = models.CharField(max_length=10, choices=SectionStatus.choices)


class StatusData(Entity):
    data = models.CharField(max_length=200)
    user = models.ForeignKey(User, on_delete=models.CASCADE)
