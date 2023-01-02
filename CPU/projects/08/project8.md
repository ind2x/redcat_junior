## Project 8

<br>

8장에서는 나머지 명령어들을 정의해준다.

<br>

+ branching commands
  + label
  + goto
  + if-goto

<br>

+ function commands
  + function
  + call
  + return 

<br>

코드 작성 후에 이제 크게 2가지 문제를 마주쳤는데, 하나는 return 부분과 하나는 static 부분이다.

return 부분은 강의 슬라이드에서 보여주는 코드 그대로 VM 코드로 작성해주면 해결이 된다.

이 부분에서 temp 메모리를 사용해줘야 하는데, 혹시 모르니 temp 7을 사용해서 temp 7에 리턴 주소를 저장시켜주었다.

<br>

static 부분은 예상치못하게 StaticsTest를 할 때, Class1와 Class2가 똑같은 static 0, static 1에 값을 저장하니까 Class1의 값이 사라지게 되어 틀린 결과가 나왔다.

vm 코드가 잘못된 줄 알았는데 포럼에 검색해보니 두 파일은 각각의 static 공간을 가진다는 것이다.

즉, Class1.vm에 있는 static 0는 ```@Class1.0```가 되고 Class2.vm에 있는 static 0는 ```@Class2.0```가 된다.

static은 address 16부터 시작되므로 각각 16, 17의 주소 값을 가지게 된다.

<br>

static 때문에 코드를 전체적으로 변경해주었다.

인자로 디렉터리명과 변환한 코드를 저장할 파일명을 받고, 인자로 받은 디렉터리에서 vm 파일을 검색하는데 Sys.vm을 우선적으로 변환해주고 변환한 코드는 인자로 받은 파일명에 저장해주게끔 변경해줄 것이다. 

<br><br>
<hr style="border: 2px solid;">
<br><br>
