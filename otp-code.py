import pyotp
import qrcode
import sys

if len(sys.argv) > 1:
    name = sys.argv[1]
else:
    raise "Wrong arguments"

key = pyotp.random_base32()

issuer_name = 'TS_PD_3'

uri = pyotp.totp.TOTP(key).provisioning_uri(
    name=name,
    issuer_name=issuer_name)

print("Account: " + issuer_name + ":" + name)

print("Key: " + key)

print("URI: " + uri)


# Qr code generation step
qrcode.make(uri).save("qr.png")

file = open('key.txt', 'w')

file.write(key)

file.close()
