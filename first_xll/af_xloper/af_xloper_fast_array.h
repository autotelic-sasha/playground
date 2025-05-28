#pragma once
#include "af_xloper/af_xloper_object_caches.h"

namespace autotelica {
	namespace xloper {

		namespace xl_fast_array {
			// xl_fast_array is a wrapper for FP12 arrays
			// they are a pain to use, but apparently the fastest way to pass lots of number to and from excel
			class xl_fast_array {
				_FP12* _array;
				// auto free means delete the pointer on exit. if you need to use this semantics, do it, but it's a pain.
				bool _auto_free;
				// for fast resizing we may change the values inside _array, but not realocate space
				int32_t _actual_rows;
				int32_t _actual_columns;
				// a trick to access array values by double indexing fast
				// store a pointer to the start of each row in an array, they use those
				std::vector<double*> _rows;

				// dealing with memory for FP12 arrays is a real pain
				// xlAutoFree doesn't work for these
				// once you create one, you own it, but you can't delete it before you return it
				// it's so easy to loose track of them
				// instead, create them in a static cache, then once in a while clear that cache up
				static std::list<std::shared_ptr<xl_fast_array>>& cache() {
					static std::list<std::shared_ptr<xl_fast_array>> _cache;
					return _cache;
				}

				void copy(xl_fast_array& in) {
					_array = in._array;
					_auto_free = in._auto_free;
					_actual_rows = in._actual_rows;
					_actual_columns = in._actual_columns;
					_rows = in._rows;
					in._array = nullptr;
					in._auto_free = false;
					in._actual_rows = 0;
					in._actual_columns = 0;
					in._rows.clear();
				}
				void release_impl() {
					if (_array) {
						free(_array);
						_array = nullptr;
						_actual_rows = 0;
						_actual_columns = 0;
					}
				}
				void make_new(int32_t rows, int32_t columns) {
					free(_array);
					int64_t size = rows * columns;
					// checkout https://www.wiley.com/en-gb/Financial+Applications+using+Excel+Add-in+Development+in+C+%2F+C%2B%2B%2C+2nd+Edition-p-9780470319048
					// section 6.2.2 for the calcualtion below
					int64_t mem_size = sizeof(_FP12) + (size - 1) * sizeof(double);
					_array = (_FP12*)malloc(mem_size);
					if (!_array)
						throw std::runtime_error("Could not allocate _FP12 array large enough.");
					_actual_rows = rows;
					_actual_columns = columns;
					_array->rows = rows;
					_array->columns = columns;
					_rows.clear();
				}
				void init_impl(double v = 0) {
					if (_array)
						std::fill_n(_array->array, _array->rows * _array->columns, v);
				}

			public:
				xl_fast_array(_FP12* array_ = nullptr, bool auto_free_ = false) :
					_array(array_),
					_auto_free(auto_free_),
					_actual_rows(0),
					_actual_columns(0) {
					if (_array) {
						_actual_rows = _array->rows;
						_actual_columns = _array->columns;
					}
					_rows.clear();
				}
				xl_fast_array(int32_t rows, int32_t columns, bool auto_free_ = false) :
					_array(nullptr),
					_auto_free(auto_free_),
					_actual_rows(0),
					_actual_columns(0) {
					make_new(rows, columns);
				}
				xl_fast_array(int32_t rows, int32_t columns, double init_value, bool auto_free_ = false) :
					_array(nullptr),
					_auto_free(auto_free_),
					_actual_rows(0),
					_actual_columns(0) {
					make_new(rows, columns);
					init_impl(init_value);
				}
				~xl_fast_array() {
					if (_auto_free)
						release_impl();
				}
				xl_fast_array(xl_fast_array const& in) {
					copy(const_cast<xl_fast_array&>(in));
				}
				xl_fast_array& operator=(xl_fast_array const& in) {
					copy(const_cast<xl_fast_array&>(in));
				}

				// allocate tries to reuse existing memory, if your stuff can fit into it
				// very handy for reusing in paramters, but tricky to keep track of
				// if it returns true, you got to remember to delete it yourself
				bool allocate(int32_t rows, int32_t columns) {
					if (rows < _actual_rows && columns < _actual_columns) {
						_rows.clear();
						_array->rows = rows;
						_array->columns = columns;
						return false;
					}
					else {
						make_new(rows, columns);
						return true;
					}
				}

				inline FP12* fp12() const { return _array; }
				inline void release() { release_impl(); }
				inline void init(double v) { init_impl(v); }

				inline int32_t rows() const { return _array ? _array->rows : 0; }
				inline int32_t unsafe_rows() const { return _array->rows; }
				inline int32_t columns() const { return _array ? _array->columns : 0; }
				inline int32_t unsafe_columns() const { return _array->columns; }
				inline int32_t actual_rows() const { return _actual_rows; }
				inline int32_t actual_columns() const { return _actual_columns; }

				inline int64_t size() const { return _array ? (unsafe_rows() * unsafe_columns()) : 0; }
				inline int64_t actual_size() const { return _actual_rows * _actual_columns; }
				inline bool empty() const { return !_array || (size() == 0); }

				inline double const* values() const { return _array ? _array->array : nullptr; }
				inline double const* unsafe_values() const { return _array->array; }
				inline double* values() { return _array ? _array->array : nullptr; }
				inline double* unsafe_values() { return _array->array; }

				inline double get(int64_t i) const {
					if (!_array || i >= size())
						throw std::runtime_error("Out of bounds or non existent array access.");
					return _array->array[i];
				}
				inline double unsafe_get(int64_t i) const {
					return _array->array[i];
				}
				inline double& get(int64_t i) {
					if (!_array || i >= size())
						throw std::runtime_error("Out of bounds or non existent array access.");
					return _array->array[i];
				}
				inline double& unsafe_get(int64_t i) {
					return _array->array[i];
				}
				inline void set(int64_t i, double const d) { get(i) = d; }
				inline void unsafe_set(int64_t i, double const d) { unsafe_get(i) = d; }

				inline double get(int32_t row, int32_t column) const {
					if (!_array || row >= rows() || column >= columns())
						throw std::runtime_error("Out of bounds or non existent array access.");
					int64_t i = column + row * columns();
					return _array->array[i];
				}
				inline double unsafe_get(int32_t row, int32_t column) const {
					int64_t i = column + row * columns();
					return _array->array[i];
				}
				inline double& get(int32_t row, int32_t column) {
					if (!_array || row >= rows() || column >= columns())
						throw std::runtime_error("Out of bounds or non existent array access.");
					int64_t i = column + row * columns();
					return _array->array[i];
				}
				inline double& unsafe_get(int32_t row, int32_t column) {
					int64_t i = column + row * columns();
					return _array->array[i];
				}
				inline void set(int32_t row, int32_t column, double const d) { get(row, column) = d; }
				inline void unsafe_set(int32_t row, int32_t column, double const d) { unsafe_get(row, column) = d; }

				// to use clever indexing, you have to prepare the container
				// it may take a while for large arrays, so only worth it if you are going to index a lot
				void prepare_indexing() {
					if (empty()) {
						_rows.clear();
						return;
					}
					_rows.reserve(rows());
					for (size_t i = 0; i < rows(); ++i) {
						_rows.push_back(unsafe_values() + i * columns());
					}
				}
				double* operator[](int r) { return _rows[r]; }
				double const* operator[] (int r) const { return _rows[r]; }

				double* row(int r) { if (r > _rows.size()) throw std::runtime_error("Invalid row access."); return _rows[r]; }
				double const* row(int r) const { if (r > _rows.size()) throw std::runtime_error("Invalid row access."); return _rows[r]; }

				// handling the memory of these FP12 structures is a real pain
				// common suggestions is to create static instances, eek!
				// we'll provide a cache to store instances ... you do have to remember to clear it once in a while though
				static size_t cache_size() { return cache().size(); }
				static void clear_cache() { cache().clear(); }
				static std::shared_ptr<xl_fast_array> make_cached(int32_t rows, int32_t columns) {
					std::shared_ptr<xl_fast_array> ret(new xl_fast_array(rows, columns));
					cache().push_back(ret);
					return ret;
				}
				static std::shared_ptr<xl_fast_array> make_cached(int32_t rows, int32_t columns, double v) {
					std::shared_ptr<xl_fast_array> ret(new xl_fast_array(rows, columns, v));
					cache().push_back(ret);
					return ret;
				}

			};

		}

		namespace xl_object_caches {
			// these are exposed by the XLL by default
			inline size_t af_xl_fast_array_cache_size() { return xl_fast_array::xl_fast_array::cache_size(); }

			inline bool af_xl_clear_fast_array_cache() { xl_fast_array::xl_fast_array::clear_cache(); return true; }

			inline bool af_xl_clear_all_caches() { return af_xl_clear_fast_array_cache() && af_xl_clear_all_object_caches(); }
	
	}
	}
}