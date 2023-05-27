import pyotp
import sys
import os


if len(sys.argv) >= 1:
    code = sys.argv[0]
else:
    raise "Wrong arguments"

file = open(os.path.join(code, "key.txt"), 'r')

key = file.read()

file.close()

# """Verifying stage starts"""

totp = pyotp.TOTP(key)

# verifying the code
if not totp.verify(input(("Enter the Code : "))):
    raise "Wrong code"

print("Success!")
