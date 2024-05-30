section .bss
    struct: resd 1
    buffer_reader: resb 600

section .data
    x_struct: db 5
    x_num: db 0xaa, 1,2,0x44,0x4f
    y_struct: db 6
    y_num: db 0xaa, 1,2,3,0x44,0x4f
    test: db "hello" , 0
    STATE: dw 0xACE1
    smaller: db 0
    part1_message: db "part1", 0 
    part2_message: db "part2", 0 
    part3_message: db "part3", 0 
    bigger: db 0
    MASK: dw 0x002D
    newline: db 10,0
    print_format: db "%02hhx", 0

section .text
global main
extern malloc
extern strlen
extern fgets
extern stdin
extern printf
extern free
extern puts
main:
stast:

    push ebp
    mov ebp, esp
    mov eax, [ebp+8]        ;argc
    mov ebx, [ebp+12]       ;argv
comparings:
    cmp eax, 1              
    je no_arguments          
    mov eax, [ebx + 4]       
    cmp word[eax], "-R"      ;"-R"
    je R
    cmp word[eax], "-I"      ;"-I"
    je I

    jmp exit

part1:
    add ecx,4
    dec ecx
    push part1_message
    call puts
    add esp,4
    ret

exit: ;exiting program
    pop ebp
    ret    

no_arguments:
    call part1
    push x_struct           
    push y_struct          
    call add_multi         
    push eax               
    call print_multi       
    call free              
    add esp, 12            
    pop ebp
    ret                    

part2:
    add ecx,4
    dec ecx
    push part2_message
    call puts
    add esp,4
    ret
I:
    call part2
    call getmulti           
    push eax              
    call getmulti         
    push eax               
    call add_multi        
    push eax             
    call print_multi       
    call free               
    add esp, 4             
    call free              
    add esp, 4                     
    call free             
    add esp, 4            
    pop ebp
    ret       
part3:
    add ecx,4
    dec ecx
    push part3_message
    call puts
    add esp,4
    ret
R:
    call part3
    call PRmulti            
    push eax               
    call PRmulti            
    push eax               
    call add_multi         
    push eax                
    call print_multi        
    call free               
    add esp, 4             
    call free              
    add esp, 4                     
    call free             
    add esp, 4            
    pop ebp
    ret                                                                             

;/////////////////////   Task 1 a ///////////////////////////

print_multi:
    push ebp                  
    mov ebp, esp               
    pushad                      
    mov edi, [ebp+8]            
    movzx ebx, byte[edi]        ;struct length (struct.size)

;loop over the struct
looper:

    movzx ecx, byte[edi + ebx]  ;always taking from the end (little endian)
    push ecx                    
    push print_format                 
    call printf        
    add esp, 8                  
    dec ebx               ;next     
    cmp bl, 0                   
    jne looper              ;continue if not the end
    
printing_new_line:
    push newline                ;"\n"
    call printf                 
    add esp, 4                  
    popad                       
    pop ebp                    
    ret                         ;return to main

;/////////////////////   Task 1 b ///////////////////////////

 getmulti:                                                                                                                                                                                                                                                                                                                                                                                      
    push ebp                   
    mov ebp, esp       
    pushad        


input_reading:    
    push dword[stdin]          
    push 600                  
    push buffer_reader              
    call fgets                  ;fgets(buffer_reader, 600, stdin)
    add esp, 12     

input_size:
    push buffer_reader                
    call strlen                 ;eax = strlen(buffer_reader);
    add esp, 4                 
    mov edi, eax    



resign_the_size:        
    dec edi
    dec edi                
    shr eax, 1                  
    inc eax 


malloc_for_new_struct:             
    push eax                    
    call malloc                 ;eax = malloc(strlen result)
    mov dword[struct], eax      
    mov esi, eax                
    pop eax                     
    dec eax                    
    mov byte[esi], al         
    mov ecx, 1     



parsing_input:
    mov ebx, 0                  ;reset
    mov bh, byte[buffer_reader + edi]  ;take the first char
    dec edi                     
    
    cmp bh, 'a'                 ;check if the digit is a number or letter
    jge character                  
    sub bh, '0'                
    jmp swaping




character:
    sub bh, 'a'                
    add bh, 0xa                 




swaping:
    mov bl, bh                  
    mov bh, 0                  
    cmp edi, 0
    jl construct
    mov bh, byte[buffer_reader + edi]  ;take the second char
    dec edi                     
    cmp bh, 'a'                 ;check if the second digit is a number or letter
    jge second_character                   
    add bh, 0xa       





second_character:
    sub bh, 'a'                
    add bh, 0xa                



construct:
    shl bh, 4                 
    or bl, bh                 



add_struct:
    mov byte[esi + ecx], bl    
    inc ecx                    
    cmp edi, 0                  
    jge parsing_input           ;jump to end if we finished


end:
    jmp saving_value                      
    ret                                               

;/////////////////////   Task 2 a ///////////////////////////

MaxMin:
    movzx edx, byte[ebx]      
    movzx ecx, byte[eax]                
    cmp ecx, edx                 
    jae no_swap                     ;jae checks unsigned numbers
    xchg eax, ebx             
    no_swap:            
    ret                           


;/////////////////////   Task 2 b ///////////////////////////

add_multi:
    push ebp              
    mov ebp, esp              
    pushad                     
    mov eax, [ebp+8]            
    mov ebx, [ebp+12]          
    

print_inputs:
    push ebx                    
    call print_multi            ;print_multi(second input) 
    add esp, 4   
    push eax                  
    call print_multi            ;print_multi(first input) 
    add esp, 4              
    call MaxMin                 ;swap eax and ebx so that eax is the bigger number              

saving_each_input_for_looping:
    mov edi, ebx    
    mov esi, eax                 
    movzx eax, byte[edi]        
    mov byte[smaller], al 
    movzx eax, byte[esi]        
    mov byte[bigger], al   
    inc eax
    inc eax              
    push eax                  
    call malloc                
    mov dword[struct], eax    
    pop ecx                    
    sub ecx,1                     
    mov byte[eax], cl   

update_values:        
    mov ecx, 0                  
    mov edx, 0                 
    inc esi                     ;esi is now pointed to the first input num
    inc edi                     ;edi is now pointed to the second input num
    inc eax                     ;eax is now pointed to the result  num


loop_on_nums_for_adding:

adding_and_carry_handling:
    movzx ebx, byte[esi]        ;first num in first array num
    add ebx, ecx                
    movzx ecx, byte[edi]        ;first num in second array num
    add ebx, ecx               
    mov cl, bh                  
    mov byte[eax], bl           ;save the result of the addition

update_the_values:
    add edx,1                    
    add esi,1                    ;esi is now pointed to the next byte in first input num
    add edi,1                    ;edi is now pointed to the next byte in second input num
    add eax,1                    ;eax is now pointed to the next byte in result  num
    cmp dl, byte[smaller]  
    jne loop_on_nums_for_adding            

    cmp dl, byte[bigger]  
    je skip

loop_on_nums_for_adding_for_second_byte:
    movzx ebx, byte[esi]        
    add ebx, ecx             
    mov cl, bh                  
    mov byte[eax], bl

update_the_values2:         
    add edx,1                    
    add esi,1                   
    add eax,1                     
    cmp dl, byte[bigger]         
    jne loop_on_nums_for_adding_for_second_byte

skip:
    mov byte[eax], cl         
    jmp saving_value       
    ret                        

;/////////////////////   Task 3  ///////////////////////////               

testing_perpose:
    push ebp                   
    mov ebp, esp                
    pushad  
    push test
    call printf

PRmulti:
    push ebp                   
    mov ebp, esp                
    pushad                      

looping_a_random_size:
    call rand_num                            ;return a random number    
    cmp al, 0                 
    je looping_a_random_size                               ;repeat till the size isn't zero

    movzx ebx, al               
    add ebx, 1                  
    push ebx                   
    call malloc                              ;calling malloc(size=bl)
    mov dword[struct], eax      
    pop ebx                     
    sub ebx,1                    
    mov byte[eax], bl           
    mov esi, eax              
    mov edx, 0                  
    
    looping_a_random_array_num_for_struct:
        call rand_num                        ;return a random number 
        mov byte[esi + edx + 1], al 
        inc edx                
        sub ebx,1                 
        jnz looping_a_random_array_num_for_struct   ;continue till we are not done
    
    jmp saving_value       
    ret                         



rand_num:
    push ebp                    
    mov ebp, esp               
    pushad                     
random_generator:
    mov bx, 0              
    movzx eax, word[STATE] 
    and ax, [MASK]          
    jnp parity            
    mov bx, 0x8000          
parity:
    movzx eax, word[STATE]  
    shr ax, 1               
    or ax, bx              
    mov word[STATE], ax    
saving_value_and:
    popad                     
    pop ebp                     
    movzx eax, word[STATE]      
    ret        


saving_value:
    popad                      
    pop ebp                    
    mov eax, [struct]           
    ret   

_exit:
endd:
    pop ebp
    ret    


