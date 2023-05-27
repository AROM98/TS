import pyotp
import qrcode
import sys
import os

if len(sys.argv) >= 1:
    name = sys.argv[0]
else:
    raise Exception("Wrong arguments")

key = pyotp.random_base32()

issuer_name = 'TS_PD_3'

uri = pyotp.totp.TOTP(key).provisioning_uri(
    name=name,
    issuer_name=issuer_name)

print("Account: " + issuer_name + ":" + name)

print("Key: " + key)

print("URI: " + uri)


# Qr code generation step
qrcode.make(uri).save(os.path.join(name, "qr.png"))
print("Qrcode gerado!!")
file = open(os.path.join(name, "key.txt"), 'w')

file.write(key)

file.close()
