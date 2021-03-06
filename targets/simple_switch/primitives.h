/* Copyright 2013-present Barefoot Networks, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Antonin Bas (antonin@barefootnetworks.com)
 *
 */

#ifndef _BM_SIMPLE_SWITCH_PRIMITIVES_H_
#define _BM_SIMPLE_SWITCH_PRIMITIVES_H_

class modify_field : public ActionPrimitive<Field &, const Data &> {
  void operator ()(Field &f, const Data &d) {
    f.set(d);
  }
};

REGISTER_PRIMITIVE(modify_field);

class add_to_field : public ActionPrimitive<Field &, const Data &> {
  void operator ()(Field &f, const Data &d) {
    f.add(f, d);
  }
};

REGISTER_PRIMITIVE(add_to_field);

class subtract_from_field : public ActionPrimitive<Field &, const Data &> {
  void operator ()(Field &f, const Data &d) {
    f.sub(f, d);
  }
};

REGISTER_PRIMITIVE(subtract_from_field);

class add : public ActionPrimitive<Field &, const Data &, const Data &> {
  void operator ()(Field &f, const Data &d1, const Data &d2) {
    f.add(d1, d2);
  }
};

REGISTER_PRIMITIVE(add);

class subtract : public ActionPrimitive<Field &, const Data &, const Data &> {
  void operator ()(Field &f, const Data &d1, const Data &d2) {
    f.sub(d1, d2);
  }
};

REGISTER_PRIMITIVE(subtract);

class bit_xor : public ActionPrimitive<Field &, const Data &, const Data &> {
  void operator ()(Field &f, const Data &d1, const Data &d2) {
    f.bit_xor(d1, d2);
  }
};

REGISTER_PRIMITIVE(bit_xor);

class bit_or : public ActionPrimitive<Field &, const Data &, const Data &> {
  void operator ()(Field &f, const Data &d1, const Data &d2) {
    f.bit_or(d1, d2);
  }
};

REGISTER_PRIMITIVE(bit_or);

class bit_and : public ActionPrimitive<Field &, const Data &, const Data &> {
  void operator ()(Field &f, const Data &d1, const Data &d2) {
    f.bit_and(d1, d2);
  }
};

REGISTER_PRIMITIVE(bit_and);

class shift_left : public ActionPrimitive<Field &, const Data &, const Data &> {
  void operator ()(Field &f, const Data &d1, const Data &d2) {
    f.shift_left(d1, d2);
  }
};

REGISTER_PRIMITIVE(shift_left);

class shift_right : public ActionPrimitive<Field &, const Data &, const Data &> {
  void operator ()(Field &f, const Data &d1, const Data &d2) {
    f.shift_right(d1, d2);
  }
};

REGISTER_PRIMITIVE(shift_right);

class drop : public ActionPrimitive<> {
  void operator ()() {
    get_field("standard_metadata.egress_spec").set(511);
    if(get_phv().has_header("intrinsic_metadata")) {
      get_field("intrinsic_metadata.mcast_grp").set(0);
    }
  }
};

REGISTER_PRIMITIVE(drop);

class generate_digest : public ActionPrimitive<const Data &, const Data &> {
  void operator ()(const Data &receiver, const Data &learn_id) {
    // discared receiver for now
    get_field("intrinsic_metadata.lf_field_list").set(learn_id);
  }
};

REGISTER_PRIMITIVE(generate_digest);

class add_header : public ActionPrimitive<Header &> {
  void operator ()(Header &hdr) {
    // TODO: reset header to 0?
    if(!hdr.is_valid()) {
      hdr.reset();
      hdr.mark_valid();
    }
  }
};

REGISTER_PRIMITIVE(add_header);

class add_header_fast : public ActionPrimitive<Header &> {
  void operator ()(Header &hdr) {
    hdr.mark_valid();
  }
};

REGISTER_PRIMITIVE(add_header_fast);

class remove_header : public ActionPrimitive<Header &> {
  void operator ()(Header &hdr) {
    hdr.mark_invalid();
  }
};

REGISTER_PRIMITIVE(remove_header);

class copy_header : public ActionPrimitive<Header &, const Header &> {
  void operator ()(Header &dst, const Header &src) {
    if(!src.is_valid()) return;
    dst.mark_valid();
    assert(dst.get_header_type_id() == src.get_header_type_id());
    for(unsigned int i = 0; i < dst.size(); i++) {
      dst[i].set(src[i]);
    }
  }
};

REGISTER_PRIMITIVE(copy_header);

/* standard_metadata.clone_spec will contain the mirror id (16 LSB) and the
   field list id to copy (16 MSB) */
class clone_ingress_pkt_to_egress : public ActionPrimitive<const Data &, const Data &> {
  void operator ()(const Data &clone_spec, const Data &field_list_id) {
    Field &f_clone_spec = get_field("standard_metadata.clone_spec");
    f_clone_spec.shift_left(field_list_id, 16);
    f_clone_spec.add(f_clone_spec, clone_spec);
  }
};

REGISTER_PRIMITIVE(clone_ingress_pkt_to_egress);

class clone_egress_pkt_to_egress : public ActionPrimitive<const Data &, const Data &> {
  void operator ()(const Data &clone_spec, const Data &field_list_id) {
    Field &f_clone_spec = get_field("standard_metadata.clone_spec");
    f_clone_spec.shift_left(field_list_id, 16);
    f_clone_spec.add(f_clone_spec, clone_spec);
  }
};

REGISTER_PRIMITIVE(clone_egress_pkt_to_egress);

class modify_field_with_hash_based_offset
  : public ActionPrimitive<Field &, const Data &,
			   const NamedCalculation &, const Data &> {
  void operator ()(Field &dst, const Data &base,
		   const NamedCalculation &hash, const Data &size) {
    uint64_t v =
      (hash.output(get_packet()) % size.get<uint64_t>()) + base.get<uint64_t>();
    dst.set(v);
  }
};

REGISTER_PRIMITIVE(modify_field_with_hash_based_offset);

class no_op : public ActionPrimitive<> {
  void operator ()() {

  }
};

REGISTER_PRIMITIVE(no_op);

class execute_meter : public ActionPrimitive<MeterArray &, const Data &, Field &> {
  void operator ()(MeterArray &meter_array, const Data &idx, Field &dst) {
    dst.set(meter_array.execute_meter(get_packet(), idx.get_uint()));
  }
};

REGISTER_PRIMITIVE(execute_meter);

class count : public ActionPrimitive<CounterArray &, const Data &> {
  void operator ()(CounterArray &counter_array, const Data &idx) {
    counter_array.get_counter(idx.get_uint()).increment_counter(get_packet());
  }
};

REGISTER_PRIMITIVE(count);

class register_read : public ActionPrimitive<Field &, const RegisterArray &, const Data &> {
  void operator ()(Field &dst, const RegisterArray &src, const Data &idx) {
    dst.set(src[idx.get_uint()]);
  }
};

REGISTER_PRIMITIVE(register_read);

class register_write : public ActionPrimitive<RegisterArray &, const Data &, const Data &> {
  void operator ()(RegisterArray &dst, const Data &idx, const Data &src) {
    dst[idx.get_uint()].set(src);
  }
};

REGISTER_PRIMITIVE(register_write);

class push : public ActionPrimitive<HeaderStack &, const Data &> {
  void operator ()(HeaderStack &stack, const Data &num) {
    stack.push_front(num.get_uint());
  }
};

REGISTER_PRIMITIVE(push);

class pop : public ActionPrimitive<HeaderStack &, const Data &> {
  void operator ()(HeaderStack &stack, const Data &num) {
    stack.pop_front(num.get_uint());
  }
};

REGISTER_PRIMITIVE(pop);

#endif
