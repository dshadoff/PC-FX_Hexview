/*
  readaddr.s - function to read a specific hardware address on the PC-FX
  (c) 2024 by Dave Shadoff
*/

.macro movw   data, reg1
       movhi  hi(\data),r0,\reg1
       movea  lo(\data),\reg1,\reg1
.endm

/*
   export the functions so they can be accessed by the linker
*/
       .global _read_addr


.equiv r_tmp,   r8
.equiv r_base1, r9

/*********************************/
/* Remember calling conventions: */
/* ----------------------------- */
/* r6  is entry parameter #1     */
/* r10 is return value           */
/* lp  is return address         */
/*********************************/


/************************************************/
/* extern u8 read_addr(u32 address);            */
/************************************************/
_read_addr:
       mov   lp, r18
       ld.b  0[r6], r10
       mov   r18, lp
       jmp   [lp]
