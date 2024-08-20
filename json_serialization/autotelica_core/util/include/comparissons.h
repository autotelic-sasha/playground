#pragma once
#include <limits>
#include <math.h>
#include <string.h>

namespace autotelica {
	namespace comparissons {
		class comparissons_config {
			float _float_epsilon;
			double _double_epsilon;
			comparissons_config() {
				_reset();
			}
			static std::shared_ptr<comparissons_config> get() {
				static std::shared_ptr<comparissons_config> instance(new comparissons_config());
				return instance;
			}
			void _reset() {
				_float_epsilon = sqrt(std::numeric_limits<float>::epsilon());
				_double_epsilon = sqrt(std::numeric_limits<double>::epsilon());
			}
		public:
			static float float_epsilon() { return get()->_float_epsilon; }
			static double double_epsilon() { return get()->_double_epsilon; }
			
			static float set_float_epsilon(float const& f) { return get()->_float_epsilon = f; }
			static double set_double_epsilon(double const& d) { return get()->_double_epsilon = d; }

			static void reset() { get()->_reset(); }
		};
		
		namespace impl {
			// adapted from https://embeddeduse.com/2019/08/26/qt-compare-two-floats/
			template< typename T>
			bool fpCompare(T const& v1, T const& v2, T const& epsilon) {
				if (v1 == v2) return true;
				if (fabs(v1 - v2) <= epsilon) return true;
				return fabs(v1 - v2) <= epsilon * std::max(fabs(v1), fabs(v2));

			}
		}

		template< typename T >
		struct are_equal {
			T const& _v1;
			T const& _v2;
			are_equal(T const& v1, T const& v2) :_v1(v1), _v2(v2) {}
			inline operator bool() const { return _v1 == _v2; }
		};
		template< >
		struct are_equal<float> {
			float const& _v1;
			float const& _v2;
			are_equal(float const& v1, float const& v2) :_v1(v1), _v2(v2) {}
			inline operator bool() const { return impl::fpCompare(_v1, _v2, comparissons_config::float_epsilon()); }
		};
		template< >
		struct are_equal<double> {
			double const& _v1;
			double const& _v2;
			are_equal(double const& v1, double const& v2) :_v1(v1), _v2(v2) {}
			inline operator bool() const { return impl::fpCompare(_v1, _v2, comparissons_config::double_epsilon()); }
		};
		template< >
		struct are_equal<const char*> {
			const char* _v1;
			const char* _v2;
			are_equal(const char* v1, const char* v2) :_v1(v1), _v2(v2) {}
			inline operator bool() const { return strcmp(_v1, _v2) == 0; }
		};

		template< typename T >
		bool are_equal_f(T const& _v1, T const& _v2) { return are_equal<T>(_v1, _v2); }

	}
}