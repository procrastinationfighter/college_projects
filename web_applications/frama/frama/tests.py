from django.test import TestCase
from django.test import Client

from django.core.files.uploadedfile import SimpleUploadedFile

from django.utils import timezone

from .models import *
from .views import *
from .forms import *

from django.test import Client

# Models tests.

class UserModelTests(TestCase):

    def test_adding_to_database(self):
        User.objects.create(username='test', password='testpass')
        self.assertTrue(User.objects.filter(username='test', password='testpass').exists())

class DirectoryModelTests(TestCase):

    def setUp(self):
        User.objects.create(username='owner', password='ownerpass')

    def test_adding_to_database(self):
        owner_user = User.objects.get(username='owner')
        Directory.objects.create(name='dir', description='my_test_case', owner=owner_user)
        self.assertTrue(Directory.objects.filter(name='dir', description='my_test_case').exists())

    def test_ownership(self):
        owner_user = User.objects.get(username='owner')
        Directory.objects.create(name='dir', description='my_test_case', owner=owner_user)
        Directory.objects.create(name='dir2', description='my_test_case', owner=owner_user)
        self.assertTrue(Directory.objects.filter(name='dir', owner__id=owner_user.id).exists())
        self.assertTrue(Directory.objects.filter(name='dir2', owner__id=owner_user.id).exists())

    def test_parent_dir(self):
        owner_user = User.objects.get(username='owner')
        parent = Directory.objects.create(name='parent', owner=owner_user)
        child = Directory.objects.create(name='child', owner=owner_user, parent_dir = parent)
        Directory.objects.create(name='child2', owner=owner_user, parent_dir = parent)

        self.assertTrue(Directory.objects.filter(name='child', parent_dir__name='parent').exists())
        self.assertTrue(Directory.objects.filter(name='child2', parent_dir__name='parent').exists())

        self.assertEquals(parent.__str__(), 'parent')
        self.assertEquals(child.__str__(), 'parent/child')

class FileModelTest(TestCase):

    def setUp(self):
        owner_user = User.objects.create(username='owner', password='ownerpass')
        Directory.objects.create(name='parent', owner=owner_user)

    def test_adding_to_database(self):
        owner_user = User.objects.get(username='owner')
        dir = Directory.objects.get(name='parent')
        file = File.objects.create(name='file', owner=owner_user, directory=dir)

        self.assertTrue(File.objects.filter(name='file', owner=owner_user, directory=dir).exists())
        self.assertEquals(dir.__str__() + '/' + file.name, file.__str__())

    def test_multi_dir_multi_owner(self):
        owner_user = User.objects.get(username='owner')
        dir = Directory.objects.get(name='parent')
        File.objects.create(name='file', owner=owner_user, directory=dir)
        File.objects.create(name='file2', owner=owner_user, directory=dir)

        self.assertTrue(File.objects.filter(name='file', owner=owner_user, directory=dir).exists())
        self.assertTrue(File.objects.filter(name='file2', owner=owner_user, directory=dir).exists())

class FileSectionModelTest(TestCase):

    def setUp(self):
        owner_user = User.objects.create(username='owner', password='ownerpass')
        dir = Directory.objects.create(name='parent', owner=owner_user)
        File.objects.create(name='file', owner=owner_user, directory=dir)

    def test_adding_to_database(self):
        file = File.objects.get(name='file')
        FileSection.objects.create(name='section', file=file, section_cat='ASSERT', section_stat='PRV')
        self.assertTrue(FileSection.objects.filter(name='section').exists())

    def test_belong_to_file(self):
        file = File.objects.get(name='file')
        FileSection.objects.create(name='section', file=file, section_cat='ASSERT', section_stat='PRV')
        FileSection.objects.create(name='section2', file=file, section_cat='LEM', section_stat='UNCH')
        
        self.assertTrue(FileSection.objects.filter(name='section', file=file).exists())
        self.assertTrue(FileSection.objects.filter(name='section2', file=file).exists())

    def test_subsection(self):
        file = File.objects.get(name='file')
        par = FileSection.objects.create(name='parent', file=file, section_cat='ASSERT', section_stat='PRV')
        FileSection.objects.create(name='child', file=file, section_cat='LEM', section_stat='UNCH', parent_section = par)
        FileSection.objects.create(name='child2', file=file, section_cat='PROC', section_stat='INV', parent_section = par)
        
        self.assertTrue(FileSection.objects.filter(name='child', parent_section = par).exists())
        self.assertTrue(FileSection.objects.filter(name='child2', parent_section = par).exists())

class StatusDataModelTest(TestCase):

    def setUp(self):
        User.objects.create(username='owner', password='ownerpass')

    def test_adding_to_database(self):
        user = User.objects.get(username='owner')
        StatusData.objects.create(data='suma podstawy', user=user)

        self.assertTrue(StatusData.objects.filter(data='suma podstawy').exists())

    def test_user(self):
        user = User.objects.get(username='owner')
        StatusData.objects.create(data='suma podstawy', user=user)
        StatusData.objects.create(data='kwadrat obu ramion', user=user)

        self.assertTrue(StatusData.objects.filter(data='suma podstawy', user=user).exists())
        self.assertTrue(StatusData.objects.filter(data='kwadrat obu ramion', user=user).exists())

# Views tests.

class LoginViewTest(TestCase):
    def setUp(self):
        User.objects.create(username='test', password='testpass')

    def test_login(self):
        response = self.client.post('/frama/login/', {'username': 'test', 'password': 'testpass'})
        self.assertEquals(response.status_code, 200)

    def test_login_page(self):
        response = self.client.get('/frama/login/')
        self.assertContains(response, '<form')

class IndexViewTest(TestCase):
    def setUp(self):
        user = User.objects.create(username='test', password='testpass')

    def test_index(self):
        self.client.login(username='test', password='testpass')
        response = self.client.get('/frama/')
        self.assertEquals(response.status_code, 302)

class ViewFileTest(TestCase):
    def setUp(self):
        user = User.objects.create(username='test', password='testpass')
        dir = Directory.objects.create(name='parent', owner=user)
        File.objects.create(name='file', owner=user, directory=dir)

    def test_main(self):
        self.client.login(username='test', password='testpass')
        file = File.objects.get(name='file')
        response = self.client.get('/frama/' + str(file.id) + '/')
        self.assertEquals(response.status_code, 302)


# Forms tests.

class AddFileFormTest(TestCase):

    def setUp(self):
        owner_user = User.objects.create(username='owner', password='ownerpass')
        Directory.objects.create(name='parent', owner=owner_user)

    def test_file_form_correct(self):
        dir = Directory.objects.get(name='parent')
        content = SimpleUploadedFile('file.exe', b'a sample file')
        form = FileForm(data={'name': 'file', 'directory': dir, 'description': 'a cool file'}, username='owner', files={'content': content})
        print(form.errors)
        self.assertTrue(form.is_valid())

        form = FileForm(data={'name': 'file', 'directory': dir}, username='owner', files={'content': content})
        print(form.errors)
        self.assertTrue(form.is_valid())

    def test_file_form_incorrect(self):
        dir = Directory.objects.get(name='parent')
        content = SimpleUploadedFile('file.exe', b'a sample file')

        form = FileForm(data={'name': 'file', 'directory': dir, 'description': 'a cool file'}, username='owner')
        self.assertFalse(form.is_valid())

        form = FileForm(data={'directory': dir, 'description': 'a cool file'}, username='owner', files={'content': content})
        self.assertFalse(form.is_valid())

        form = FileForm(data={'name': 'file', 'description': 'a cool file'}, username='owner', files={'content': content})
        self.assertFalse(form.is_valid())

        try:
            form = FileForm(data={'name': 'file', 'directory': dir, 'description': 'a cool file'}, files={'content': content})
        except KeyError:
            pass

class AddDirectoryFormTest(TestCase):

    def setUp(self):
        owner_user = User.objects.create(username='owner', password='ownerpass')
        Directory.objects.create(name='parent', owner=owner_user)

    def test_dir_form_correct(self):
        dir = Directory.objects.get(name='parent')

        form = DirectoryForm(data={'name': 'file', 'parent_dir': None, 'description': 'a cool directory'}, username='owner')
        self.assertTrue(form.is_valid())

        form = DirectoryForm(data={'name': 'file', 'parent_dir': None}, username='owner')
        self.assertTrue(form.is_valid())
        
        form = DirectoryForm(data={'name': 'file', 'parent_dir': dir}, username='owner')
        self.assertTrue(form.is_valid())

    def test_dir_form_incorrect(self):
        form = DirectoryForm(data={'parent_dir': None, 'description': 'a cool directory'}, username='owner')
        self.assertFalse(form.is_valid())

        try:
            form = FileForm(data={'name': 'file', 'parent_dir': None, 'description': 'a cool directory'})
        except KeyError:
            pass