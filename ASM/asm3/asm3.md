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




<br><br>
<hr style="border: 2px solid;">
<br><br>

