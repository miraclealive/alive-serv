#include <string.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

#define AES_BLOCK_SIZE 256

int encode_base64(unsigned const char* input, unsigned char** buffer, int input_size);
int calculate_decoded_len(unsigned const char* base64_input);
int decode_base64(unsigned char* base64_input, unsigned char** buffer);
int aes_init_enc(EVP_CIPHER_CTX *e_ctx);
int aes_init_dec(EVP_CIPHER_CTX *e_ctx);
unsigned char *aes_encrypt(EVP_CIPHER_CTX *e, unsigned char *plaintext, int *len);
unsigned char *aes_decrypt(EVP_CIPHER_CTX *e, unsigned char *ciphertext, int *len);
unsigned char *encrypt_packet(json_t *json);

