from django.apps import AppConfig


class CodesConfig(AppConfig):
    name = 'codes'

    def ready(self):
        import codes.signals
