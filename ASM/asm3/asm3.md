## itoa

<br>

```c
#include <stdio.h>

void itoa(int value, char *str);
int main()
{
    char buf[1024] = {0, };
    itoa(12345678, buf);
    printf("itoa(12345678) : %s\n",buf);
}

void itoa(int value, char *str)
{
    int max_pos = 1;
    int idx = 0;
    int saved_value = value;

    while(value / max_pos >= 10) {  // > 10 이면 10 자체는 빈칸으로 나옴
        max_pos *= 10;
    }

    while(max_pos > 0)
    {
        str[idx] = 0x30 + (value / max_pos);
        value -= (value / max_pos * max_pos);

        idx++;
        max_pos = max_pos / 10;
    }
}
```

<br>

강의 내용처럼 코드를 먼저 따라쳐보았는데, 결과값에 이상한 값이 붙거나 작은 값을 했을 때 숫자가 아닌 이상한 값이 출력되는 문제가 발생했다.

그래서 위의 itoa.c 코드를 실행파일로 만들어서 gdb로 보니 memset 함수를 호출하는 부분이 있다.

```char buf[1024] = {0, };``` 이 부분이 memset 함수를 호출하는 코드 부분이 되겠다.

따라서 memset 코드를 작성하여 추가해준 뒤 해보니 정상적으로 잘 작동했다.

이제 인텔 형식으로 내 생각대로 짜보겠다.

<br><br>
<hr style="border: 2px solid;">
<br><br>
