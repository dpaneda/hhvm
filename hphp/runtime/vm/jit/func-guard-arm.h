/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_JIT_FUNC_GUARD_ARM_H
#define incl_HPHP_JIT_FUNC_GUARD_ARM_H

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"

#include "hphp/vixl/a64/instructions-a64.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

struct Func;

///////////////////////////////////////////////////////////////////////////////

namespace jit { namespace arm {

///////////////////////////////////////////////////////////////////////////////

namespace detail {

inline const Func** funcPrologueToGuardImmPtr(TCA prologue) {
  return reinterpret_cast<const Func**>(prologue) - 1;
}

}

///////////////////////////////////////////////////////////////////////////////

inline bool funcPrologueHasGuard(TCA prologue, const Func* func) {
  return *detail::funcPrologueToGuardImmPtr(prologue) == func;
}

inline TCA funcPrologueToGuard(TCA prologue, const Func* func) {
  if (!prologue || prologue == mcg->tx().uniqueStubs.fcallHelperThunk) {
    return prologue;
  }

  using namespace vixl;
  // Skip backwards over:
  // - The guarded Func* (8 bytes)
  // - The address of the redispatch stub (8 bytes)
  // - OPTIONALLY a Nop (4 bytes)
  auto beforeImm = Instruction::Cast(prologue - 20);
  if (beforeImm->Mask(~ImmHint_mask) == HINT &&
      beforeImm->Mask(ImmHint_mask) == NOP) {
    // There was a nop before the immediate. Now skip backwards over 6
    // instructions. See emitFuncGuard().
    return prologue - 20 - 6 * kInstructionSize;
  } else {
    // No nop. Only need to skip over 5 instructions.
    return prologue - 20 - 5 * kInstructionSize;
  }
}

inline void funcPrologueSmashGuard(TCA prologue, const Func* func) {
  *detail::funcPrologueToGuardImmPtr(prologue) = nullptr;
}

///////////////////////////////////////////////////////////////////////////////

void emitFuncGuard(const Func* func, CodeBlock& cb);

///////////////////////////////////////////////////////////////////////////////

}}}

#endif
