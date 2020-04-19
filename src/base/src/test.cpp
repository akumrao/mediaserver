/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */




#include "base/test.h"
#include "base/logger.h"
//#include "base/memory.h"
#include "base/singleton.h"
#include "base/time.h"
#include "base/util.h"

#include <assert.h>
#include <iostream>


using std::cout;
using std::cerr;
using std::endl;


namespace base {
    namespace test {


        static Singleton<TestRunner> singleton;

        void init() {
            // Set the logger to only log warning level and above if no debug
            // channel has been set yet.
            if (!Logger::instance().get("debug", false))
                Logger::instance().add(new ConsoleChannel("debug", Level::Warn));

            // Initialize the default test runner.
            TestRunner::getDefault();

            // Nothing else to do...
        }

        int finalize() {
            bool passed = TestRunner::getDefault().passed();
            singleton.destroy();

            // Finalize the garbage collector to ensure memory if freed before exiting.
            // GarbageCollector::instance().finalize();  //arvind

            return passed ? 0 : 1;
        }

        void runAll() {
            TestRunner::getDefault().run();
        }

        void describe(const std::string& name, std::function<void() > target) {
            auto test = new FunctionTest(target, name);
            TestRunner::getDefault().add(test);
        }

        void describe(const std::string& name, Test* test) {
            test->name = name;
            TestRunner::getDefault().add(test);
        }

        void expectImpl(bool passed, const char* assert, const char* file, long line) {
            if (passed)
                return;

            std::stringstream ss;
            ss << "failed on " << assert << " in " << file << " at " << line;

            auto test = TestRunner::getDefault().current();
            if (test) {
                test->errors.push_back(ss.str());
            }

            std::cout << ss.str() << std::endl;
        }


        //
        // Test Runner
        //

        TestRunner::TestRunner()
        : _current(nullptr) {
        }

        TestRunner::~TestRunner() {
            clear();
        }

        void TestRunner::add(Test* test) {
            cout << test->name << " added" << endl;
            std::lock_guard<std::mutex> guard(_mutex);
            _tests.push_back(test);
        }

        Test* TestRunner::get(const std::string& name) const {
            std::lock_guard<std::mutex> guard(_mutex);
            for (auto it = _tests.begin(); it != _tests.end(); ++it) {
                if ((*it)->name == name)
                    return *it;
            }
            return nullptr;
        }

        void TestRunner::clear() {
            std::lock_guard<std::mutex> guard(_mutex);
            util::clearList<Test>(_tests);
        }

        TestList TestRunner::tests() const {
            std::lock_guard<std::mutex> guard(_mutex);
            return _tests;
        }

        ErrorMap TestRunner::errors() const {
            ErrorMap errors;
            TestList tests = this->tests();
            for (auto it = tests.begin(); it != tests.end(); ++it) {
                if (!(*it)->passed()) {
                    errors[(*it)] = (*it)->errors;
                }
            }
            return errors;
        }

        bool TestRunner::passed() const {
            return errors().empty();
        }

        Test* TestRunner::current() const {
            std::lock_guard<std::mutex> guard(_mutex);
            return _current;
        }

        void TestRunner::run() {
            cout << "===============================================================" << endl;
            // cout << "running all tests" << endl;

            uint64_t start = time::hrtime();
            double duration = 0;
            TestList tests = this->tests();
            for (auto it = tests.begin(); it != tests.end(); ++it) {
                {
                    std::lock_guard<std::mutex> guard(_mutex);
                    _current = *it;
                }
                cout
                        << "---------------------------------------------------------------" << endl;
                cout << _current->name << " starting" << endl;
                uint64_t test_start = time::hrtime();
                try {
                    //_current->start();  // For asyncronous test
                    _current->run();    /// For syncronous test
                } catch (std::exception& exc) {
                    _current->errors.push_back(exc.what());
                    cout << "exception thrown: " << exc.what() << endl;
                }
                _current->duration = (time::hrtime() - test_start) / 1e9;
                cout << _current->name << " ended after " << _current->duration << " seconds" << endl;
            }

            duration = (time::hrtime() - start) / 1e9;

            cout << "---------------------------------------------------------------" << endl;
            cout << "all tests completed after " << duration << " seconds" << endl;
            // cout << "summary: " << endl;

            //uv_thread_sleep( 1000*95);  // For asyncronous test
                    
            for (auto it = tests.begin(); it != tests.end(); ++it) {

             //   if ((*it)->stop()) {

                    if ((*it)->passed()) {
                        cout << (*it)->name << " passed" << endl;
                    } else {
                        cout << (*it)->name << " failed" << endl;
                    }
               // }
            }
        }

        TestRunner& TestRunner::getDefault() {
            return *singleton.get();
        }


        //
        // Test
        //

        Test::Test(const std::string& name)
        : name(name)
        , duration(0) {
        }

        Test::~Test() {
            // cout << "destroying " << name << endl;
        }

        bool Test::stop() {
            // TBD

            true;
        }

        bool Test::passed() {
            return errors.empty();
        }


    } // namespace test
} // namespace base


/// @\}
