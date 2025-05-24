bits 16 ; We are in 16-bit real mode

GLOBAL plot_pixel, draw_rect, draw_home_screen

; Function: plot_pixel
; Description: Plots a pixel on the screen in mode 13h.
; Arguments:
;   cx: x-coordinate
;   dx: y-coordinate
;   al: color value
; Clobbers: ax, bx, di (es is preserved by plot_pixel itself)
plot_pixel:
    push bp         ; Preserve caller's bp
    mov bp, sp      ; Set up stack frame for this function

    ; Calculate video memory offset: (y * 320) + x
    mov ax, dx      ; ax = y (argument dx)
    mov bx, dx      ; bx = y (argument dx)

    shl ax, 8       ; ax = y * 256 (y << 8)
    shl bx, 6       ; bx = y * 64  (y << 6)
    add ax, bx      ; ax = (y * 256) + (y * 64) = y * 320

    add ax, cx      ; ax = (y * 320) + x (argument cx) ; final offset in ax

    ; Write color to video memory A000:[offset]
    push es         ; Preserve ES register
    mov bx, 0xA000
    mov es, bx      ; Set ES to video memory segment
    mov di, ax      ; Load offset into DI
    ; AL (argument al) already contains the color

    stosb           ; Store AL at ES:[DI] and increment DI.

    pop es          ; Restore ES register

    pop bp          ; Restore caller's bp
    ret             ; Return from function

; Function: draw_rect
; Description: Draws a filled rectangle on the screen.
; Arguments:
;   ax: x_start
;   bx: y_start
;   cx: width
;   dx: height
;   si: color
; Clobbers: ax, bx, cx, dx, si, di, bp (registers used within this function)
draw_rect:
    ; Preserve registers that this function uses for its own loop counters/logic
    ; and are not return values or part of the calling convention beyond this function.
    push bp     ; Used for y_current
    push di     ; Used for x_current
    ; Input parameters ax, bx, cx, dx, si are used directly or modified.

    ; Calculate y_end = y_start + height. dx will hold y_end.
    add dx, bx  ; dx = original_bx (y_start) + original_dx (height)
    ; Calculate x_end = x_start + width. cx will hold x_end.
    add cx, ax  ; cx = original_ax (x_start) + original_cx (width)

    mov bp, bx  ; bp = current_y, starting at y_start (original bx)

.y_loop:
    cmp bp, dx  ; Compare current_y (bp) with y_end (dx)
    jge .draw_rect_done ; If current_y >= y_end, rectangle is drawn

    mov di, ax  ; di = current_x, starting at x_start (original ax)
.x_loop:
    cmp di, cx  ; Compare current_x (di) with x_end (cx)
    jge .x_loop_done ; If current_x >= x_end, this row is drawn
    
    ; Call plot_pixel(current_x, current_y, color)
    ; plot_pixel expects: cx=x, dx=y, al=color
    
    pusha       ; Save all general purpose registers of draw_rect before calling plot_pixel
    
    mov cx, di  ; Set x for plot_pixel (current_x from di)
    mov dx, bp  ; Set y for plot_pixel (current_y from bp)
    mov al, si  ; Set color for plot_pixel (original color from input si)
                ; Note: si is not modified by plot_pixel, but this ensures al has the color.
    
    call plot_pixel
    
    popa        ; Restore all general purpose registers of draw_rect

    inc di      ; next current_x
    jmp .x_loop
.x_loop_done:
    inc bp      ; next current_y
    jmp .y_loop

.draw_rect_done:
    pop di      ; Restore original di
    pop bp      ; Restore original bp
    ret

; Function: draw_home_screen
; Description: Draws a predefined home screen layout.
; Arguments: None
; Clobbers: ax, bx, cx, dx, si (used by draw_rect)
draw_home_screen:
    pusha ; Save all registers, as this function will make multiple calls to draw_rect

    ; Status Bar: x=0, y=0, width=320, height=20, color=Light Gray (7)
    mov ax, 0       ; x_start
    mov bx, 0       ; y_start
    mov cx, 320     ; width
    mov dx, 20      ; height
    mov si, 7       ; color Light Gray
    call draw_rect

    ; Main Content Area Background: x=0, y=20, width=320, height=130, color=Dark Gray (8)
    mov ax, 0       ; x_start
    mov bx, 20      ; y_start
    mov cx, 320     ; width
    mov dx, 130     ; height
    mov si, 8       ; color Dark Gray
    call draw_rect

    ; Dock: x=0, y=150, width=320, height=50, color=Light Gray (7)
    mov ax, 0       ; x_start
    mov bx, 150     ; y_start
    mov cx, 320     ; width
    mov dx, 50      ; height
    mov si, 7       ; color Light Gray (using Light Gray for dock as per previous decision)
    call draw_rect

    ; Icon 1 (Placeholder App Icon): x=20, y=30, width=40, height=40, color=Red (4)
    mov ax, 20      ; x_start
    mov bx, 30      ; y_start
    mov cx, 40      ; width
    mov dx, 40      ; height
    mov si, 4       ; color Red
    call draw_rect

    ; Icon 2 (Placeholder App Icon): x=70, y=30, width=40, height=40, color=Green (2)
    mov ax, 70      ; x_start
    mov bx, 30      ; y_start
    mov cx, 40      ; width
    mov dx, 40      ; height
    mov si, 2       ; color Green
    call draw_rect
    
    popa  ; Restore all registers
    ret
