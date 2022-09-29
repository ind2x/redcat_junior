/* 파일 암호화 / 복호화 프로그램 최소요구사항

+ AES 암호화 / 복호화 지원 (CBC, ECB), 비트는 128, 192, 256

+ 해시 지원 -> md5, sha1, sha256, sha512

+ 단일 파일 암호화 / 복호화 지원

+ 디렉토리 전체 암호화 / 복호화 지원

*/

#include <stdio.h>      // fopen, fseek, ftell, fread, fclose, strcasecmp
#include <string.h>     // strcpy, memset
#include <stdlib.h>     // exit, malloc
#include <unistd.h>     // getopt, stat
#include <sys/types.h>  // stat
#include <sys/stat.h>   // stat
#include <errno.h>      // perror

void help();
void checkAlgorithm(char *algorithm); // check if algorithm is supported
void checkFileOrDir(char *input); // check input is file or dir by stat.st_mode

int checkFileSize(FILE *input_file);  // check file size to allocate buffer[file size]

void processFile(char *input_file, char *output_file); // if input is file, process file

void makeDir(); // make output_dir
void processDir(char *input_dir, char *output_dir); // if input is dir, process dir

void encrypt();
void decrypt();

int file, dir, decrypt_on, verbose_on;
char *algorithm = NULL;

typedef struct supportedAlgorithm {
    char algo[10]; 
} sAlgorithm;

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
    
    checkAlgorithm(algorithm);

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
    printf("Usage: ./crypt -i <Input> -o <Output> -<algorithm>\n");
    printf("\nOPTIONS: \n");
    printf("    %-16s   %14s\n","-h","Print Usage");
    printf("    %-16s   %30s\n","-i <file/directory>","Input File or Directory to encrypt");
    printf("    %-16s   %30s\n","-o <file/directory>","Output File or Directory for save the result");
    printf("    %-16s   %34s\n","-a <algorithm>","Encryption algorithms supported");
    printf("                          [AES128ECB, AES128CBC, AES192ECB, AES192CBC, AES256ECB, AES256CBC, MD5, SHA1, SHA256, SHA512]");
    printf("\n    %-16s %21s\n","-d","Decrypt (only AES possible)");
    printf("    %-16s %12s\n","-v","Verbose");
}

void checkAlgorithm(char *algorithm)
{
    sAlgorithm a[] = {
        {"AES128ECB"}, 
        {"AES128CBC"}, 
        {"AES192ECB"}, 
        {"AES192CBC"}, 
        {"AES256ECB"}, 
        {"AES256CBC"}, 
        {"MD5"}, 
        {"SHA1"}, 
        {"SHA256"}, 
        {"SHA512"}
    };
    
    int check = 0;
    for(int i=0; i<10; i++)
    {
        if(strcasecmp(algorithm, a[i].algo) == 0) 
            check = 1; 
    }
    if(check == 0) {
        fprintf(stderr, "> Error: This algorithm isn't supported!\n");
        exit(1);
    }
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
            printf("> WTF is this?\n");
            break;
    }
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
    char *buffer = malloc(size + 1); // alloc buffer[file size]
    
    memset(buffer, 0, size + 1);    // 0으로 초기화
    fseek(fp, 0, SEEK_SET); // 파일 처음으로 이동
    fread(buffer, size, 1, fp);
    fclose(fp);

    if(verbose_on)
    {
        printf("> File Size: %d\n",size);
        printf("> Plaintext: %s\n", buffer);
    }
    // if(!decrypt_on) encrypt(input_content);
    
    free(buffer);
}

void processDir(char *input_dir, char *output_dir)
{

}
