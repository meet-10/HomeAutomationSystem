
from django.middleware.csrf import get_token



def token(request):
    if request.method == 'GET':
        token = get_token(request)
    return HttpResponse(token)

