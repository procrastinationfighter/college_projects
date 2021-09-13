from django.shortcuts import get_object_or_404, render, redirect
from django.http import HttpResponse, HttpResponseRedirect
from django.views.generic.edit import CreateView
from django.urls import reverse

from django.contrib.auth.forms import AuthenticationForm
from django.contrib.auth import authenticate, login
from django.contrib.auth.decorators import login_required

from .models import *
from .forms import *

import subprocess;

class FileCreateView(CreateView):
    form_class = FileForm
    template_name = 'frama/add.html'

    def get_success_url(self):
        return reverse('index')
    
    def form_valid(self, form):
        form.instance.owner = User.objects.get(username=self.request.user.username)
        return super().form_valid(form)

    def get_form_kwargs(self):
        kwargs = super(FileCreateView, self).get_form_kwargs()
        kwargs.update({'username': self.request.user.username})
        return kwargs

class DirCreateView(CreateView):
    form_class = DirectoryForm
    template_name = 'frama/add.html'

    def get_success_url(self):
        return reverse('index')

    def form_valid(self, form):
        form.instance.owner = User.objects.get(username=self.request.user.username)
        return super().form_valid(form)

    def get_form_kwargs(self):
        kwargs = super(DirCreateView, self).get_form_kwargs()
        kwargs.update({'username': self.request.user.username})
        return kwargs


def user_login(request):
    if request.user.is_authenticated:
        return redirect('index')
    elif request.method == 'POST':
        form = AuthenticationForm(request=request, data=request.POST)
        if form.is_valid():
            username = form.cleaned_data.get('username')
            password = form.cleaned_data.get('password')
            user = authenticate(username=username, password=password)
            if user is not None:
                login(request, user)
                return redirect('index')
            else:
                return render(request, 'frama/login.html', {'form': form})
        else:
            return render(request, 'frama/login.html', {'form': form})
    else:
        form = AuthenticationForm()
        return render(request, 'frama/login.html', {'form': form})

def get_user_dirs(user):
    return (Directory.objects.all()
    .filter(parent_dir__isnull=True)
    .filter(owner__username=user.username)
    .order_by('name'))

@login_required(login_url='/frama/login')
def index(request):
    directories = get_user_dirs(request.user)
    context = {'directories': directories}
    return render(request, 'frama/index.html', context)

@login_required(login_url='/frama/login')
def delete(request):
    directories = get_user_dirs(request.user)
    context = {'directories': directories}
    return render(request, 'frama/delete.html', context)

@login_required(login_url='/frama/login')
def delete_file(request, file_id):
    file = get_object_or_404(File, pk=file_id)
    file.is_available = False
    file.save()
    return redirect('index')

@login_required(login_url='/frama/login')
def delete_dir(request, dir_id):
    dir = get_object_or_404(Directory, pk=dir_id)
    dir.is_available = False
    dir.save()
    return redirect('index')

@login_required(login_url='/frama/login')
def view_file(request, file_id):
    directories = get_user_dirs(request.user)

    file = get_object_or_404(File, pk=file_id)
    file = file.content
    file.open("r")

    try:
        focus_args = ['frama-c', '-wp', '-wp-print']
        focus_args.append(file.path)
        focus_result = subprocess.run(focus_args, stdout=subprocess.PIPE)
        focus_result = focus_result.stdout.decode("utf-8")
        subprocess.run(['frama-c', '-wp', '-wp-log=r:result.txt', file.path])

        log = open("result.txt", "r")

        context = {
            'directories': directories, 
            'file': file.read(), 
            'focus': focus_result,
            'log': log.read()
            }
        file.close()
        log.close()

        return render(request, 'frama/index.html', context)
    except:
        return render(request, 'frama/index.html', {
                                            'directories': directories, 
                                            'file': file.read(), 
                                            'focus': focus_result,
                                            'log': "Frama-C is probably NOT installed on server."
                                            })
