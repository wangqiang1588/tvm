/*!
 *  Copyright (c) 2016 by Contributors
 * \file ir_util.h
 * \brief Helper functions to construct and compose IR nodes.
 */
#ifndef TVM_PASS_IR_UTIL_H_
#define TVM_PASS_IR_UTIL_H_

#include <tvm/ir.h>
#include <vector>

namespace tvm {
namespace ir {
/*!
 * \brief combine the nest stmt, whose body is not defined.
 * \param nest A list of For and LetStmt, whose body is not defined.
 * \param body body
 * \return The combined Stmt
 */
Stmt MergeNest(const std::vector<Stmt>& nest, Stmt body);

/*!
 * \brief combine the nest stmt, whose body is not defined.
 * \param nest A list of For and LetStmt, whose body is not defined.
 * \param body body
 * \return The combined Stmt
 */
Stmt MergeNest(const std::vector<std::vector<Stmt> >& nest, Stmt body);

/*!
 * \brief combine sequence of operations.
 * \param seq The sequence.
 * \return The combined Stmt
 */
Stmt MergeSeq(const std::vector<Stmt>& seq);

/*!
 * \brief update array with an unary function
 * \param arr array
 * \param fupdate an unary function
 * \tparam T type of array element
 * \tparam F type of the unary function
 * \return if update happens, return the new array, else return the
 *  original array
 */
template<typename T, typename F>
inline Array<T> UpdateArray(Array<T> arr, F fupdate) {
  std::vector<T> new_arr(arr.size());
  bool changed = false;
  for (size_t i = 0; i < arr.size(); ++i) {
    T old_elem = arr[i];
    T new_elem = fupdate(old_elem);
    if (!new_elem.same_as(old_elem)) changed = true;
    new_arr[i] = new_elem;
  }
  if (!changed) {
    return arr;
  } else {
    return Array<T>(new_arr);
  }
}

/*!
 * \brief Get construct from struct
 * \param dtype The data type.
 * \param handle the struct handle.
 * \param index the offset index.
 * \param kind The data kind.
 * \return the get expression.
 */
inline Expr TVMStructGet(
    Type dtype, Var handle, int index,
    intrinsic::TVMStructFieldKind kind) {
  Array<Expr> args ={
    handle,
    make_const(Int(32), index),
    make_const(Int(32), kind)};
  return Call::make(dtype, intrinsic::tvm_struct_get, args, Call::PureIntrinsic);
}

/*!
 * \brief Address of handle + offset
 * \param handle the array handle.
 * \param dtype The data type.
 * \param offset the offset index.
 */
inline Expr AddressOffset(Var handle, Type dtype, int offset) {
  return Call::make(
      Handle(), intrinsic::tvm_address_of,
      {Load::make(dtype, handle, make_const(Int(32), offset * dtype.lanes()),
                  const_true(dtype.lanes()))},
      Call::PureIntrinsic);
}

/*!
 * \brief Set value into struct.
 * \param handle the struct handle.
 * \param index the offset index.
 * \param kind The data kind.
 * \param value The value to be set.
 * \return the set stmt.
 */
inline Stmt TVMStructSet(
    Var handle, int index,
    intrinsic::TVMStructFieldKind kind, Expr value) {
  Array<Expr> args ={
    handle,
    make_const(Int(32), index),
    make_const(Int(32), kind),
    value};
  return Evaluate::make(
      Call::make(Int(32), intrinsic::tvm_struct_set, args, Call::Intrinsic));
}

/*!
 * \brief Get the type that is passed around TVM PackedFunc API.
 * \param t The original type.
 * \return The corresponding API type.
 */
inline Type APIType(Type t) {
  if (t.is_handle()) return t;
  CHECK_EQ(t.lanes(), 1)
      << "Cannot pass vector type through packed API.";
  if (t.is_uint() || t.is_int()) return Int(64);
  CHECK(t.is_float());
  return Float(64);
}
}  // namespace ir
}  // namespace tvm
#endif  // TVM_PASS_IR_UTIL_H_
