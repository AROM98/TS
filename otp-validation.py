import pyotp
import sys
import os


if len(sys.argv) >= 1:
    filepath = sys.argv[0]
else:
    raise Exception("Wrong arguments")

file = open(os.path.join(filepath, "key.txt"), 'r')

key = file.read()

file.close()

# """Verifying stage starts"""

totp = pyotp.TOTP(key)

# verifying the code
if not totp.verify(input(("Enter the Code : "))):
    raise Exception("Wrong code")

print("Success!")
