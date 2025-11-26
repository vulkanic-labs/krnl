//#include "tensor/tensor.hpp"
//#include "core/shader.hpp"
//#include "core/log.h"
//#include <cstring>
//#include <cassert>
//
//namespace krnl {
//
//    /* -----------------------
//       Tensor Implementation
//       ----------------------- */
//
//    Tensor::Tensor(wgpu::Device device, const Shape& shape, DType dtype, const krnl::Buffer& buffer)
//        : m_shape(shape), m_dtype(dtype), m_buffer(buffer), m_device(device)
//    {
//        m_sizeBytes = m_shape.size() * dtypeSize(m_dtype);
//    }
//
//    Tensor Tensor::Empty(wgpu::Device device, const Shape& shape, DType dtype, const char* label) {
//        size_t bytes = shape.size() * dtypeSize(dtype);
//        krnl::Buffer buf = krnl::Buffer::CreateStorage(device, bytes, label);
//        return Tensor(device, shape, dtype, buf);
//    }
//
//    Tensor Tensor::FromHost(wgpu::Device device, const std::vector<float>& data, const Shape& shape, PersistentStagingPool* pool, const char* label) {
//        size_t expected = shape.size();
//        assert(data.size() == expected && "FromHost size mismatch shape");
//        size_t bytes = expected * sizeof(float);
//        krnl::Buffer buf = krnl::Buffer::CreateStorage(device, bytes, label);
//        Tensor t(device, shape, DType::F32, buf);
//        // upload
//        if (pool) {
//            // use pool
//            auto staging = pool->allocate(bytes);
//            std::memcpy(staging->mappedPtr, data.data(), bytes);
//            pool->submitUpload(staging, buf.get(), bytes, 0, device.GetQueue());
//        }
//        else {
//            // fallback to writeViaStaging
//            t.m_buffer.writeViaStaging(data.data(), bytes, device.GetQueue());
//        }
//        return t;
//    }
//
//    Tensor Tensor::Zeros(wgpu::Device device, const Shape& shape, PersistentStagingPool* pool, const char* label) {
//        size_t elements = shape.size();
//        std::vector<float> zeros(elements);
//        return FromHost(device, zeros, shape, pool, label);
//    }
//
//    void Tensor::write(const void* src, size_t bytes, wgpu::Queue queue, PersistentStagingPool* pool) {
//        assert(bytes <= m_sizeBytes);
//        if (pool) {
//            auto staging = pool->allocate(bytes);
//            std::memcpy(staging->mappedPtr, src, bytes);
//            pool->submitUpload(staging, m_buffer.get(), bytes, 0, queue);
//        }
//        else {
//            m_buffer.writeViaStaging(src, bytes, queue);
//        }
//    }
//
//    void Tensor::writeAsync(const void* src, size_t bytes, wgpu::Queue queue, PersistentStagingPool* pool) {
//        // Simple wrapper identical to write — async semantics are handled by staging and queue.
//        write(src, bytes, queue, pool);
//    }
//
//    std::vector<float> Tensor::toHost(wgpu::Device device, wgpu::Queue queue) const {
//        std::promise<std::vector<float>> p;
//        auto f = p.get_future();
//        // create callback that sets promise
//        m_buffer.readAsync(device, queue, [&p](const void* data, size_t size) {
//            const float* fptr = reinterpret_cast<const float*>(data);
//            size_t count = size / sizeof(float);
//            std::vector<float> out(count);
//            std::memcpy(out.data(), fptr, size);
//            p.set_value(std::move(out));
//            });
//        // wait (blocks current thread until GPU finished and callback invoked)
//        auto result = f.get();
//        return result;
//    }
//
//    /* -----------------------
//       TensorOps
//       ----------------------- */
//
//    Pipeline TensorOps::makeAddPipeline(wgpu::Device device, ParameterSet& params) {
//        // WGSL for elementwise add (simple)
//        const std::string wgsl = R"(
//        @group(0) @binding(0) var<storage, read> A : array<f32>;
//        @group(0) @binding(1) var<storage, read> B : array<f32>;
//        @group(0) @binding(2) var<storage, read_write> Out : array<f32>;
//
//        @compute @workgroup_size(64)
//        fn main(@builtin(global_invocation_id) gid : vec3<u32>) {
//            let i = gid.x;
//            Out[i] = A[i] + B[i];
//        }
//    )";
//        return Pipeline::CreateComputeFromWGSL(device, wgsl, params, "main", "add_pipeline");
//    }
//
//    Tensor TensorOps::Add(const Tensor& A, const Tensor& B, wgpu::Queue queue) {
//        assert(A.dtype() == B.dtype());
//        assert(A.shape().dims == B.shape().dims);
//
//        wgpu::Device device = A.buffer().get()->GetDevice(); // there's no GetDevice on wgpu::Buffer - we assume device was stored in Tensor
//        // But we have m_device in Tensor — use work-around: we need device; so require device param or expose m_device accessor.
//        // For simplicity use A.m_device (friend?), but here we'll assume m_device available via A (we'll access private by making friend or adding accessor).
//        // For this example, assume Tensor has a getDevice() method. We'll add that in real code. For now, let's proceed:
//        // Build params (A,B,out)
//        Shape s = A.shape();
//        Tensor Out = Tensor::Empty(A.m_device, s, A.dtype(), "add_out");
//
//        std::vector<ParameterSet::Entry> entries = {
//            { const_cast<krnl::Buffer*>(&A.m_buffer), wgpu::ShaderStage::Compute },
//            { const_cast<krnl::Buffer*>(&B.m_buffer), wgpu::ShaderStage::Compute },
//            { const_cast<krnl::Buffer*>(&Out.m_buffer), wgpu::ShaderStage::Compute }
//        };
//        ParameterSet params(A.m_device, entries);
//
//        // Create pipeline
//        Pipeline p = makeAddPipeline(A.m_device, params);
//
//        // Record commands
//        wgpu::CommandEncoder encoder = A.m_device.CreateCommandEncoder();
//        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
//
//        // use helper
//        uint32_t total = static_cast<uint32_t>(A.elementCount());
//        uint32_t wgSize = 64;
//        uint32_t groups = (total + wgSize - 1) / wgSize;
//
//        p.encodeDispatch(pass, groups, 1, 1);
//        pass.End();
//
//        wgpu::CommandBuffer cmd = encoder.Finish();
//        queue.Submit(1, &cmd);
//
//        return Out;
//    }
//
//    Pipeline TensorOps::makeMatMulPipeline(wgpu::Device device, ParameterSet& params, size_t M, size_t N, size_t K) {
//        // Simple matmul WGSL: naive but tiled. This shader expects:
//        // A (M x K) row-major, B (K x N) row-major, Out (M x N) row-major
//        const std::string wgsl = R"(
//        struct Meta { M: u32, N: u32, K: u32 };
//        @group(0) @binding(3) var<uniform> meta : Meta;
//
//        @group(0) @binding(0) var<storage, read> A : array<f32>;
//        @group(0) @binding(1) var<storage, read> B : array<f32>;
//        @group(0) @binding(2) var<storage, read_write> Out : array<f32>;
//
//        @compute @workgroup_size(8,8)
//        fn main(@builtin(global_invocation_id) gid : vec3<u32>) {
//            let row = gid.y;
//            let col = gid.x;
//            if (row >= meta.M || col >= meta.N) {
//                return;
//            }
//            var sum : f32 = 0.0;
//            let M = meta.M;
//            let N = meta.N;
//            let K = meta.K;
//            // compute dot product
//            let rowBase = row * K;
//            for (var k : u32 = 0u; k < K; k = k + 1u) {
//                let a = A[rowBase + k];
//                let b = B[k * N + col];
//                sum = sum + a * b;
//            }
//            Out[row * N + col] = sum;
//        }
//    )";
//        return Pipeline::CreateComputeFromWGSL(device, wgsl, params, "main", "matmul_pipeline");
//    }
//
//    Tensor TensorOps::MatMul(const Tensor& A, const Tensor& B, wgpu::Queue queue) {
//        // Validate shapes
//        assert(A.dtype() == B.dtype());
//        size_t M = A.shape().dims[0];
//        size_t K = A.shape().dims[1];
//        assert(B.shape().dims[0] == K);
//        size_t N = B.shape().dims[1];
//
//        // Output
//        Shape outShape; outShape.dims = { M, N };
//        Tensor Out = Tensor::Empty(A.m_device, outShape, A.dtype(), "matmul_out");
//
//        // Create a small uniform buffer for meta
//        struct Meta { uint32_t M, N, K; } meta = { static_cast<uint32_t>(M), static_cast<uint32_t>(N), static_cast<uint32_t>(K) };
//        krnl::Buffer metaBuf = krnl::Buffer::CreateUniform(A.m_device, sizeof(Meta), "matmul_meta");
//        A.m_device.GetQueue().WriteBuffer(metaBuf.get(), 0, &meta, sizeof(Meta));
//
//        // Build params entries: A=0, B=1, Out=2, meta uniform=3
//        std::vector<ParameterSet::Entry> entries = {
//            { const_cast<krnl::Buffer*>(&A.m_buffer), wgpu::ShaderStage::Compute },
//            { const_cast<krnl::Buffer*>(&B.m_buffer), wgpu::ShaderStage::Compute },
//            { const_cast<krnl::Buffer*>(&Out.m_buffer), wgpu::ShaderStage::Compute },
//            { &metaBuf, wgpu::ShaderStage::Compute }
//        };
//        ParameterSet params(A.m_device, entries);
//
//        Pipeline p = makeMatMulPipeline(A.m_device, params, M, N, K);
//
//        // Encode and dispatch
//        wgpu::CommandEncoder encoder = A.m_device.CreateCommandEncoder();
//        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
//
//        // compute workgroups
//        uint32_t wgX = (uint32_t)((N + 7) / 8);
//        uint32_t wgY = (uint32_t)((M + 7) / 8);
//
//        p.encodeDispatch(pass, wgX, wgY, 1);
//        pass.End();
//
//        wgpu::CommandBuffer cmd = encoder.Finish();
//        queue.Submit(1, &cmd);
//
//        return Out;
//    }
//
//} // namespace krnl
