//#pragma once
//#include <vector>
//#include <cstdint>
//#include <memory>
//#include <string>
//#include <future>
//#include <webgpu/webgpu_cpp.h>
//#include "core/buffer.hpp"
//#include "core/stagingpool.hpp"
//#include "core/pipeline.hpp"
//#include "core/parameterset.hpp"
//
//namespace krnl {
//
//    enum class DType {
//        F32,
//        // add others later
//    };
//
//    struct Shape {
//        std::vector<size_t> dims;
//        size_t size() const {
//            size_t s = 1;
//            for (auto d : dims) s *= d;
//            return s;
//        }
//    };
//
//    class Tensor {
//    public:
//        // Create an empty tensor (uninitialized)
//        static Tensor Empty(wgpu::Device device, const Shape& shape, DType dtype, const char* label = nullptr);
//
//        // Create and upload from host vector<float>
//        static Tensor FromHost(wgpu::Device device, const std::vector<float>& data, const Shape& shape, PersistentStagingPool* pool = nullptr, const char* label = nullptr);
//
//        // Create zeros
//        static Tensor Zeros(wgpu::Device device, const Shape& shape, PersistentStagingPool* pool = nullptr, const char* label = nullptr);
//
//        // Accessors
//        const Shape& shape() const { return m_shape; }
//        size_t elementCount() const { return m_shape.size(); }
//        size_t byteSize() const { return m_sizeBytes; }
//        DType dtype() const { return m_dtype; }
//        const krnl::Buffer& buffer() const { return m_buffer; }
//
//        // Write data from host (synchronous): uses staging pool if provided else Buffer::writeViaStaging
//        void write(const void* src, size_t bytes, wgpu::Queue queue, PersistentStagingPool* pool = nullptr);
//
//        // Async write (returns immediately)
//        void writeAsync(const void* src, size_t bytes, wgpu::Queue queue, PersistentStagingPool* pool = nullptr);
//
//        // Blocking host read (returns vector<float>)
//        std::vector<float> toHost(wgpu::Device device, wgpu::Queue queue) const;
//
//        // convenience dtype helpers
//        static size_t dtypeSize(DType dt) {
//            switch (dt) {
//            case DType::F32: return sizeof(float);
//            default: return sizeof(float);
//            }
//        }
//
//    private:
//        Tensor() = default;
//        Tensor(wgpu::Device device, const Shape& shape, DType dtype, const krnl::Buffer& buffer);
//
//    private:
//        Shape m_shape;
//        DType m_dtype;
//        size_t m_sizeBytes = 0;
//        krnl::Buffer m_buffer;
//        wgpu::Device m_device;
//    };
//
//    /////////////////////////
//    // TensorOps
//    /////////////////////////
//    class TensorOps {
//    public:
//        // elementwise add: C = A + B  (all same shape)
//        // returns new Tensor with result
//        static Tensor Add(const Tensor& A, const Tensor& B, wgpu::Queue queue);
//
//        // matmul: C = A * B
//        // A: MxK, B: KxN -> C: MxN
//        static Tensor MatMul(const Tensor& A, const Tensor& B, wgpu::Queue queue);
//
//    private:
//        // internal helpers to get pipelines (cached per device could be added)
//        static Pipeline makeAddPipeline(wgpu::Device device, ParameterSet& params);
//        static Pipeline makeMatMulPipeline(wgpu::Device device, ParameterSet& params, size_t M, size_t N, size_t K);
//    };
//
//} // namespace krnl
