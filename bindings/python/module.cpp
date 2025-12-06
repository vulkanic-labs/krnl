#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include <krnl.hpp>
#include <vector>
#include <cstdint>

namespace py = pybind11;

//TEST
PYBIND11_MODULE(krnl_python, m) {
	m.doc() = "Python bindings for krnl library";
	py::enum_<krnl::BufferUsageType>(m, "BufferUsageType")
		.value("None", krnl::BufferUsageType::None)
		.value("CopySrc", krnl::BufferUsageType::CopySrc)
		.value("CopyDst", krnl::BufferUsageType::CopyDst)
		.value("Index", krnl::BufferUsageType::Index)
		.value("Vertex", krnl::BufferUsageType::Vertex)
		.value("Uniform", krnl::BufferUsageType::Uniform)
		.value("Storage", krnl::BufferUsageType::Storage)
		.value("Indirect", krnl::BufferUsageType::Indirect)
		.value("QueryResolve", krnl::BufferUsageType::QueryResolve)
		.export_values();
}