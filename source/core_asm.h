#pragma once

extern "C" __attribute__((visibility("default"))) void asm_call(uint32_t *regs, uint32_t func)
{
	__asm 
	("											;\
		addiu   $sp, $sp, -256					;\
												;\
		sw		$ra, 100($sp)					;\
												;\
		sw		$a0, 4($sp)						;\
		sw		$a1, 8($sp)						;\
		sw		$a2, 12($sp)					;\
		sw		$a3, 16($sp)					;\
		sw		$t0, 20($sp)					;\
		sw		$t1, 24($sp)					;\
		sw		$t2, 28($sp)					;\
		sw		$t3, 32($sp)					;\
		sw		$12, 36($sp)					;\
		sw		$13, 40($sp)					;\
		sw		$14, 44($sp)					;\
		sw		$15, 48($sp)					;\
		sw		$t8, 52($sp)					;\
		sw		$t9, 56($sp)					;\
		sw		$s0, 60($sp)					;\
		sw		$s1, 64($sp)					;\
		sw		$s2, 68($sp)					;\
		sw		$s3, 72($sp)					;\
		sw		$s4, 76($sp)					;\
		sw		$s5, 80($sp)					;\
		sw		$s6, 84($sp)					;\
		sw		$s7, 88($sp)					;\
		sw		$v0, 92($sp)					;\
		sw		$v1, 96($sp)					;\
												;\
		move	$t9, $a0						;\
												;\
		lw		$a0, 0($t9)						;\
		lw		$a1, 4($t9)						;\
		lw		$a2, 8($t9)						;\
		lw		$a3, 12($t9)					;\
		lw		$t0, 16($t9)					;\
		lw		$t1, 20($t9)					;\
		lw		$t2, 24($t9)					;\
		lw		$t3, 28($t9)					;\
		lw		$12, 32($t9)					;\
		lw		$13, 36($t9)					;\
		lw		$14, 40($t9)					;\
		lw		$15, 44($t9)					;\
		lw		$t8, 48($t9)					;\
		/*lw	$t9, 52($t9)*/					;\
		lw		$s0, 56($t9)					;\
		lw		$s1, 60($t9)					;\
		lw		$s2, 64($t9)					;\
		lw		$s3, 68($t9)					;\
		lw		$s4, 72($t9)					;\
		lw		$s5, 76($t9)					;\
		lw		$s6, 80($t9)					;\
		lw		$s7, 84($t9)					;\
		lw		$v0, 88($t9)					;\
		lw		$v1, 92($t9)					;\
												;\
		lw		$t9, 8($sp)						;\
		jalr	$t9								;\
		nop										;\
												;\
		lw		$t9, 4($sp)						;\
												;\
		sw		$a0, 0($t9)						;\
		sw		$a1, 4($t9)						;\
		sw		$a2, 8($t9)						;\
		sw		$a3, 12($t9)					;\
		sw		$t0, 16($t9)					;\
		sw		$t1, 20($t9)					;\
		sw		$t2, 24($t9)					;\
		sw		$t3, 28($t9)					;\
		sw		$12, 32($t9)					;\
		sw		$13, 36($t9)					;\
		sw		$14, 40($t9)					;\
		sw		$15, 44($t9)					;\
		sw		$t8, 48($t9)					;\
		/*sw	$t9, 52($t9)*/					;\
		sw		$s0, 56($t9)					;\
		sw		$s1, 60($t9)					;\
		sw		$s2, 64($t9)					;\
		sw		$s3, 68($t9)					;\
		sw		$s4, 72($t9)					;\
		sw		$s5, 76($t9)					;\
		sw		$s6, 80($t9)					;\
		sw		$s7, 84($t9)					;\
		sw		$v0, 88($t9)					;\
		sw		$v1, 92($t9)					;\
												;\
		lw		$a0, 4($sp)						;\
		lw		$a1, 8($sp)						;\
		lw		$a2, 12($sp)					;\
		lw		$a3, 16($sp)					;\
		lw		$t0, 20($sp)					;\
		lw		$t1, 24($sp)					;\
		lw		$t2, 28($sp)					;\
		lw		$t3, 32($sp)					;\
		lw		$12, 36($sp)					;\
		lw		$13, 40($sp)					;\
		lw		$14, 44($sp)					;\
		lw		$15, 48($sp)					;\
		lw		$t8, 52($sp)					;\
		lw		$t9, 56($sp)					;\
		lw		$s0, 60($sp)					;\
		lw		$s1, 64($sp)					;\
		lw		$s2, 68($sp)					;\
		lw		$s3, 72($sp)					;\
		lw		$s4, 76($sp)					;\
		lw		$s5, 80($sp)					;\
		lw		$s6, 84($sp)					;\
		lw		$s7, 88($sp)					;\
		lw		$v0, 92($sp)					;\
		lw		$v1, 96($sp)					;\
												;\
		lw		$ra, 100($sp)					;\
		addiu   $sp, $sp, 256					;\
		jr		$ra								;\
		nop										;\
	");

}