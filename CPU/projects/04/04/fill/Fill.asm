// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input.
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel;
// the screen should remain fully black as long as the key is pressed. 
// When no key is pressed, the program clears the screen, i.e. writes
// "white" in every pixel;
// the screen should remain fully clear as long as no key is pressed.

// screen = 16bit word 32 column per raw, total 256 raw
@SCREEN
D=A
@base   // base = 16384(SCREEN)
M=D

(MAIN)
    @i
    M=0
    @KBD
    D=M
    // blacken if KBD != 0, else clear
    @BLACKEN
    D;JNE
    @CLEAR
    0;JMP

(BLACKEN)   // 16384 ~ 24575
    @8192
    D=A
    @i
    D=D-M
    @MAIN
    D;JEQ

    @base
    D=M
    @i
    A=D+M  // RAM[16384+i]
    M=-1
    @i
    M=M+1
    @BLACKEN
    0;JMP

(CLEAR)     // 16384 ~ 24575
    @8192
    D=A
    @i
    D=D-M
    @MAIN
    D;JEQ
    
    @base
    D=M
    @i
    A=D+M  // RAM[16384+i]
    M=0
    @i
    M=M+1
    @CLEAR
    0;JMP