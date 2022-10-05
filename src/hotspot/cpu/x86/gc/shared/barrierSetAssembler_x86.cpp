/*
 * Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#include "precompiled.hpp"
#include "gc/shared/barrierSetAssembler.hpp"
#include "gc/shared/collectedHeap.hpp"
#include "interpreter/interp_masm.hpp"
#include "runtime/jniHandles.hpp"
#include "runtime/thread.hpp"

#define __ masm->

address BarrierSetAssembler::_rdma_load_barrier = NULL;

void BarrierSetAssembler::access_pre_barrier(MacroAssembler* masm, Address oop, Register tmp1) {
  // do nothing for now
  assert(false, "Should not reach here");
}

void BarrierSetAssembler::load_at(MacroAssembler* masm, DecoratorSet decorators, BasicType type,
                                  Register dst, Address src, Register tmp1, Register tmp_thread) {
  bool in_heap = (decorators & IN_HEAP) != 0;
  bool in_native = (decorators & IN_NATIVE) != 0;
  bool is_not_null = (decorators & IS_NOT_NULL) != 0;
  bool atomic = (decorators & MO_RELAXED) != 0;

  Label load_done;
  // increase access counter, if remote then rdma read and save to dst
  // potential_load_from_remote(masm, decorators, type, dst, src, tmp1, tmp_thread, load_done);

  switch (type) {
  case T_OBJECT:
  case T_ARRAY: {
    if (in_heap) {
#ifdef _LP64
      if (UseCompressedOops) {
        __ movl(dst, src);
        if (is_not_null) {
          __ decode_heap_oop_not_null(dst);
        } else {
          __ decode_heap_oop(dst);
        }
      } else
#endif
      {
        __ movptr(dst, src);
      }
    } else {
      assert(in_native, "why else?");
      __ movptr(dst, src);
    }
    break;
  }
  case T_BOOLEAN: __ load_unsigned_byte(dst, src);  break;
  case T_BYTE:    __ load_signed_byte(dst, src);    break;
  case T_CHAR:    __ load_unsigned_short(dst, src); break;
  case T_SHORT:   __ load_signed_short(dst, src);   break;
  case T_INT:     __ movl  (dst, src);              break;
  case T_ADDRESS: __ movptr(dst, src);              break;
  case T_FLOAT:
    assert(dst == noreg, "only to ftos");
    __ load_float(src);
    break;
  case T_DOUBLE:
    assert(dst == noreg, "only to dtos");
    __ load_double(src);
    break;
  case T_LONG:
    assert(dst == noreg, "only to ltos");
#ifdef _LP64
    __ movq(rax, src);
#else
    if (atomic) {
      __ fild_d(src);               // Must load atomically
      __ subptr(rsp,2*wordSize);    // Make space for store
      __ fistp_d(Address(rsp,0));
      __ pop(rax);
      __ pop(rdx);
    } else {
      __ movl(rax, src);
      __ movl(rdx, src.plus_disp(wordSize));
    }
#endif
    break;
  default: Unimplemented();
  }
  __ bind(load_done);
}

void BarrierSetAssembler::store_at(MacroAssembler* masm, DecoratorSet decorators, BasicType type,
                                   Address dst, Register val, Register tmp1, Register tmp2) {
  bool in_heap = (decorators & IN_HEAP) != 0;
  bool in_native = (decorators & IN_NATIVE) != 0;
  bool is_not_null = (decorators & IS_NOT_NULL) != 0;
  bool atomic = (decorators & MO_RELAXED) != 0;

  switch (type) {
  case T_OBJECT:
  case T_ARRAY: {
    if (in_heap) {
      if (val == noreg) {
        assert(!is_not_null, "inconsistent access");
#ifdef _LP64
        if (UseCompressedOops) {
          __ movl(dst, (int32_t)NULL_WORD);
        } else {
          __ movslq(dst, (int32_t)NULL_WORD);
        }
#else
        __ movl(dst, (int32_t)NULL_WORD);
#endif
      } else {
#ifdef _LP64
        if (UseCompressedOops) {
          assert(!dst.uses(val), "not enough registers");
          if (is_not_null) {
            __ encode_heap_oop_not_null(val);
          } else {
            __ encode_heap_oop(val);
          }
          __ movl(dst, val);
        } else
#endif
        {
          __ movptr(dst, val);
        }
      }
    } else {
      assert(in_native, "why else?");
      assert(val != noreg, "not supported");
      __ movptr(dst, val);
    }
    break;
  }
  case T_BOOLEAN:
    __ andl(val, 0x1);  // boolean is true if LSB is 1
    __ movb(dst, val);
    break;
  case T_BYTE:
    __ movb(dst, val);
    break;
  case T_SHORT:
    __ movw(dst, val);
    break;
  case T_CHAR:
    __ movw(dst, val);
    break;
  case T_INT:
    __ movl(dst, val);
    break;
  case T_LONG:
    assert(val == noreg, "only tos");
#ifdef _LP64
    __ movq(dst, rax);
#else
    if (atomic) {
      __ push(rdx);
      __ push(rax);                 // Must update atomically with FIST
      __ fild_d(Address(rsp,0));    // So load into FPU register
      __ fistp_d(dst);              // and put into memory atomically
      __ addptr(rsp, 2*wordSize);
    } else {
      __ movptr(dst, rax);
      __ movptr(dst.plus_disp(wordSize), rdx);
    }
#endif
    break;
  case T_FLOAT:
    assert(val == noreg, "only tos");
    __ store_float(dst);
    break;
  case T_DOUBLE:
    assert(val == noreg, "only tos");
    __ store_double(dst);
    break;
  case T_ADDRESS:
    __ movptr(dst, val);
    break;
  default: Unimplemented();
  }
}

#ifndef _LP64
void BarrierSetAssembler::obj_equals(MacroAssembler* masm,
                                     Address obj1, jobject obj2) {
  __ cmpoop_raw(obj1, obj2);
}

void BarrierSetAssembler::obj_equals(MacroAssembler* masm,
                                     Register obj1, jobject obj2) {
  __ cmpoop_raw(obj1, obj2);
}
#endif
void BarrierSetAssembler::obj_equals(MacroAssembler* masm,
                                     Register obj1, Address obj2) {
  __ cmpptr(obj1, obj2);
}

void BarrierSetAssembler::obj_equals(MacroAssembler* masm,
                                     Register obj1, Register obj2) {
  __ cmpptr(obj1, obj2);
}

void BarrierSetAssembler::try_resolve_jobject_in_native(MacroAssembler* masm, Register jni_env,
                                                        Register obj, Register tmp, Label& slowpath) {
  __ clear_jweak_tag(obj);
  __ movptr(obj, Address(obj, 0));
}

void BarrierSetAssembler::tlab_allocate(MacroAssembler* masm,
                                        Register thread, Register obj,
                                        Register var_size_in_bytes,
                                        int con_size_in_bytes,
                                        Register t1,
                                        Register t2,
                                        Label& slow_case) {
  assert_different_registers(obj, t1, t2);
  assert_different_registers(obj, var_size_in_bytes, t1);
  Register end = t2;
  if (!thread->is_valid()) {
#ifdef _LP64
    thread = r15_thread;
#else
    assert(t1->is_valid(), "need temp reg");
    thread = t1;
    __ get_thread(thread);
#endif
  }

  __ verify_tlab();

  __ movptr(obj, Address(thread, JavaThread::tlab_top_offset()));
  if (var_size_in_bytes == noreg) {
    __ lea(end, Address(obj, con_size_in_bytes));
  } else {
    __ lea(end, Address(obj, var_size_in_bytes, Address::times_1));
  }
  __ cmpptr(end, Address(thread, JavaThread::tlab_end_offset()));
  __ jcc(Assembler::above, slow_case);

  // update the tlab top pointer
  __ movptr(Address(thread, JavaThread::tlab_top_offset()), end);

  // recover var_size_in_bytes if necessary
  if (var_size_in_bytes == end) {
    __ subptr(var_size_in_bytes, obj);
  }
  __ verify_tlab();
}

// Defines obj, preserves var_size_in_bytes
void BarrierSetAssembler::eden_allocate(MacroAssembler* masm,
                                        Register thread, Register obj,
                                        Register var_size_in_bytes,
                                        int con_size_in_bytes,
                                        Register t1,
                                        Label& slow_case) {
  assert(obj == rax, "obj must be in rax, for cmpxchg");
  assert_different_registers(obj, var_size_in_bytes, t1);
  if (!Universe::heap()->supports_inline_contig_alloc()) {
    __ jmp(slow_case);
  } else {
    Register end = t1;
    Label retry;
    __ bind(retry);
    ExternalAddress heap_top((address) Universe::heap()->top_addr());
    __ movptr(obj, heap_top);
    if (var_size_in_bytes == noreg) {
      __ lea(end, Address(obj, con_size_in_bytes));
    } else {
      __ lea(end, Address(obj, var_size_in_bytes, Address::times_1));
    }
    // if end < obj then we wrapped around => object too long => slow case
    __ cmpptr(end, obj);
    __ jcc(Assembler::below, slow_case);
    __ cmpptr(end, ExternalAddress((address) Universe::heap()->end_addr()));
    __ jcc(Assembler::above, slow_case);
    // Compare obj with the top addr, and if still equal, store the new top addr in
    // end at the address of the top addr pointer. Sets ZF if was equal, and clears
    // it otherwise. Use lock prefix for atomicity on MPs.
    __ locked_cmpxchgptr(end, heap_top);
    __ jcc(Assembler::notEqual, retry);
    incr_allocated_bytes(masm, thread, var_size_in_bytes, con_size_in_bytes, thread->is_valid() ? noreg : t1);
  }
}

void BarrierSetAssembler::incr_allocated_bytes(MacroAssembler* masm, Register thread,
                                               Register var_size_in_bytes,
                                               int con_size_in_bytes,
                                               Register t1) {
  if (!thread->is_valid()) {
#ifdef _LP64
    thread = r15_thread;
#else
    assert(t1->is_valid(), "need temp reg");
    thread = t1;
    __ get_thread(thread);
#endif
  }

#ifdef _LP64
  if (var_size_in_bytes->is_valid()) {
    __ addq(Address(thread, in_bytes(JavaThread::allocated_bytes_offset())), var_size_in_bytes);
  } else {
    __ addq(Address(thread, in_bytes(JavaThread::allocated_bytes_offset())), con_size_in_bytes);
  }
#else
  if (var_size_in_bytes->is_valid()) {
    __ addl(Address(thread, in_bytes(JavaThread::allocated_bytes_offset())), var_size_in_bytes);
  } else {
    __ addl(Address(thread, in_bytes(JavaThread::allocated_bytes_offset())), con_size_in_bytes);
  }
  __ adcl(Address(thread, in_bytes(JavaThread::allocated_bytes_offset())+4), 0);
#endif
}

void BarrierSetAssembler::potential_load_from_remote(
  MacroAssembler* masm, DecoratorSet decorators, BasicType type,
  Register dst, Address src, Register tmp1, Register tmp_thread, Label& load_done) {
  // check for remote ref, local ref would just exit
  Label exit, is_remote, done;
  // push some regs to stack for temp usage
  __ push (c_rarg2);
  __ push (c_rarg3);
  
  // Check if the oop is in remote
  __ movptr(c_rarg2, src);
  __ movptr(c_rarg3, (intptr_t) Universe::verify_remote_oop_mask());
  __ andptr(c_rarg2, c_rarg3);
  __ movptr(c_rarg3, (intptr_t) Universe::verify_remote_oop_bits());
  __ cmpptr(c_rarg2, c_rarg3);
  __ jcc(Assembler::zero, is_remote);
  // jmp to done, procceed as regular load, nothing else to do here
  __ jmp(done);

  __ bind(is_remote);
  // what to do if oop is remote
  // call a lrb stub which will
  // check if src is in temp local rdma region, if yes then save it to dst
  __ call(RuntimeAddress(CAST_FROM_FN_PTR(address, BarrierSetAssembler::rdma_load_barrier())));


  __ bind(done);
  // restore registers
  __ pop (c_rarg3);
  __ pop (c_rarg2);
}

void BarrierSetAssembler::potential_store_to_remote(
  MacroAssembler* masm, DecoratorSet decorators, BasicType type,
  Address dst, Register val, Register tmp1, Register tmp2) {
  // check for remote ref, local ref would just exit


}

#undef __

#define __ cgen->assembler()->

address BarrierSetAssembler::generate_rdma_load_barrier(StubCodeGenerator* cgen) {
  __ align(CodeEntryAlignment);
  StubCodeMark mark(cgen, "StubRoutines", "shenandoah_lrb");
  address start = __ pc();

  Label slow_path;

  // We use RDI, which also serves as argument register for slow call.
  // RAX always holds the src object ptr, except after the slow call,
  // then it holds the result. R8/RBX is used as temporary register.

  Register tmp1 = rdi;
  Register tmp2 = LP64_ONLY(r8) NOT_LP64(rbx);

  __ push(tmp1);
  __ push(tmp2);

  // Check for object being in the collection set.
  __ mov(tmp1, rax);
  __ shrptr(tmp1, ShenandoahHeapRegion::region_size_bytes_shift_jint());
  __ movptr(tmp2, (intptr_t) ShenandoahHeap::in_cset_fast_test_addr());
  __ movbool(tmp2, Address(tmp2, tmp1, Address::times_1));
  __ testbool(tmp2);
  __ jccb(Assembler::notZero, slow_path);

  __ pop(tmp2);
  __ pop(tmp1);
  __ ret(0);

  __ bind(slow_path);

  __ push(rcx);
  __ push(rdx);
  __ push(rdi);
#ifdef _LP64
  __ push(r8);
  __ push(r9);
  __ push(r10);
  __ push(r11);
  __ push(r12);
  __ push(r13);
  __ push(r14);
  __ push(r15);
#endif
  __ push(rbp);
  __ movptr(rbp, rsp);
  __ andptr(rsp, -StackAlignmentInBytes);
  __ push_FPU_state();
  if (UseCompressedOops) {
    // __ call_VM_leaf(CAST_FROM_FN_PTR(address, ShenandoahRuntime::load_reference_barrier_narrow), rax, rsi);
    // call vm here
    // __ call_VM_leaf(CAST_FROM_FN_PTR(address, SharedRuntime::load_reference_barrier_narrow), rax, rsi);
  } else {
    // __ call_VM_leaf(CAST_FROM_FN_PTR(address, ShenandoahRuntime::load_reference_barrier), rax, rsi);
  }
  __ pop_FPU_state();
  __ movptr(rsp, rbp);
  __ pop(rbp);
#ifdef _LP64
  __ pop(r15);
  __ pop(r14);
  __ pop(r13);
  __ pop(r12);
  __ pop(r11);
  __ pop(r10);
  __ pop(r9);
  __ pop(r8);
#endif
  __ pop(rdi);
  __ pop(rdx);
  __ pop(rcx);

  __ pop(tmp2);
  __ pop(tmp1);
  __ ret(0);

  return start;
}

address BarrierSetAssembler::rdma_load_barrier() {
  assert(_rdma_load_barrier != NULL, "rdma load stub must be initialized");
  return _rdma_load_barrier;
}