/* File: main.cc
 * -------------
 * This file defines the main() routine for the program and not much else.
 * You should not need to modify this file.
 */

#include <string.h>
#include <stdio.h>
#include "utility.h"
#include "errors.h"
#include "parser.h"

void SysCallCodeGen();

/* Function: main()
 * ----------------
 * Entry point to the entire program.  We parse the command line and turn
 * on any debugging flags requested by the user when invoking the program.
 * InitScanner() is used to set up the scanner.
 * InitParser() is used to set up the parser. The call to yyparse() will
 * attempt to parse a complete program from the input.
 */
int main(int argc, char *argv[])
{
    ParseCommandLine(argc, argv);

    InitScanner();
    InitParser();
    yyparse();
    if (ReportError::NumErrors() == 0)
        SysCallCodeGen();
    return (ReportError::NumErrors() == 0 ? 0 : -1);
}

void SysCallCodeGen()
{
    printf("  _PrintInt:\n");
    printf("      subu $sp, $sp, 8	# decrement sp to make space to save ra,fp\n");
    printf("      sw $fp, 8($sp)	# save fp\n");
    printf("      sw $ra, 4($sp)	# save ra\n");
    printf("      addiu $fp, $sp, 8	# set up new fp\n");
    printf("      lw $a0, 4($fp)	# fill a to $t0 from $fp+4\n");
    printf("    # LCall _PrintInt\n");
    printf("      li $v0, 1\n");
    printf("      syscall\n");
    printf("    # EndFunc\n");
    printf("    # (below handles reaching end of fn body with no explicit return)\n");
    printf("      move $sp, $fp		# pop callee frame off stack\n");
    printf("      lw $ra, -4($fp)	# restore saved ra\n");
    printf("      lw $fp, 0($fp)	# restore saved fp\n");
    printf("      jr $ra		# return from function\n");
    printf("\n");
    printf("  _ReadInteger:\n");
    printf("      subu $sp, $sp, 8	# decrement sp to make space to save ra,fp\n");
    printf("      sw $fp, 8($sp)	# save fp\n");
    printf("      sw $ra, 4($sp)	# save ra\n");
    printf("      addiu $fp, $sp, 8	# set up new fp\n");
    printf("      li $v0, 5\n");
    printf("      syscall\n");
    printf("    # EndFunc\n");
    printf("    # (below handles reaching end of fn body with no explicit return)\n");
    printf("      move $sp, $fp		# pop callee frame off stack\n");
    printf("      lw $ra, -4($fp)	# restore saved ra\n");
    printf("      lw $fp, 0($fp)	# restore saved fp\n");
    printf("      jr $ra		# return from function\n");
    printf("\n");
    printf("\n");
    printf("  _PrintBool:\n");
    printf("          subu $sp, $sp, 8      # decrement sp to make space to save ra, fp\n");
    printf("          sw $fp, 8($sp)        # save fp\n");
    printf("          sw $ra, 4($sp)        # save ra\n");
    printf("          addiu $fp, $sp, 8     # set up new fp\n");
    printf("          lw $t1, 4($fp)        # fill a from $fp+4\n");
    printf("      li $t2, 0\n");
    printf("          li $v0, 4\n");
    printf("      beq $t1, $t2, _PrintBoolFalse\n");
    printf("      la $a0, _PrintBoolTrueString\n");
    printf("      j _PrintBoolEnd\n");
    printf("    _PrintBoolFalse:\n");
    printf("    	  la $a0, _PrintBoolFalseString\n");
    printf("    _PrintBoolEnd:\n");
    printf("          syscall\n");
    printf("        # EndFunc\n");
    printf("        # (below handles reaching end of fn body with no explicit return)\n");
    printf("          move $sp, $fp         # pop callee frame off stack\n");
    printf("          lw $ra, -4($fp)       # restore saved ra\n");
    printf("          lw $fp, 0($fp)        # restore saved fp\n");
    printf("          jr $ra                # return from function\n");
    printf("\n");
    printf("      .data			# create string constant marked with label\n");
    printf("      _PrintBoolTrueString: .asciiz \"true\"\n");
    printf("      .text\n");
    printf("\n");
    printf("      .data			# create string constant marked with label\n");
    printf("      _PrintBoolFalseString: .asciiz \"false\"\n");
    printf("      .text\n");
    printf("\n");
    printf("  _PrintString:\n");
    printf("          subu $sp, $sp, 8      # decrement sp to make space to save ra, fp\n");
    printf("          sw $fp, 8($sp)        # save fp\n");
    printf("          sw $ra, 4($sp)        # save ra\n");
    printf("          addiu $fp, $sp, 8     # set up new fp\n");
    printf("          lw $a0, 4($fp)        # fill a from $fp+4\n");
    printf("          li $v0, 4\n");
    printf("          syscall\n");
    printf("        # EndFunc\n");
    printf("        # (below handles reaching end of fn body with no explicit return)\n");
    printf("          move $sp, $fp         # pop callee frame off stack\n");
    printf("          lw $ra, -4($fp)       # restore saved ra\n");
    printf("          lw $fp, 0($fp)        # restore saved fp\n");
    printf("          jr $ra                # return from function\n");
    printf("\n");
    printf("  _Alloc:\n");
    printf("    subu $sp, $sp, 8      # decrement sp to make space to save ra,fp\n");
    printf("          sw $fp, 8($sp)        # save fp\n");
    printf("          sw $ra, 4($sp)        # save ra\n");
    printf("          addiu $fp, $sp, 8     # set up new fp\n");
    printf("          lw $a0, 4($fp)        # fill a from $fp+4\n");
    printf("          li $v0, 9\n");
    printf("          syscall\n");
    printf("        # EndFunc\n");
    printf("        # (below handles reaching end of fn body with no explicit return)\n");
    printf("          move $sp, $fp         # pop callee frame off stack\n");
    printf("          lw $ra, -4($fp)       # restore saved ra\n");
    printf("          lw $fp, 0($fp)        # restore saved fp\n");
    printf("          jr $ra                # return from function\n");
    printf("\n");
    printf("  _Halt:\n");
    printf("          li $v0, 10\n");
    printf("          syscall\n");
    printf("        # EndFunc\n");
    printf("\n");
    printf("\n");
    printf("  _StringEqual:\n");
    printf("          subu $sp, $sp, 8      # decrement sp to make space to save ra, fp\n");
    printf("          sw $fp, 8($sp)        # save fp\n");
    printf("          sw $ra, 4($sp)        # save ra\n");
    printf("          addiu $fp, $sp, 8     # set up new fp\n");
    printf("          lw $a0, 4($fp)        # fill a from $fp+4\n");
    printf("          lw $a1, 8($fp)        # fill a from $fp+8\n");
    printf("        beq $a0,$a1,Lrunt10\n");
    printf("        lbu  $a2,($a0)\n");
    printf("        lbu  $a3,($a1)\n");
    printf("        #addiu $a0,$a0,4\n");
    printf("        #addiu $a1,$a1,4\n");
    printf("        beq $a2,$a3,Lrunt11\n");
    printf("  Lrunt13:\n");
    printf("        li  $v0,0\n");
    printf("        # EndFunc\n");
    printf("        # (below handles reaching end of fn body with no explicit return)\n");
    printf("          move $sp, $fp         # pop callee frame off stack\n");
    printf("          lw $ra, -4($fp)       # restore saved ra\n");
    printf("          lw $fp, 0($fp)        # restore saved fp\n");
    printf("        j $ra\n");
    printf("  Lrunt12:\n");
    printf("        lbu  $t0,($a0)\n");
    printf("        lbu  $t1,($a1)\n");
    printf("        bne  $t0,$t1,Lrunt13\n");
    printf("        beqz  $t0,Lrunt10\n");
    printf("        addiu $a0,$a0,1\n");
    printf("        addiu $a1,$a1,1\n");
    printf("        addiu $a2,$a2,-1\n");
    printf("  Lrunt11:\n");
    printf("        bgez $a2,Lrunt12\n");
    printf("  Lrunt10:\n");
    printf("        li $v0,1\n");
    printf("        # EndFunc\n");
    printf("        # (below handles reaching end of fn body with no explicit return)\n");
    printf("          move $sp, $fp         # pop callee frame off stack\n");
    printf("          lw $ra, -4($fp)       # restore saved ra\n");
    printf("          lw $fp, 0($fp)        # restore saved fp\n");
    printf("          jr $ra                # return from function\n");
    printf("\n");
    printf("\n");
    printf("\n");
    printf("  _ReadLine:\n");
    printf("          subu $sp, $sp, 8      # decrement sp to make space to save ra, fp\n");
    printf("          sw $fp, 8($sp)        # save fp\n");
    printf("          sw $ra, 4($sp)        # save ra\n");
    printf("          addiu $fp, $sp, 8     # set up new fp\n");
    printf("          li $a0, 100\n");
    printf("          li $v0, 9\n");
    printf("          syscall\n");
    printf("          addi $a0, $v0, 0\n");
    printf("          li $v0, 8\n");
    printf("          li $a1,100 \n");
    printf("          syscall\n");
    printf("          li $t0,0              # Set index to 0\n");
    printf("  Lrunt21:\n");
    printf("          addu $t1,$a0,$t0      # Load character at index\n");
    printf("          lb $a3,($t1)          # Load character at index\n");
    printf("          addi $t0,$t0,1        # Increment index\n");
    printf("          bnez $a3,Lrunt21      # Loop until the end of string is reached\n");
    printf("          beq $a1,$t0,Lrunt20   # Do not remove \\n when string = maxlength\n");
    printf("          sb $0,-1($t1)         # Add the terminating character in its place\n");
    printf("  Lrunt20:\n");
    printf("          addi $v0, $a0, 0\n");
    printf("        # EndFunc\n");
    printf("        # (below handles reaching end of fn body with no explicit return)\n");
    printf("          move $sp, $fp         # pop callee frame off stack\n");
    printf("          lw $ra, -4($fp)       # restore saved ra\n");
    printf("          lw $fp, 0($fp)        # restore saved fp\n");
    printf("          jr $ra                # return from function\n");
}
