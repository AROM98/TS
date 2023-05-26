import pyotp
import sys


if len(sys.argv) > 1:
    code = sys.argv[1]
else:
    raise "Wrong arguments"

file = open('key.txt', 'r')

key = file.read()

file.close()

"""Verifying stage starts"""

totp = pyotp.TOTP(key)

# verifying the code
if not totp.verify(code):
    raise "Wrong code"

print("Success!")
