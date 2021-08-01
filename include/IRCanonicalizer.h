#pragma once

#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

using namespace llvm;

/// IRCanonicalizer aims to transform LLVM IR into canonical form.
class IRCanonicalizer : public FunctionPass {
public:
  static char ID;

  /// Constructor for the IRCanonicalizer.
  ///
  /// \param PreserveOrder Preserves original order of instructions.
  /// \param RenameAll Renames all instructions (including user-named).
  /// \param FoldPreoutputs Folds all regular instructions (including pre-outputs).
  /// \param ReorderOperands Sorts and reorders operands in commutative instructions.
  IRCanonicalizer(bool PreserveOrder, bool RenameAll, bool FoldPreoutputs,
                  bool ReorderOperands)
      : FunctionPass(ID), PreserveOrder(PreserveOrder), RenameAll(RenameAll),
        FoldPreoutputs(FoldPreoutputs), ReorderOperands(ReorderOperands) {}

  bool runOnFunction(Function &F) override;

private:
  // Random constant for hashing, so the state isn't zero.
  const uint64_t MagicHashConstant = 0x6acaa36bef8325c5ULL;

  /// \name Canonicalizer flags.
  /// @{
  /// Preserves original order of instructions.
  bool PreserveOrder;
  /// Renames all instructions (including user-named).
  bool RenameAll;
  /// Folds all regular instructions (including pre-outputs).
  bool FoldPreoutputs;
  /// Sorts and reorders operands in commutative instructions.
  bool ReorderOperands;
  /// @}

  /// \name Naming.
  /// @{
  void nameFunctionArguments(Function &F);
  void nameBasicBlocks(Function &F);
  void nameInstructions(SmallVector<Instruction *, 16> &Outputs);
  void nameInstruction(Instruction *I,
                       SmallPtrSet<const Instruction *, 32> &Visited);
  void nameAsInitialInstruction(Instruction *I);
  void nameAsRegularInstruction(Instruction *I,
                                SmallPtrSet<const Instruction *, 32> &Visited);
  void foldInstructionName(Instruction *I);
  /// @}

  /// \name Reordering.
  /// @{
  void reorderInstructions(SmallVector<Instruction *, 16> &Outputs);
  void reorderInstruction(Instruction *Used, Instruction *User,
                          SmallPtrSet<const Instruction *, 32> &Visited);
  void reorderInstructionOperandsByNames(Instruction *I);
  void reorderPHIIncomingValues(PHINode *PN);
  /// @}

  /// \name Utility methods.
  /// @{
  SmallVector<Instruction *, 16> collectOutputInstructions(Function &F);
  bool isOutput(const Instruction *I);
  bool isInitialInstruction(const Instruction *I);
  bool hasOnlyImmediateOperands(const Instruction *I);
  SetVector<int>
  getOutputFootprint(Instruction *I,
                     SmallPtrSet<const Instruction *, 32> &Visited);
  /// @}
};