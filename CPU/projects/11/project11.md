# Project 11
## Program Compilation
---

<br>

![image](https://user-images.githubusercontent.com/52172169/235931745-558fced2-74a7-4470-ac66-8a549269e52b.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/235931825-bc5a102f-bbf9-4508-9b4f-0b3f8b3d6698.png)

<br><br>

### Handling variables
---

<br>

![image](https://user-images.githubusercontent.com/52172169/235932022-d85ad04a-0b11-4311-8197-b3aae9136112.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/235932691-85db4f20-1174-4c62-a635-6c9b267428cc.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/235932840-d932cc80-cd29-4503-9dd5-3364b041cc59.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/235933081-9c054d4e-3719-4b72-8893-b7ae3c3f9519.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/235933812-dd0df5c0-c6c7-4b6d-ad94-9afb0e9dfd35.png)

<br><br>

### Handling expressions
---

<br>

![image](https://user-images.githubusercontent.com/52172169/235934421-9595d304-de2a-4809-86e7-a4a19cbd0f8e.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/235934462-9e8d1c75-bb59-4553-9cc8-96ddaf93f733.png)

<br>

그래서 expression과 term 컴파일 함수에서 infix 형태를 postfix 형태로 출력해주게끔 코드를 다시 짜주라는 듯함.

<br>

![image](https://user-images.githubusercontent.com/52172169/235935209-e7f55aa2-269a-424e-8a02-ffe03d972555.png)

<br>

Jack Compiler는 operator 간에 우선순위가 없어서 ```5+3*2```가 16이 나옴. 그래도 괄호 안에 있는 연산자는 먼저 처리하긴 함

<br>

![image](https://user-images.githubusercontent.com/52172169/235937123-91c5b99e-b9ae-45b9-ae67-73f77c004c9d.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/235937321-9a745505-2bf1-40a3-92e6-9d4fd5908ba5.png)

<br><br>

### Handling statements
---

<br>

![image](https://user-images.githubusercontent.com/52172169/235937463-9b20afbd-f0b5-4a38-836b-0e3eb3bf582f.png)

<br>

+ let

![image](https://user-images.githubusercontent.com/52172169/235937997-d9420c13-0ec3-4b9a-80ce-2ae8bd6417e9.png)

<br>

+ return

![image](https://user-images.githubusercontent.com/52172169/235938607-6cd2a20c-074f-4b42-9971-f1b752b9b992.png)

<br>

+ do

![image](https://user-images.githubusercontent.com/52172169/235938871-49b51766-7b38-4cc4-92c5-c1f26f0a97f3.png)

<br>

+ if

![image](https://user-images.githubusercontent.com/52172169/235948684-c90c9cb1-6a38-4190-ac44-8cd7acbcc9e1.png)

![image](https://user-images.githubusercontent.com/52172169/235948777-22574ae3-cd05-4ab9-a7ad-d3830e20bf79.png)

<br>

+ while

![image](https://user-images.githubusercontent.com/52172169/235948912-834628c9-da21-4e80-93f4-a3c46109c2fe.png)

![image](https://user-images.githubusercontent.com/52172169/235948963-da6cb4ed-a103-4a15-93ba-f3d981175bb4.png)

<br><br>

### Handling objects
---

<br>

오브젝트 생성 시 ```Memory.alloc(n)```을 이용해서 만들어주는 듯함. 해서 그에 따른 vm 코드는 다음과 같이 생성해주면 된다고 함.

<br>

![image](https://user-images.githubusercontent.com/52172169/235955147-201fd67f-8376-409e-8412-a1ddc64e20b7.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/235955238-ad80e8bc-54cd-44d6-aaa2-a109dd855715.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/235956581-3c772101-8d12-493a-bfbb-642ca753501f.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/235957047-7be9e250-8ebd-4e52-bc70-79b8e45d46ba.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/235959702-f1618422-2d87-4745-8018-f89639fd7a13.png)

<br>

따라서 컴파일 시 Memory.alloc을 이용해서 constructor나 method 등등을 처리해주는 모습을 볼 수 있음.

<br>

![image](https://user-images.githubusercontent.com/52172169/235960928-8ee523c4-2785-4d01-bd5c-c0933d5f6b62.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/236438837-caaa1fd0-1924-4328-aa74-96d13ef39502.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/236445872-cfad0baf-e4ce-44e1-8bd4-81a9aa70b55e.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/236446108-3644d33b-9223-4964-812d-ab859771651e.png)

<br><br>

### Handling arrays
---

<br>

![image](https://user-images.githubusercontent.com/52172169/236447060-c69af45d-7eae-4c08-9c9d-6bef364fa765.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/236448093-99724177-30e1-44a7-ad02-265dbe27fea5.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/236451175-6d58f034-2f06-490b-97c7-4417e60ca727.png)

<br>

그러나 문제가 있는데 바로 위의 일반식대로 작성하면 만약 두 개의 array가 있다면 pop pointer 1 부분에서 에러가 발생할 것이다. 

따라서 아래처럼 컴파일 코드를 짜줘야한다.

<br>

![image](https://user-images.githubusercontent.com/52172169/236451368-44664ed8-7a3e-4f87-8a4a-2f63accc025d.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/236451810-c6c9a5a1-3b91-4de6-a1a9-c1e776777657.png)

<br><br>
<hr style="border: 2px solid;">
<br><br>

## Generate code
### big picture
---

<br>

![image](https://user-images.githubusercontent.com/52172169/236458443-94d2da89-451d-4f53-88dd-484200447dfe.png)

<br><br>

### Subroutines
---

<br>

![image](https://user-images.githubusercontent.com/52172169/236458479-e8193baf-a497-48ab-b1a6-31c36eb284cc.png)

<br><br>

### Constants
---

<br>

![image](https://user-images.githubusercontent.com/52172169/236458675-1388b1b1-d528-4b08-83d8-2d565bd9a058.png)

<br><br>

### Variables
---

<br>

![image](https://user-images.githubusercontent.com/52172169/236458711-589fb51f-676e-46eb-b25f-4167b99c2c50.png)

<br><br>

### Arrays
---

<br>

![image](https://user-images.githubusercontent.com/52172169/236458802-21357157-79fc-4ffa-8f44-2db0e3c9ed68.png)

<br><br>

### Subroutine Calls
---

<br>

9장에 있는 자료와 같이 봐야함. 또한 내용도 자세히 살펴봐야 함.

예를 들면, method call을 처리할 때, ```push obj``` 부분은 ```push pointer 0```를 하라는 뜻임.

왜냐하면, method call 할 때, ```push argument 0``` 후 ```pop pointer 0``` 코드가 작성되기 때문임. (추후에 vm코드 디버깅할 때 알게됨)

<br>

![image](https://github.com/ind2x/redcat_junior/assets/52172169/49d92732-f2be-414d-be08-6a8742e5b46e)

<br>

![image](https://user-images.githubusercontent.com/52172169/236458901-67b4cd3d-30bb-4120-83ed-44f48048fc6d.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/236460001-c7bfd5e8-ae16-4f77-b25f-4a2409eff905.png)

<br><br>

### OS Functions
---

<br>

![image](https://user-images.githubusercontent.com/52172169/236460251-1d4335f9-439b-44ff-a719-deee1dab57b4.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/236460332-bb2ca93d-9c79-4489-b5e7-2667b69186bd.png)

<br><br>
<hr style="border: 2px solid;">
<br><br>

## Design
### Jack Compiler
---

<br>

![image](https://user-images.githubusercontent.com/52172169/236460781-0d4ad1cc-8086-4aa7-8ede-e615e237be99.png)

<br><br>

### Symbol Table
---

<br>

![image](https://user-images.githubusercontent.com/52172169/236460909-4153a268-f36b-49ff-8023-d6d9cbd32dbc.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/236460966-4f49ddb5-a66c-4c4c-82b5-1738b91f20ba.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/236461071-c1d07547-6b67-49ab-b9b1-4b0b318e4df4.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/236461125-f5c902c2-2b68-4ee8-93be-fbf1aaba4fc5.png)

<br><br>

### VMWriter
---

<br>

![image](https://user-images.githubusercontent.com/52172169/236461207-6b2c4e3b-cdc9-433c-b704-9ed45f333b33.png)

<br><br>

### CompilationEngine
---

<br>

이제는 xml코드에서 vm코드로 바꿔줘야 함.

<br>

![image](https://user-images.githubusercontent.com/52172169/236461385-0aad740a-9539-4300-a1ee-8d1b92ba460a.png)

<br><br>
<hr style="border: 2px solid;">
<br><br>

## Develop-Step
---

<br>

![image](https://user-images.githubusercontent.com/52172169/236463722-fec0814c-5fd4-4df9-9977-c94525ac2d73.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/236463546-9504a80c-87a3-40ca-986f-457dc3e0ce1c.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/236463789-b5977c30-07d5-4c84-90f5-25c482625058.png)

<br><br>
<hr style="border: 2px solid;">
<br><br>
