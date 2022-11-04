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

#ifndef SHARE_OOPS_ARRAYOOP_INLINE_HPP
#define SHARE_OOPS_ARRAYOOP_INLINE_HPP

#include "oops/access.inline.hpp"
#include "oops/arrayOop.hpp"

void* arrayOopDesc::base(BasicType type) const {
  oop resolved_obj = Access<>::resolve(as_oop());
  if (is_remote_oop()) tty->print_cr("!!!!!!!!! arrayOopDesc::base");
  return arrayOop(resolved_obj)->base_raw(type);
}

void* arrayOopDesc::base_raw(BasicType type) const {
  if (is_remote_oop()) {
    RemoteMem* r_mem = Universe::heap()->remote_mem();
    assert(r_mem, "remote mem must be init");
    if (r_mem->is_in_evac_set((void*)this)) {
      void* buffering_addr = r_mem->get_corresponding_evac_buffer_address((void*)this);
      assert(buffering_addr, "must not be NULL");
      oop buffering_oop = oop(buffering_addr);
      tty->print_cr("array base_raw: Remote obj %p, buffering at %p, klass %p", (void*)this, buffering_addr, buffering_oop->klass_or_null_local());
      return reinterpret_cast<void*>(cast_from_oop<intptr_t>(buffering_oop) + base_offset_in_bytes(type));
    }
    assert(false, "We are not here yet");
    return NULL;
  }
  if (is_remote_oop()) tty->print_cr("!!!!!!!!! arrayOopDesc::base_raw");
  return reinterpret_cast<void*>(cast_from_oop<intptr_t>(as_oop()) + base_offset_in_bytes(type));
}

void arrayOopDesc::set_length(HeapWord* mem, int length) {
  if (is_remote_oop(mem)) {
    RemoteMem* r_mem = Universe::heap()->remote_mem();
    assert(r_mem, "remote mem must be init");
    if (r_mem->is_in_evac_set((void*)mem)) {
      void* buffering_addr = r_mem->get_corresponding_evac_buffer_address((void*)mem);
      assert(buffering_addr, "must not be NULL");
      oop buffering_oop = oop(buffering_addr);
      tty->print_cr("array set_length: Remote obj %p, buffering at %p, klass %p", (void*)mem, buffering_addr, buffering_oop->klass_or_null_local());
      *(int*)(((char*)buffering_addr) + length_offset_in_bytes()) = length;
      return;
    }
    assert(false, "We are not here yet");
    r_mem->write((char*)mem + length_offset_in_bytes(), (char*)&length, sizeof(int));
    return;
  }
  *(int*)(((char*)mem) + length_offset_in_bytes()) = length;
}

int arrayOopDesc::length() const {
  if (is_remote_oop()) {
    RemoteMem* r_mem = Universe::heap()->remote_mem();
    if (r_mem->is_in_evac_set((void*)this)) {
      void* buffering_addr = r_mem->get_corresponding_evac_buffer_address((void*)this);
      assert(buffering_addr, "remote addr must not be null");
      return *(int*)(((intptr_t)buffering_addr) + length_offset_in_bytes());
    }
    assert(false, "Should not reach here yet");
  }
  return *(int*)(((intptr_t)this) + length_offset_in_bytes());
}

#endif // SHARE_OOPS_ARRAYOOP_INLINE_HPP
