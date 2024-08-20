#pragma once
#include "timing.h"
#include "testing_util.h"
#include <thread>
#include <chrono>

namespace autotelica {
    namespace examples {
        namespace timing {
            void sleep() {
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(23124235ns);
            }
            template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
            void examples() {
                // code here will only be run in example runs
                AF_TEST_COMMENT("Timing some sleeps.");
                using namespace autotelica::timing;
                
                timers _timers;
                auto& t1 = _timers.add("Timer1");
                auto& t2 = _timers.add("Timer2");
                auto& t3 = _timers.add("Timer3");
                t1.start();
                t2.start();
                sleep();
                t2.stop();
                t3.start();
                sleep();
                sleep();
                t3.stop();
                t1.stop();
                
                std::cout << _timers << std::endl;

            }
            template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
            void tests() {
                
            }
        }
    }
}

AF_DECLARE_TEST_SET("Timing", timing,
    autotelica::examples::timing::examples<>(), autotelica::examples::timing::tests<>());

