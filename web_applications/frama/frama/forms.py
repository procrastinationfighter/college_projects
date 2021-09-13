from django.forms import ModelForm, widgets
from frama.models import File, Directory

class FileForm(ModelForm):
    class Meta:
        model = File
        fields = ['name', 'directory', 'content', 'description']
        
    def __init__(self, *args, **kwargs) -> None:
        username = kwargs.pop("username")
        super(FileForm, self).__init__(*args, **kwargs)
        self.fields['directory'].queryset = Directory.objects.filter(owner__username=username).filter(is_available=True)

class DirectoryForm(ModelForm):
    class Meta:
        model = Directory
        fields = ['name', 'parent_dir', 'description']
        
    def __init__(self, *args, **kwargs) -> None:
        username = kwargs.pop("username")
        super(DirectoryForm, self).__init__(*args, **kwargs)
        self.fields['parent_dir'].queryset = Directory.objects.filter(owner__username=username).filter(is_available=True)