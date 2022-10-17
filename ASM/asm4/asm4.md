## 구구단 출력

<br>

```c
#include <stdio.h>

int main()
{
    int i = 2;
    int j = 1;
    while(j<10)
    {
        i = 2;
        while(i < 10)
        {
            printf("%d * %d = %2d    ", i, j, i*j);
            i++;
        }
        printf("\n");
        j++;
    }
}
```

<br><br>
