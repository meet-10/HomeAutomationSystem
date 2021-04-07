from django.shortcuts import render , HttpResponse
from django.middleware.csrf import get_token


def display(request):
    return render(request, 'display.html')

def token(request):
    if request.method == 'GET':
        token = get_token(request)
    return HttpResponse(token)


    