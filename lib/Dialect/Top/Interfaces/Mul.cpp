//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// TPU-MLIR is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "tpu_mlir/Support/Dnnl/Dnnl.h"



int64_t top::MulOp::getFLOPs() {
  return module::getNumElements(getOutput()) *
         (getInputs().size() - 1 + (getDoRelu() ? 1 : 0));
}

LogicalResult top::MulOp::init(InferenceParameter &p) {
  auto binary = new Binary();
  auto lhs_shape =  module::getShape(getInputs()[0]);
  auto rhs_shape = module::getShape(getInputs()[1]);

  (*binary)
      .hs(p.inputs[0], p.inputs[1], lhs_shape, rhs_shape)
      .dst(p.outputs[0], module::getShape(getOutput()))
      .do_relu(getDoRelu())
      .relu_limit(getReluLimit().convertToDouble())
      .algorithem(algorithm::binary_mul)
      .setup();

  p.handle = (void *)binary;

  return success();
}
void top::MulOp::deinit(InferenceParameter &p) {
  if (p.handle != nullptr) {
    auto binary = (Binary *)p.handle;
    delete binary;
    p.handle = nullptr;
  }
}

LogicalResult top::MulOp::inference(InferenceParameter &p) {
  if (p.handle == nullptr) {
    return failure();
  }
  auto binary = (Binary *)p.handle;
  binary->run();
  return success();
}

void top::MulOp::shape_inference() {
  broadcast_shape_inference(getOperation());
  for (int i = 0; i < getNumOperands(); i++) {
    auto value = getInputs()[i];
    broadcast_tensor_reshape(getOutput(), value);
  }
  auto inputs = getInputs();
  // shape value inference can only support shape and weight
  bool need_shape_val_infer =
      std::all_of(inputs.begin(), inputs.end(), [](auto in_op) {
        return module::isWeight(in_op) || module::isShape(in_op);
      }) &&
      std::any_of(inputs.begin(), inputs.end(), [](auto in_op) {
        return module::isShape(in_op);
      });
  if (need_shape_val_infer) {
    std::vector<std::vector<int64_t>> input_shapes_v;
    for (auto in_op : inputs) {
      if (module::isShape(in_op)) {
        auto input_shape_v = module::getShapeTensorValue(in_op);
        input_shapes_v.push_back(input_shape_v);
      } else if (module::isWeight(in_op)) {
        auto data = in_op.getDefiningOp<top::WeightOp>().read_as_float();
        std::vector<int64_t> data_v(data->begin(), data->end());
        input_shapes_v.push_back(data_v);
      }
    }
    auto out_shape = module::getShape(getOutput());
    auto output_shape_v =
        module::commonShapeValInfer(getOperation(), input_shapes_v, out_shape);
    module::bindShapeTensorValue(getOutput(), output_shape_v);
  }
}
