add_mlir_dialect_library(TPUMLIRCodegenPass
  Passes.cpp
  DecomposeLinalgGeneric.cpp
  TileAndFuse.cpp
  StructuredTransform.cpp
  InstrctionSelction.cpp
  Benefit.cpp

  DEPENDS
  TPUMLIRSG2260Gen
  TPUMLIRSG2260Dialect
  TPUMLIRCodegenPassIncGen
  LINK_LIBS
  PUBLIC
  MLIRIR
  MLIRAffineDialect
  MLIRArithDialect
  MLIRFuncDialect
  MLIRLinalgDialect
  MLIRLinalgTransforms
  MLIRMemRefDialect
  MLIRSCFDialect
  MLIRSCFTransforms
  MLIRSCFUtils
  MLIRTensorDialect
  MLIRTensorTilingInterfaceImpl
)
