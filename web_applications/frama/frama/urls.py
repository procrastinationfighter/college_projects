from django.urls import path

from django.contrib import admin

from . import views

urlpatterns = [
    path('', views.index, name='index'),
    path('admin/', admin.site.urls),
    path('add_file/', views.FileCreateView.as_view(), name='add_file'),
    path('add_dir/', views.DirCreateView.as_view(), name='add_dir'),
    path('delete/', views.delete, name='delete'),
    path('<int:file_id>/delete/', views.delete_file, name='delete_file'),
    path('dir/<int:dir_id>/delete/', views.delete_dir, name='delete_dir'),
    path('<int:file_id>/', views.view_file, name='view_file'),
    path('login/', views.user_login, name='login'),
]