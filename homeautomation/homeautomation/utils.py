import os
from twilio.rest import Client

 # Your Account Sid and Auth Token from twilio.com/console
# and set the environment variables. See http://twil.io/secure
account_sid = 'ACfbbcca2013c847f32fbc76a404b91a79'
auth_token = '4a803118761a12e3bafce43252f210dd'
client = Client(account_sid, auth_token)

def send_sms(user_code, phone_number):
    message = client.messages \
                            .create(
                                body=f'Hi! Your user and verification code is -{user_code}',
                                from_='+14255295048',
                                to='+919409180128'
                            )

    print(message.sid)