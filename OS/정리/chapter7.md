# C언어로 커널을 작성하자
## 실행 가능한 C코드 커널 생성 방법

<br>

이번 장에서 할 일은 c코드를 작성하여 이를 빌드하여 보호 모드 커널 이미지에 통합하는 것이다.

c언어로 작성한 커널을 보호모드 엔트리 포인트 뒷부분에 연결하고 엔트리 포인트에서는 C 커널의 시작부분으로 이동하는 것이다.

C코드는 컴파일과 링크 과정을 통해 결과물이 생성되는데 아래 그림과 같이 연결할 것이다.

<br>

![image](https://user-images.githubusercontent.com/52172169/195353132-4e3401f7-2415-4bc1-8ae0-9fa49bdc1e80.png)

<br>

<br><br>

### 빌드 조건과 제약 상황
---

<br>

+ C의 라이브러리를 호출하지 않게 빌드해야 한다.
  + 커널에서는 C의 라이브러리를 호출할 수 없기 때문에 라이브러리 함수가 실행되지 않게 된다.

<br>

+ ```0x10200```에서 C코드가 실행하게끔 빌드한다.
  + ```0x10000```에는 보호 모드 엔트리 포인트가 있으므로 결합된 C코드는 512바이트 이후에서 실행되어야 한다.

<br>

+ 코드나 데이터 외에 기타 정보를 포함하지 않는 순수한 바이너리 형태여야 한다.
  + gcc로 컴파일 해주면 ELF 포맷으로 나오기 때문에 그대로 사용 시 엔트리포인트에서 이를 해석해야 하므로 코드가 복잡해진다

<br><br>

### 소스 코드 컴파일 시 라이브러리를 사용하지 않은 오브젝트 파일 생성 방법
---

<br>

그러면 라이브러리를 호출하지 않게 빌드하려면 어떻게 해야하는가

홀로(Freestanding)라는 의미에서 컴파일 옵션으로 ```-ffreestanding```을 주면 된다.

따라서 ```clang -c -m32 -ffreestanding main.c```를 해주면 ```main.o```의 오브젝트 파일이 생성된다.

<br><br>

### 오브젝트 파일 링크 - 라이브러리를 사용하지 않고 특정 주소에서 실행 가능한 커널 이미지 파일 생성 방법
---

<br>

컴파일보다 까다로운 과정으로, 실행 파일을 구성하는 **섹션의 배치**와 **로딩될 주소**, 코드 내에서 가장 먼저 실행될 **엔트리 포인트**를 지정해줘야 한다.

**섹션을 재배치하는 이유**는 **링킹 과정에서 코드와 데이터 이외에 디버깅 관련 정보가 포함되므로 이를 제거해주기 위해** 코드와 데이터 부분을 앞으로 옮겨주고 나머지를 제거해줄 것이다.

이러한 정보들은 커널을 실행하는데 아무런 관련이 없기 때문에 최종 바이너리 파일 생성 시 이 부분을 손쉽게 제거하려고 코드와 데이터 부분을 앞부분으로 재배치하는 것이다.

<br>

**섹션**은 **실행 파일 또는 오브젝트 파일에 있고 공통된 속성을 담는 영역**으로, 여러 섹션이 있지만 핵심 섹션으로는 **.text, .data, .bss**가 있다.

```.text```는 **실행 가능한 코드가 있는 영역**이며, 권한은 **읽기 권한**만 있다.

```.data```는 **초기화된 전역변수와 정적변수를 담는 영역**으로, **읽기과 쓰기 권한**이 있다.

```.bss```는 **초기화되지 않은 전역변수, 정적변수를 담는 영역**으로, **읽기과 쓰기 권한**이 있다.

<br>

오브젝트 파일은 중간 단계의 코드이므로 **섹션의 크기**와 **오프셋 정보**만 들어있다.

**링킹 과정에서 오브젝트 파일을 결합하고 실제 메모리에 로딩될 위치를 정한다.**

이 과정에서 **링커가 실행파일을 만드려면 파일 구성에 대한 정보가 필요**한데, 이 때 **링커 스크립트**를 사용한다.

<br>

링커 스크립트에는 섹션 관련 정보가 저장되어 있는 텍스트 형식의 파일로, 여기서 우리가 섹션 재배치를 해줄 수 있다.

자세한 코드는 https://github.com/kkamagui/mint64os-examples/blob/master/source_code/chap07/01.Kernel32/elf_i386.x

<br>

위의 코드를 ```01.Kernel32``` 폴더에 ```elf_i386.x```로 저장해준다.

수정된 링크 스크립트를 통해 라이브러리를 호출하지 않고 실행파일을 만드는 방법이다.

```clang -melt_i386 -T elf_i386.x -nostdlib Main.o -o Main.elf```

T 옵션으로 링커 스크립트로 링크를 해주고, nostdlib 옵션은 no standard library라는 뜻이다.

<br><br>

이제 **로딩할 메모리 주소와 엔트리 포인트를 지정**해줘야 한다.

방법으로는 컴파일 옵션으로 주는 방법과 링커 스크립트에서 수정해주는 방법이 있다.

먼저 C코드는 아래 사진으로 보듯이 ```0x10200```에서 시작한다.

<br>

![image](https://user-images.githubusercontent.com/52172169/195381478-d0cfe013-41de-4f0c-913f-f15e65f5d8b8.png)

<br>

+ 컴파일 옵션
  + 메모리 주소 : ```clang -Ttext 0x10200 Main.o -o Main.elf```
  + 엔트리 포인트 : ```clang -e Main Main.o -o Main.elf```

<br>

+ 링커 스크립트
  + ```.text 0x10200```으로 수정
  + ```ENTRY(Main)```으로 수정

<br>

특정 함수를 가장 앞쪽에 위치시키는 방법은 **오브젝트 파일 내의 함수 간의 위치 조정**과 **실행 파일 내의 함수 간의 위치 조정**이다.

두 개 모두 가장 앞쪽으로 위치시켜주면 된다.

<br><br>

이제 **실행 파일을 바이너리 파일로 변환**해줘야 한다.

실행 파일에는 코드와 데이터 부분 이외에 다른 섹션의 정보도 포함되므로 이를 제거하여 바이너리 파일로 변환해줘야 한다.

이 과정을 objcopy 프로그램으로 해줄 수 있다고 한다.

```objcopy -j .text -j .data -j .rodata -j .bss -S -O binary Kernel32.elf Kernel32.bin```

j 옵션으로 섹션 추출, S 옵션으로 재배치 정보와 심볼 제거, -O로 파일 형식 지정

<br><br>
<hr style="border: 2px solid;">
<br><br>

## C 소스파일 추가 및 보호 모드 엔트리 포인트 통합

<br>

자세한 코드는 책에서 확인.

C코드의 엔트리 포인트는 Main함수

기존의 EntryPoint.s 코드부분에서 보호모드 부분에서 무한루프 부분을 C코드 커널로 넘어가도록 수정해준다.

<br>

+ main.c

```c
#include "Types.h"

void kPrintString(int iX, int iY, const char* pcString);

void main(void)
{
    kPrintString(0,3,"C Language Kernel Started~!!!");
    while(1);
}

void kPrintString(int iX, int iY, const char *pcString)
{
    CHARACTER* pstScreen = (CHARACTER*) 0xB8000;
    int i;

    pstScreen += (iY * 80) + iX;
    for(int i=0; pcString[i] != 0; i++)
    {
        pstScreen[i].bCharactor = pcString[i];
    }
}
```

<br>

+ 01.Kernel32/makefile

```
NASM = nasm
CC = clang -c -m32 -ffreestanding -nostdlib
LD = ld -melf_i386 -T ../elf_i386.x -nostdlib -e main -Ttext 0x10200
OBJCOPY = objcopy -j .text -j .data -j .rodata -j .bss -S -O binary

OBJECTDIR = Temp
SOURCEDIR = Source

all: prepare Kernel32.bin

prepare:
	mkdir -p $(OBJECTDIR)

$(OBJECTDIR)/EntryPoint.bin: $(SOURCEDIR)/EntryPoint.s
	$(NASM) -o $@ $<

dep:
	@echo === Make Dependancy File ===
	make -C $(OBJECTDIR) -f ../makefile InternalDependency
	@echo === Dependancy Search Complete ===

ExecuteInternalBuild: dep
	make -C $(OBJECTDIR) -f ../makefile Kernel32.elf

$(OBJECTDIR)/Kernel32.elf.bin: ExecuteInternalBuild
	$(OBJCOPY) $(OBJECTDIR)/Kernel32.elf $@

Kernel32.bin: $(OBJECTDIR)/EntryPoint.bin $(OBJECTDIR)/Kernel32.elf.bin
	cat $^ > $@

clean:
	rm -f *.bin
	rm -f $(OBJECTDIR)/*.*


## for InternalDependency
CENTRYPOINTOBJECTFILE = main.o
CSOURCEFILES = $(wildcard ../$(SOURCEDIR)/*.c)
ASSEMBLYSOURCEFILES = $(wildcard ../$(SOURCEDIR)/*.asm)
COBJECTFILES = $(subst main.o, , $(notdir $(patsubst %.c,%.o,$(CSOURCEFILES))))
ASSEMBLYOBJECTFILES = $(notdir $(patsubst %.asm,%.o,$(ASSEMBLYSOURCEFILES)))

%.o: ../$(SOURCEDIR)/%.c
	$(CC) -c $<

%.o: ../$(SOURCEDIR)/%.asm
	$(NASM) -f elf32 -o $@ $<

InternalDependency:
	$(CC) -MM $(CSOURCEFILES) > Dependency.dep

Kernel32.elf: $(CENTRYPOINTOBJECTFILE) $(COBJECTFILES) $(ASSEMBLYOBJECTFILES)
	$(LD) -o $@ $^

ifeq (Dependency.dep, $(wildcard Dependency.dep))
include Dependency.dep
endif
```

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 커널 빌드와 실행

<br>

먼저 부트로더가 OS를 메모리에 옮겨놓으면 메모리의 0x10000부터 올라가는데, 여기에는 보호모드 커널의 엔트리 포인트 코드가 들어있고 그 다음 0x10200에 C커널이 들어가 있다.

따라서 정상적으로 보호모드 커널이 실행되려면 부트로더의 TOTALSECTORCOUNT 값을 2로 수정해줘야 한다.

<br>

수정해준 뒤 make를 해주면 Kernel32.bin 파일이 생성되는 것을 볼 수 있는데, 문제는 이 파일의 크기가 저자는 646바이트, 내 기준 719이다.

즉, 2섹터에 못미치는 크기로 qemu는 섹터 단위로 정렬된 디스크 이미지만 처리할 수 있어서 에러가 발생한다.

따라서 디스크 이미지를 512바이트로 정렬하고 모자란 부분을 0x00으로 채워주면 해결할 수 있다고 한다.

이 작업을 이미지메이커 프로그램으로 자동화시켜준다.

<br>



<br><br>
<hr style="border: 2px solid;">
<br><br>
