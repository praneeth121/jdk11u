/*
 * Copyright (c) 2013, 2019, Red Hat, Inc. All rights reserved.
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

#ifndef SHARE_VM_GC_SHENANDOAH_SHENANDOAHHEAPREGION_HPP
#define SHARE_VM_GC_SHENANDOAH_SHENANDOAHHEAPREGION_HPP

#include "gc/shared/spaceDecorator.hpp"
#include "gc/shenandoah/shenandoahAllocRequest.hpp"
#include "gc/shenandoah/shenandoahAsserts.hpp"
#include "gc/shenandoah/shenandoahHeap.hpp"
#include "gc/shenandoah/shenandoahPacer.hpp"
#include "gc/shenandoah/shenandoahPadding.hpp"
#include "memory/universe.hpp"
#include "utilities/sizes.hpp"

class VMStructs;
class ShenandoahHeapRegionStateConstant;

class ShenandoahHeapRegion {
  friend class VMStructs;
  friend class ShenandoahHeapRegionStateConstant;
private:
  /*
    Region state is described by a state machine. Transitions are guarded by
    heap lock, which allows changing the state of several regions atomically.
    Region states can be logically aggregated in groups.

      "Empty":
      .................................................................
      .                                                               .
      .                                                               .
      .         Uncommitted  <-------  Committed <------------------------\
      .              |                     |                          .   |
      .              \---------v-----------/                          .   |
      .                        |                                      .   |
      .........................|.......................................   |
                               |                                          |
      "Active":                |                                          |
      .........................|.......................................   |
      .                        |                                      .   |
      .      /-----------------^-------------------\                  .   |
      .      |                                     |                  .   |
      .      v                                     v    "Humongous":  .   |
      .   Regular ---\-----\     ..................O................  .   |
      .     |  ^     |     |     .                 |               .  .   |
      .     |  |     |     |     .                 *---------\     .  .   |
      .     v  |     |     |     .                 v         v     .  .   |
      .    Pinned  Cset    |     .  HStart <--> H/Start   H/Cont   .  .   |
      .       ^    / |     |     .  Pinned         v         |     .  .   |
      .       |   /  |     |     .                 *<--------/     .  .   |
      .       |  v   |     |     .                 |               .  .   |
      .  CsetPinned  |     |     ..................O................  .   |
      .              |     |                       |                  .   |
      .              \-----\---v-------------------/                  .   |
      .                        |                                      .   |
      .........................|.......................................   |
                               |                                          |
      "Trash":                 |                                          |
      .........................|.......................................   |
      .                        |                                      .   |
      .                        v                                      .   |
      .                      Trash ---------------------------------------/
      .                                                               .
      .                                                               .
      .................................................................

    Transition from "Empty" to "Active" is first allocation. It can go from {Uncommitted, Committed}
    to {Regular, "Humongous"}. The allocation may happen in Regular regions too, but not in Humongous.

    Transition from "Active" to "Trash" is reclamation. It can go from CSet during the normal cycle,
    and from {Regular, "Humongous"} for immediate reclamation. The existence of Trash state allows
    quick reclamation without actual cleaning up.

    Transition from "Trash" to "Empty" is recycling. It cleans up the regions and corresponding metadata.
    Can be done asynchronously and in bulk.

    Note how internal transitions disallow logic bugs:
      a) No region can go Empty, unless properly reclaimed/recycled;
      b) No region can go Uncommitted, unless reclaimed/recycled first;
      c) Only Regular regions can go to CSet;
      d) Pinned cannot go Trash, thus it could never be reclaimed until unpinned;
      e) Pinned cannot go CSet, thus it never moves;
      f) Humongous cannot be used for regular allocations;
      g) Humongous cannot go CSet, thus it never moves;
      h) Humongous start can go pinned, and thus can be protected from moves (humongous continuations should
         follow associated humongous starts, not pinnable/movable by themselves);
      i) Empty cannot go Trash, avoiding useless work;
      j) ...
   */

  enum RegionState {
    _empty_uncommitted,       // region is empty and has memory uncommitted
    _empty_committed,         // region is empty and has memory committed
    _regular,                 // region is for regular allocations
    _humongous_start,         // region is the humongous start
    _humongous_cont,          // region is the humongous continuation
    _pinned_humongous_start,  // region is both humongous start and pinned
    _cset,                    // region is in collection set
    _pinned,                  // region is pinned
    _pinned_cset,             // region is pinned and in cset (evac failure path)
    _trash,                   // region contains only trash
    _temp,                    // Temporary region for rdma evac, either temp local or temp remote referencing each other
    _REGION_STATES_NUM        // last
  };

  static const char* region_state_to_string(RegionState s) {
    switch (s) {
      case _empty_uncommitted:       return "Empty Uncommitted";
      case _empty_committed:         return "Empty Committed";
      case _regular:                 return "Regular";
      case _humongous_start:         return "Humongous Start";
      case _humongous_cont:          return "Humongous Continuation";
      case _pinned_humongous_start:  return "Humongous Start, Pinned";
      case _cset:                    return "Collection Set";
      case _pinned:                  return "Pinned";
      case _pinned_cset:             return "Collection Set, Pinned";
      case _trash:                   return "Trash";
      case _temp:                    return "Temp";
      default:
        ShouldNotReachHere();
        return "";
    }
  }

  // This method protects from accidental changes in enum order:
  int region_state_to_ordinal(RegionState s) const {
    switch (s) {
      case _empty_uncommitted:      return 0;
      case _empty_committed:        return 1;
      case _regular:                return 2;
      case _humongous_start:        return 3;
      case _humongous_cont:         return 4;
      case _cset:                   return 5;
      case _pinned:                 return 6;
      case _trash:                  return 7;
      case _pinned_cset:            return 8;
      case _pinned_humongous_start: return 9;
      case _temp:                   return 10;
      default:
        ShouldNotReachHere();
        return -1;
    }
  }

  void report_illegal_transition(const char* method);

public:
  static const int region_states_num() {
    return _REGION_STATES_NUM;
  }

  // Allowed transitions from the outside code:
  void make_regular_allocation();
  void make_regular_bypass();
  void make_humongous_start();
  void make_humongous_cont();
  void make_humongous_start_bypass();
  void make_humongous_cont_bypass();
  void make_pinned();
  void make_unpinned();
  void make_cset();
  void make_trash();
  void make_trash_immediate();
  void make_empty();
  void make_uncommitted();
  void make_committed_bypass();
  void make_temp_local();
  void make_temp_remote();

  // Individual states:
  bool is_empty_uncommitted()      const { return _state == _empty_uncommitted; }
  bool is_empty_committed()        const { return _state == _empty_committed; }
  bool is_regular()                const { return _state == _regular; }
  bool is_humongous_continuation() const { return _state == _humongous_cont; }

  // Participation in logical groups:
  bool is_empty()                  const { return is_empty_committed() || is_empty_uncommitted(); }
  bool is_active()                 const { return !is_empty() && !is_trash(); }
  bool is_trash()                  const { return _state == _trash; }
  bool is_humongous_start()        const { return _state == _humongous_start || _state == _pinned_humongous_start; }
  bool is_humongous()              const { return is_humongous_start() || is_humongous_continuation(); }
  bool is_committed()              const { return !is_empty_uncommitted(); }
  bool is_cset()                   const { return _state == _cset   || _state == _pinned_cset; }
  bool is_pinned()                 const { return _state == _pinned || _state == _pinned_cset || _state == _pinned_humongous_start; }

  bool is_remote()                 const { return _is_remote; }
  bool is_local()                  const { return !_is_remote; }

  bool is_temp()                   const { return _state == _temp; }

  bool is_temp_local()             const { return is_temp() && is_local(); }
  bool is_temp_remote()            const { return is_temp() && is_remote(); }

  // Macro-properties:
  bool is_alloc_allowed()          const { return is_empty() || is_regular() || _state == _pinned; }
  bool is_stw_move_allowed()       const { return is_regular() || _state == _cset || (ShenandoahHumongousMoves && _state == _humongous_start); }

  RegionState state()              const { return _state; }
  int  state_ordinal()             const { return region_state_to_ordinal(_state); }

  void record_pin();
  void record_unpin();
  size_t pin_count() const;

  bool is_in(HeapWord* v) { return v >= bottom() && v < end(); }

private:
  static size_t RegionCount;
  static size_t RegionSizeBytes;
  static size_t RegionSizeWords;
  static size_t RegionSizeBytesShift;
  static size_t RegionSizeWordsShift;
  static size_t RegionSizeBytesMask;
  static size_t RegionSizeWordsMask;
  static size_t HumongousThresholdBytes;
  static size_t HumongousThresholdWords;
  static size_t MaxTLABSizeBytes;
  static size_t MaxTLABSizeWords;

  // Never updated fields
  size_t const _index;
  HeapWord* const _bottom;
  HeapWord* const _end;

  // Rarely updated fields
  HeapWord* _new_top;
  double _empty_time;

  // Seldom updated fields
  RegionState _state;
  bool _is_remote;

  // Frequently updated fields
  HeapWord* _top;

  size_t _tlab_allocs;
  size_t _gclab_allocs;

  volatile size_t _live_data;
  volatile size_t _critical_pins;

  HeapWord* volatile _update_watermark;

  // pointer to a temporary region, remote one point to local one and vice versa, must be null if this is not a temp region
  ShenandoahHeapRegion* _temp_forwarding;

  // mark where the first tlab is allocated for evacuation, always NULL for local region
  HeapWord* _evac_bottom;
  size_t _rdma_buffer_offset_in_words;

public:
  ShenandoahHeapRegion(HeapWord* start, size_t index, bool committed, bool is_remote = false);

  static const size_t MIN_NUM_REGIONS = 10;

  // Return adjusted max heap size
  static size_t setup_sizes(size_t max_heap_size);

  double empty_time() {
    return _empty_time;
  }

  inline static size_t required_regions(size_t bytes) {
    return (bytes + ShenandoahHeapRegion::region_size_bytes() - 1) >> ShenandoahHeapRegion::region_size_bytes_shift();
  }

  inline static size_t region_count() {
    return ShenandoahHeapRegion::RegionCount;
  }

  inline static size_t region_size_bytes() {
    return ShenandoahHeapRegion::RegionSizeBytes;
  }

  inline static size_t region_size_words() {
    return ShenandoahHeapRegion::RegionSizeWords;
  }

  inline static size_t region_size_bytes_shift() {
    return ShenandoahHeapRegion::RegionSizeBytesShift;
  }

  inline static size_t region_size_words_shift() {
    return ShenandoahHeapRegion::RegionSizeWordsShift;
  }

  inline static size_t region_size_bytes_mask() {
    return ShenandoahHeapRegion::RegionSizeBytesMask;
  }

  inline static size_t region_size_words_mask() {
    return ShenandoahHeapRegion::RegionSizeWordsMask;
  }

  // Convert to jint with sanity checking
  inline static jint region_size_bytes_jint() {
    assert (ShenandoahHeapRegion::RegionSizeBytes <= (size_t)max_jint, "sanity");
    return (jint)ShenandoahHeapRegion::RegionSizeBytes;
  }

  // Convert to jint with sanity checking
  inline static jint region_size_words_jint() {
    assert (ShenandoahHeapRegion::RegionSizeWords <= (size_t)max_jint, "sanity");
    return (jint)ShenandoahHeapRegion::RegionSizeWords;
  }

  // Convert to jint with sanity checking
  inline static jint region_size_bytes_shift_jint() {
    assert (ShenandoahHeapRegion::RegionSizeBytesShift <= (size_t)max_jint, "sanity");
    return (jint)ShenandoahHeapRegion::RegionSizeBytesShift;
  }

  // Convert to jint with sanity checking
  inline static jint region_size_words_shift_jint() {
    assert (ShenandoahHeapRegion::RegionSizeWordsShift <= (size_t)max_jint, "sanity");
    return (jint)ShenandoahHeapRegion::RegionSizeWordsShift;
  }

  inline static size_t humongous_threshold_bytes() {
    return ShenandoahHeapRegion::HumongousThresholdBytes;
  }

  inline static size_t humongous_threshold_words() {
    return ShenandoahHeapRegion::HumongousThresholdWords;
  }

  inline static size_t max_tlab_size_bytes() {
    return ShenandoahHeapRegion::MaxTLABSizeBytes;
  }

  inline static size_t max_tlab_size_words() {
    return ShenandoahHeapRegion::MaxTLABSizeWords;
  }

  inline size_t index() const {
    return _index;
  }

  // Allocation (return NULL if full)
  inline HeapWord* allocate(size_t word_size, ShenandoahAllocRequest::Type type);

  inline void clear_live_data();
  void set_live_data(size_t s);

  // Increase live data for newly allocated region
  inline void increase_live_data_alloc_words(size_t s);

  // Increase live data for region scanned with GC
  inline void increase_live_data_gc_words(size_t s);

  inline bool has_live() const;
  inline size_t get_live_data_bytes() const;
  inline size_t get_live_data_words() const;

  inline size_t garbage() const;

  void print_on(outputStream* st) const;

  void recycle();

  void oop_iterate(OopIterateClosure* cl);

  HeapWord* block_start(const void* p) const;
  size_t block_size(const HeapWord* p) const;
  bool block_is_obj(const HeapWord* p) const { return p < top(); }

  // Find humongous start region that this region belongs to
  ShenandoahHeapRegion* humongous_start_region() const;

  HeapWord* top() const         { return _top;     }
  void set_top(HeapWord* v)     { _top = v;        }

  HeapWord* new_top() const     { return _new_top; }
  void set_new_top(HeapWord* v) { _new_top = v;    }

  HeapWord* bottom() const      { return _bottom;  }
  HeapWord* end() const         { return _end;     }

  HeapWord* evac_bottom() const           { return _evac_bottom; }
  void set_evac_bottom(HeapWord* v)  { _evac_bottom = v; }

  size_t rdma_buffer_offset_in_words() const      {
    assert(_rdma_buffer_offset_in_words != region_size_words(), "Must not be at invalid value to do this, aka must have been set");
    return _rdma_buffer_offset_in_words;
  }
  
  void set_rdma_buffer_offset_in_words(size_t v)  {
    assert(_rdma_buffer_offset_in_words == region_size_words(), "Must be at invalid value to do this, aka have not been set before");
    assert(v < region_size_words(), "Must be region-bound");
    _rdma_buffer_offset_in_words = v;
  }

  size_t capacity() const       { return byte_size(bottom(), end()); }
  size_t used() const           { return byte_size(bottom(), top()); }
  size_t free() const           { return byte_size(top(),    end()); }

  inline void adjust_alloc_metadata(ShenandoahAllocRequest::Type type, size_t);
  void reset_alloc_metadata();
  size_t get_shared_allocs() const;
  size_t get_tlab_allocs() const;
  size_t get_gclab_allocs() const;

  inline HeapWord* get_update_watermark() const;
  inline void set_update_watermark(HeapWord* w);
  inline void set_update_watermark_at_safepoint(HeapWord* w);

  ShenandoahHeapRegion* get_temp_forwarding() const { return _temp_forwarding; }
  void set_temp_forwarding(ShenandoahHeapRegion* r) {
#ifdef ASSERT
    if (is_remote()) {
      assert(r->is_local(), "Must be the opposite");
    } else {
      assert(r->is_remote(), "Must be the opposite");
    }
#endif
    _temp_forwarding = r;
  }

  HeapWord* get_temp_forwarding(HeapWord* p) {
    assert(is_temp(), "Must be in temp state");
    assert(_temp_forwarding != NULL, "Must be forwarded");

    size_t offset_in_words = pointer_delta(bottom(), p);
    assert(offset_in_words < region_size_words(), "pointer must be contained in the region");

    return _temp_forwarding->bottom() + offset_in_words;
  }

private:
  void do_commit();
  void do_uncommit();

  void oop_iterate_objects(OopIterateClosure* cl);
  void oop_iterate_humongous(OopIterateClosure* cl);

  inline void internal_increase_live_data(size_t s);

  void set_state(RegionState to);
};

#endif // SHARE_VM_GC_SHENANDOAH_SHENANDOAHHEAPREGION_HPP
