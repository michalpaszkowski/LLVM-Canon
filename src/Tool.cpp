#include "IRCanonicalizer.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/FileSystem.h"

using namespace llvm;

/// Reads a module from a file.
/// On error, messages are written to stderr and null is returned.
///
/// \param Context LLVM Context for the module.
/// \param Name Input file name.
static std::unique_ptr<Module> readModule(LLVMContext &Context,
                                          StringRef Name) {
  SMDiagnostic Diag;
  std::unique_ptr<Module> Module = parseIRFile(Name, Diag, Context);

  if (!Module)
    Diag.print("llvm-canon", errs());

  return Module;
}

/// Input LLVM module file name.
cl::opt<std::string> InputFilename("f", cl::desc("Specify input filename"),
                                   cl::value_desc("filename"), cl::Required);
/// Output LLVM module file name.
cl::opt<std::string> OutputFilename("o", cl::desc("Specify output filename"),
                                    cl::value_desc("filename"), cl::Required);

/// \name Canonicalizer flags.
/// @{
/// Preserves original order of instructions.
cl::opt<bool> PreserveOrder("preserve-order",
    cl::desc("Preserves original instruction order"));
/// Renames all instructions (including user-named).
cl::opt<bool>RenameAll("rename-all",
    cl::desc("Renames all instructions (including user-named)"));
/// Folds all regular instructions (including pre-outputs).
cl::opt<bool> FoldPreoutputs("fold-all",
    cl::desc("Folds all regular instructions (including pre-outputs)"));
/// Sorts and reorders operands in commutative instructions.
cl::opt<bool> ReorderOperands("reorder-operands",
    cl::desc("Sorts and reorders operands in commutative instructions"));
/// @}

int main(int argc, char **argv) {
  cl::ParseCommandLineOptions(
      argc, argv,
      " LLVM-Canon\n\n"
      " This tool aims to transform LLVM Modules into canonical form by"
      " reordering and renaming instructions while preserving the same"
      " semantics. Making it easier to spot semantic differences while"
      " diffing two modules which have undergone different passes.\n");

  LLVMContext Context;

  std::unique_ptr<Module> Module = readModule(Context, InputFilename);

  if (!Module)
    return 1;

  IRCanonicalizer Canonicalizer(PreserveOrder, RenameAll, FoldPreoutputs,
                                ReorderOperands);

  for (auto &Function : *Module) {
    Canonicalizer.runOnFunction(Function);
  }

  if (verifyModule(*Module, &errs()))
    return 1;

  std::error_code EC;
  raw_fd_ostream OutputStream(OutputFilename, EC, sys::fs::OF_None);

  if (EC) {
    errs() << EC.message();
    return 1;
  }

  Module->print(OutputStream, nullptr, false);
  OutputStream.close();
  return 0;
}