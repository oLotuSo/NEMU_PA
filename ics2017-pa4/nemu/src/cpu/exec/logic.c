#include "cpu/exec.h"

make_EHelper(test) {
  rtl_and(&t2, &id_dest->val, &id_src->val);

  rtl_li(&t1,0);
  rtl_set_CF(&t1);
  rtl_set_OF(&t1);

  rtl_update_ZFSF(&t2, id_dest->width);

  print_asm_template2(test);
}

make_EHelper(and) {
  rtl_and(&t2, &id_dest->val, &id_src->val); 
  operand_write(id_dest, &t2); 
 
  rtl_li(&t1,0);
  rtl_set_OF(&t1); 
  rtl_set_CF(&t1); 

  rtl_update_ZFSF(&t2, id_dest->width); 

  print_asm_template2(and);
}

make_EHelper(xor) {
  rtl_xor(&t2,&id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);

  rtl_li(&t1,0);
  rtl_set_CF(&t1);
  rtl_set_OF(&t1);

  rtl_update_ZFSF(&t2,id_dest->width);

  print_asm_template2(xor);
}

make_EHelper(or) {
  rtl_or(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);

  rtl_li(&t1,0);
  rtl_set_CF(&t1);
  rtl_set_OF(&t1);

  rtl_update_ZFSF(&t2, id_dest->width);

  print_asm_template2(or);
}

make_EHelper(sar) {
  rtl_sar(&t1, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t1);
  rtl_update_ZFSF(&t1,id_dest->width);
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(sar);
}

make_EHelper(shl) {
  rtl_shl(&t1, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t1);
  rtl_update_ZFSF(&t1, id_dest->width);
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(shl);
}

make_EHelper(shr) {
  rtl_shr(&t1, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t1);
  rtl_update_ZFSF(&t1, id_dest->width);
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(shr);
}

make_EHelper(rol){
  rtl_shri(&t2, &id_dest->val, id_dest->width * 8 - id_src->val);
	rtl_shl(&t3, &id_dest->val, &id_src->val);
	rtl_or(&t1, &t2, &t3);
	operand_write(id_dest, &t1);

  print_asm_template2(rol);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  rtl_li(&t0, id_dest->val);
  rtl_not(&t0);
  operand_write(id_dest, &t0);

  print_asm_template1(not);
}
