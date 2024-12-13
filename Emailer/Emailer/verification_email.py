from sendgrid import SendGridAPIClient
from sendgrid.helpers.mail import Mail
import os
import traceback

# Required environment variables:
# HOST_EMAIL: The email address to send from
# SENDGRID_API_KEY: Your SendGrid API key for authentication
#                   Get this from your SendGrid account settings and set it as an environment variable
#                   Follow SendGrid documentation for obtaining the API key
host_email = os.environ.get('HOST_EMAIL')
if not host_email:
    raise ValueError("HOST_EMAIL environment variable is required")

sendgrid_api_key = os.environ.get('SENDGRID_API_KEY')
if not sendgrid_api_key:
    raise ValueError("SENDGRID_API_KEY environment variable is required")

class Email():
    def __init__(self, dest, smtp_server, smtp_port):
        global host_email

        self.dest = dest
        self.smtp_server = smtp_server
        self.smtp_port = smtp_port

        self.subject = "Your Verification Code"

    def send_verif_code(self, verif_code):
        global host_email
        body = f"Your verification code is : {verif_code}"
        try:
            # Creating the email message 
            message = Mail(from_email=host_email, to_emails=self.dest,
                         subject=self.subject, 
                         plain_text_content=body)

            sendgrid_client = SendGridAPIClient(sendgrid_api_key)
            response = sendgrid_client.send(message)
            print("Verification email sent successfully.")
        except Exception as e:
            traceback.print_exc()
            print(e)

