# Project 10
## Compiler 구조 및 예시

<br>

+ Compiler

![image](https://user-images.githubusercontent.com/52172169/220122498-ea8d5908-2124-4200-9109-d234efd0b917.png)

<br>

+ tokenizer

![image](https://user-images.githubusercontent.com/52172169/220122345-ec6f1830-f612-497c-b14e-5b4ef521760d.png)

<br>

+ parser tree

![image](https://user-images.githubusercontent.com/52172169/220122698-ba4226ec-0109-43e7-958b-afa28c74d4ca.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/220122655-78031e3a-c421-41ca-8c16-fcd5cfe3244c.png)

<br><br>
<hr style="border: 2px solid;">
<br><br>

## Jack Grammar

+ Jack Grammar

![image](https://user-images.githubusercontent.com/52172169/220128942-546183a5-ce6d-4552-974f-5ac76d6a58a7.png)

<br>

+ lexical elements

![image](https://user-images.githubusercontent.com/52172169/220129417-bed17d57-3423-4a63-9483-399b289f79e3.png)

<br>

+ program structure

![image](https://user-images.githubusercontent.com/52172169/220129473-2d7f2057-5aaf-4117-bf7d-0feb05e88cd8.png)

<br>

+ statements

![image](https://user-images.githubusercontent.com/52172169/220129525-a030b46d-4e69-4b09-81e5-11a901d49854.png)

<br>

+ expression

![image](https://user-images.githubusercontent.com/52172169/220129562-58425c42-580d-435c-b2fc-03a9c0531710.png)

<br><br>
<hr style="border: 2px solid;">
<br><br>

## Jack Analyzer API

<br>

+ Usage

![image](https://user-images.githubusercontent.com/52172169/220130321-2d2d43f2-1ad6-409b-a264-80600d35760e.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/235303417-7781c763-77b4-4d16-9484-7865648d8e51.png)

<br>

+ JackTokenizer

![image](https://user-images.githubusercontent.com/52172169/235303431-6997bf0e-2472-47ee-b6e5-8a0130f4a977.png)

<br>

+ CompilationEngine

![image](https://user-images.githubusercontent.com/52172169/235303456-05776cea-54ae-4616-b2a5-38bb3c611a78.png)

![image](https://user-images.githubusercontent.com/52172169/235303464-8e9cdcbb-b076-4a60-9221-4786a73456b0.png)

![image](https://user-images.githubusercontent.com/52172169/235303471-fd25bcad-475b-4c09-b948-439afc651436.png)

<br>

CompilationEngine 작성 시 Expression은 제외한 나머지 부분 먼저 작성해준 다음 Expression 부분 작성하라고 함

<br>

+ JackAnalyzer

![image](https://user-images.githubusercontent.com/52172169/235303497-0888bdf9-4717-416f-a4b8-8704adde9107.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/235303621-92b739c2-b6ea-4fd6-bf1c-d43a7946faee.png)

<br>

+ Handling Expressions

![image](https://user-images.githubusercontent.com/52172169/235303632-4c6bf114-c042-4908-bad7-b84102c3055d.png)

![image](https://user-images.githubusercontent.com/52172169/235303641-69c54016-e8cc-41ca-acfd-269dd18d4e52.png)

<br><br>
<hr style="border: 2px solid;">
<br><br>
