/* 파일 암호화 / 복호화 프로그램 최소요구사항

+ AES 암호화 / 복호화 지원 (CBC, ECB), 비트는 128, 192, 256

+ 해시 지원 -> md5, sha1, sha256, sha512

+ 단일 파일 암호화 / 복호화 지원

+ 디렉토리 전체 암호화 / 복호화 지원

+ 추가 구현 사항 -> 디렉토리 암호화 시 쓰레드로  속도 향상

*/

#include <stdio.h>          // fopen, fseek, ftell, fread, fclose, sprintf
#include <string.h>         // strcpy, strstr, strcat, strncat, memset
#include <stdlib.h>         // exit, malloc
#include <unistd.h>         // getopt, stat
#include <sys/types.h>      // stat
#include <sys/stat.h>       // stat
#include <errno.h>          // perror
#include <openssl/evp.h>    // EVP_MD_CTX, EVP_CIPHER_CTX
#include <openssl/conf.h>   // EVP_CIPHER_CTX
#include <openssl/err.h>    // handleErrors

void help();
void checkFileOrDir(char *input); // check input is file or dir by stat.st_mode

void makeDigest(unsigned char *plaintext, char *output, int FileSize);  // hash

void handleErrors(void);
void encryptAES(unsigned char *plaintext, int plaintext_len, unsigned char *key, 
                const char *cipher, char *output);
void decryptAES(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, 
                const char *cipher, char *output);

int checkFileSize(FILE *input_file);  // check file size to allocate buffer[file size]
void processFile(char *input_file, char *output_file); // if input is file, process file

void makeDir(); // make output_dir
void processDir(char *input_dir, char *output_dir); // if input is dir, process dir

int file, dir, decrypt_on, verbose_on;
char *algorithm = NULL;

/* 128 / 192 / 256 bit key */
unsigned char *key_128 = (unsigned char *)"0123456789012345";
unsigned char *key_192 = (unsigned char *)"012345678901234567890123";
unsigned char *key_256 = (unsigned char *)"01234567890123456789012345678901";

/* A 128 bit IV */
unsigned char *iv = (unsigned char *)"0123456789012345";

int main(int argc, char *argv[])
{
    if(argc < 2) 
    { 
        help();  
        exit(1);
    }

    int c;
    char *input = NULL, *output = NULL;
    
    while((c = getopt(argc, argv, "hi:o:a:dv")) != -1)
    {
        switch(c)
        {
            case 'h':
                help();
                break;
            case 'i':
                input = malloc(strlen(optarg) + 1);
                strcpy(input, optarg);
                break;
            case 'o':
                output = malloc(strlen(optarg) + 1);
                strcpy(output, optarg);
                break;
            case 'a':
                algorithm = malloc(strlen(optarg) +1);
                strcpy(algorithm, optarg);
                break;
            case 'd':
                decrypt_on = 1;
                break;
            case 'v':
                verbose_on = 1;
                break;
            default:
                break;
        }
    }

    if(input == NULL || algorithm == NULL) // argc < 5로 체크했으나 샘플 코드 보고 수정
    {
        help();
        exit(1);
    }
    
    checkFileOrDir(input);

    if(file) {
        processFile(input, output);
    }

    if(dir) {
        processDir(input, output);
    }

    free(input);
    if(output != NULL) { free(output); }
    free(algorithm);

    return 0;
}

void help()
{
    printf("Usage: ./crypt -i <Input> -a <algorithm> [OPTIONS]\n");
    printf("\nOPTIONS: \n");
    printf("    %-16s   %14s\n","-h","Print Usage");
    printf("    %-16s   %30s\n","-i <file/directory>","Input File or Directory to encrypt");
    printf("    %-16s   %30s\n","-o <file/directory>","Output File or Directory for save the result");
    printf("    %-16s   %37s\n","-a <algorithm>","Encrypt file by selected algorithm");
    printf("                          Support openssl-dgst, AES-[128|192|256]-[CBC|ECB]\n");
    printf("                          AES default (aes-256-cbc), Except md4, ripemd160\n");
    printf("    %-16s %32s\n","-d","Decrypt (only AES possible)");
    printf("    %-16s %12s\n","-v","Verbose");
}

void checkFileOrDir(char *input)
{
    struct stat sb;
    dir = 0, file = 0;

    if (stat(input, &sb) == -1) 
    {
        perror("stat");
        exit(1);
    }

    if(verbose_on) {
        printf("> Input Type: ");
    }

    switch (sb.st_mode & S_IFMT) {
        case S_IFDIR:  
            if(verbose_on) 
            { 
                printf("Directory\n"); 
                printf("> Directory Name: %s\n",input);
            }
            dir = 1;
            break;
        case S_IFREG:  
            if(verbose_on) 
            { 
                printf("Regular File\n");
                printf("> File Name: %s\n",input);
            }
            file = 1;
            break;
        default:       
            printf("> Error: Input File or Directory!\n");
            break;
    }
}

void handleErrors(void)
{
    ERR_print_errors_fp(stderr);
    abort();
}

void encryptAES(unsigned char *plaintext, int plaintext_len, unsigned char *key, 
                const char *cipher, char *output)
{
    EVP_CIPHER_CTX *ctx;

    unsigned char ciphertext[128];
    int ciphertext_len = 0;
    int len = 0;

    OpenSSL_add_all_ciphers();
    const EVP_CIPHER *cp;
    cp = EVP_get_cipherbyname(cipher);

    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();

    if(1 != EVP_EncryptInit_ex(ctx, cp, NULL, key, iv))
        handleErrors();

    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
        handleErrors();
    
    ciphertext_len = len;

    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
        handleErrors();
    
    ciphertext_len += len;

    if(output == NULL) 
    {
        printf("> Cipher: %s\n", cipher);
        printf("> Result: %s\n", ciphertext);
        if(verbose_on) {
            printf("> Length: %u byte\n", ciphertext_len);
        }
    }
    else
    {
        FILE *fp = fopen(output, "w+");
        fwrite(ciphertext, 1, ciphertext_len, fp);
        fclose(fp);
    }
    EVP_CIPHER_CTX_free(ctx);
}

void decryptAES(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, 
                const char *cipher, char *output)
{
    EVP_CIPHER_CTX *ctx;
    unsigned char plaintext[256];
    int len = 0;
    int plaintext_len = 0;

    OpenSSL_add_all_ciphers();
    const EVP_CIPHER *cp;
    cp = EVP_get_cipherbyname(cipher);

    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();

    if(1 != EVP_DecryptInit_ex(ctx, cp, NULL, key, iv))
        handleErrors();

    if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
        handleErrors();

    plaintext_len = len;

    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
        handleErrors();

    plaintext_len += len;

    if(output == NULL) 
    {
        printf("> Cipher: %s\n", cipher);
        printf("> Result: %s\n", plaintext);
        if(verbose_on) {
            printf("> Length: %u byte\n", plaintext_len);
        }
    }
    else
    {
        FILE *fp = fopen(output, "w+");
        fwrite(plaintext, 1, plaintext_len, fp);
        fclose(fp);
    }
    EVP_CIPHER_CTX_free(ctx);
}

/*
https://github.com/yuanhui360/CPP-Programming-on-Linux/blob/main/YH-119/evp_digest_disp.cpp

https://m.blog.naver.com/seongjeongki7/220890684562

https://wiki.openssl.org/index.php/EVP_Message_Digests
*/
void makeDigest(unsigned char *plaintext, char *output, int FileSize)
{
    EVP_MD_CTX *mdctx;
    const EVP_MD *md;
    unsigned char digestMessage[EVP_MAX_MD_SIZE];
    unsigned int dM_len;

    OpenSSL_add_all_digests();
    md = EVP_get_digestbyname(algorithm);
    if (md == NULL) {
        printf("> Error: Unknown message digest < %s >\n", algorithm);
        exit(1);
    }

    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, plaintext, FileSize); // no md4
    EVP_DigestFinal_ex(mdctx, digestMessage, &dM_len);

    char HexDigestMessage[EVP_MAX_MD_SIZE*2] = "";

    for(int i=0; i < dM_len; i++)  // make binary value to hex
    {
        char dM[3];
        sprintf(dM, "%02x",digestMessage[i]);
        strncat(HexDigestMessage, dM, 2);
    }
    
    if(output == NULL) 
    {
        printf("> Algogrithm: %s\n",algorithm);
        printf("> Result: %s\n",HexDigestMessage);
        if(verbose_on) {
            printf("> Length: %u byte\n", dM_len);
        }
    }
    else
    {
        FILE *fp = fopen(output, "w+");
        fwrite(HexDigestMessage, 1, dM_len*2, fp);
        fclose(fp);
    }

    EVP_MD_CTX_free(mdctx);
}

int checkFileSize(FILE *fp)
{
    fseek(fp, 0, SEEK_END); // 파일 끝으로 이동
    int size = ftell(fp);       // 파일 포인터 현재 위치 얻음
    return size;
}

void processFile(char *input_file, char *output_file)
{
    FILE *fp = fopen(input_file, "r+");
    
    // if(input_content == NULL) {} --> 파일 없으면 stat에서 에러 일으킴
    
    int size = checkFileSize(fp);
    unsigned char *buffer = malloc(size + 1); // alloc buffer[file size]
    
    memset(buffer, 0, size + 1);    // 0으로 초기화
    fseek(fp, 0, SEEK_SET);         // 파일 처음으로 이동
    fread(buffer, 1, size, fp);
    fclose(fp);

    if(verbose_on)
    {
        printf("> File Size: %d\n",size);
        printf("> Plaintext: %s\n", buffer);
    }
    
    if(strncmp(algorithm, "aes", 3) == 0)
    {
        char cipher[12] = "aes-";
        char mode[] = "cbc";
        unsigned char *key = NULL;
        if(strstr(algorithm, "ecb")) { strcpy(mode, "ecb"); }
        
        if(strstr(algorithm,"128")) 
        { 
            strcat(cipher,"128-");
            key = key_128;
        }
        else if(strstr(algorithm,"192")) 
        { 
            strcat(cipher,"192-");
            key = key_192;
        }
        else
        { 
            strcat(cipher,"256-");
            key = key_256;
        }
        strcat(cipher, mode);

        if(decrypt_on) {
            decryptAES(buffer, size, key, cipher, output_file);
        }
        else { 
            encryptAES(buffer, size, key, cipher, output_file);
        }
    }
    else {
        makeDigest(buffer, output_file, size);
    }
    
    free(buffer);
}

void processDir(char *input_dir, char *output_dir)
{
    // not constructed
}
