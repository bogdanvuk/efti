#!/usr/bin/env python
from email.mime.text import MIMEText
from email.mime.application import MIMEApplication
from email.mime.multipart import MIMEMultipart
from smtplib import SMTP
import smtplib
import sys
import getpass

recipients = ['izi85@yahoo.com']
emaillist = [elem.strip().split(',') for elem in recipients]
msg = MIMEMultipart()
msg['Subject'] = "CV results"
msg['From'] = 'bogdan.vukobratovic@gmail.com'

msg.preamble = 'Multipart massage.\n'

part = MIMEText("Hi, please find the attached file")
msg.attach(part)

part = MIMEApplication(open(str(sys.argv[1]),"rb").read())
part.add_header('Content-Disposition', 'attachment', filename=str(sys.argv[1]))
msg.attach(part)


server = smtplib.SMTP("smtp.gmail.com:587")
server.ehlo()
server.starttls()
password = getpass.getpass()
server.login("bogdan.vukobratovic@gmail.com", password)


server.sendmail(msg['From'], emaillist , msg.as_string())
