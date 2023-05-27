import pyotp
import qrcode
import os

output_path = "/home/kubuntuu/Desktop/TS/TS/"

key = pyotp.random_base32()

name = input("Enter your name : ")
issuer_name = 'TS_PD_3'

uri = pyotp.totp.TOTP(key).provisioning_uri(
    name=name,
    issuer_name=issuer_name)

print("Account: " + issuer_name + ":" + name)

print("Key: " + key)

print("URI: " + uri)


# Qr code generation step
qrcode.make(uri).save(os.path.join(output_path, "qr.png"))

"""Verifying stage starts"""

totp = pyotp.TOTP(key)

# verifying the code
if not totp.verify(input(("Enter the Code : "))):
    raise "Wrong code"
else: 
    print("Success!")
