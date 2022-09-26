/* 파일 암호화 / 복호화 프로그램 최소요구사항

+ AES 암호화 / 복호화 지원 (CBC, ECB), 비트는 128, 192, 256

+ 해시 지원 -> md5, sha1, sha256, sha512

+ 단일 파일 암호화 / 복호화 지원

+ 디렉토리 전체 암호화 / 복호화 지원

*/

#include <stdio.h>
#include <string.h> // strcpy
#include <stdlib.h> // exit
#include <unistd.h> // getopt, stat
#include <sys/types.h> // stat
#include <sys/stat.h> // stat
#include <errno.h> // perror

void help();
void checkFileOrDir(char *input); // check input is file or dir by stat.st_mode

void processFile(char *input_file, char *output_file); // if input is file, process file
void readFile(char *input_file);
void writeFile(char *output_file);

void processDir(char *input_dir, char *output_dir); // if input is dir, process dir
void makeDir();
void readDir(char *input_dir);
void writeDir(char *output_dir);

void encrypt();
void decrypt();

int file, dir, decrypt_on, verbose_on;
char *algorithm = NULL;

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

    return 0;
}

void help()
{
    printf("Usage: ./crypt -i <Input> -o <Output> -<algorithm>\n");
    printf("\nOPTIONS: \n");
    printf("    %-16s   %14s\n","-h","Print Usage");
    printf("    %-16s   %30s\n","-i <file/directory>","Input File or Directory to encrypt");
    printf("    %-16s   %30s\n","-o <file/directory>","Output File or Directory for save the result");
    printf("    %-16s   %34s\n","-a <algorithm>","Encryption algorithms supported");
    printf("                          [aes128ecb, aes128cbc, aes192ecb, aes192cbc, aes256ecb, aes256cbc, md5, sha1, sha256, sha512]");
    printf("\n    %-16s %12s\n","-d","Decrypt");
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
        printf("Input Type: ");
    }

    switch (sb.st_mode & S_IFMT) {
        case S_IFDIR:  
            if(verbose_on) 
            { 
                printf("Directory\n"); 
                printf("Directory Name: %s\n",input);
            }
            dir = 1;
            break;
        case S_IFREG:  
            if(verbose_on) 
            { 
                printf("Regular File\n");
                printf("File Name: %s\n",input);
            }
            file = 1;
            break;
        default:       
            printf("WTF is this?\n");
            break;
    }
}

void processFile(char *input_file, char *output_file)
{
    FILE *input_content = fopen(input_file, "r+");
    
    if(input_content == NULL) {
        fprintf(stderr, "No file!\n");
        exit(1);
    }

    // if(!decrypt_on) encrypt(input_content);
    
}

void processDir(char *input_dir, char *output_dir)
{

}
