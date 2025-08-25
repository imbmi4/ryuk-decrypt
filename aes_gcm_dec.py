from Crypto.Cipher import AES


with open("d8975c4d-19f9-4182-9794-ddea001be5a5.aes", "r") as key_file:
    key_data = key_file.read()

with open("FLAG.txt.ryk", "rb") as encrypted_file:
    encrypted_data = encrypted_file.read()


key = bytes.fromhex(key_data)
nonce = encrypted_data[:12]
real_encrypted_data = encrypted_data[12:-16]
gcm_tag = encrypted_data[-16:]

cipher = AES.new(key, AES.MODE_GCM, nonce=nonce)
decrypted_data = cipher.decrypt_and_verify(real_encrypted_data, gcm_tag)
print(decrypted_data)