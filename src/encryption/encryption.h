#include <string.h>
#include <jansson.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

#define AES_BLOCK_SIZE 256
#define IV_LENGTH 16

int encode_base64(unsigned const char* input, char** buffer, int input_size);
int calculate_decoded_len(const char* base64_input);
int decode_base64(char* base64_input, unsigned char** buffer);
int aes_init_enc(EVP_CIPHER_CTX *e_ctx, unsigned char *encryption_iv);
int aes_init_dec(EVP_CIPHER_CTX *e_ctx, unsigned char *decryption_iv);
unsigned char *aes_encrypt(EVP_CIPHER_CTX *e, char *plain_text, int *len);
char *aes_decrypt(EVP_CIPHER_CTX *e, unsigned char *cipher_text, int *len);
char *encrypt_packet(json_t *json_input);
void *decrypt_packet(char *base64_input, json_t **json_output);

