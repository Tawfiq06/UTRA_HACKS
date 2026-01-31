/**
 * @file test_framework.h
 * @brief Lightweight test framework for Arduino
 * @details Provides test assertions and reporting for hardware testing
 */

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <Arduino.h>

// ============================================================================
// TEST MACROS
// ============================================================================

// Test assertion - checks condition and reports result
#define TEST_ASSERT(condition, message) \
  TestFramework::assert((condition), F(message), F(__FILE__), __LINE__)

// Test assertion with expected value
#define TEST_ASSERT_EQ(expected, actual, message) \
  TestFramework::assertEqual((expected), (actual), F(message), F(__FILE__), __LINE__)

// Test assertion for range
#define TEST_ASSERT_RANGE(value, min, max, message) \
  TestFramework::assertRange((value), (min), (max), F(message), F(__FILE__), __LINE__)

// Test section marker
#define TEST_SECTION(name) \
  TestFramework::beginSection(F(name))

// Test case marker
#define TEST_CASE(name) \
  TestFramework::beginTest(F(name))

// ============================================================================
// TEST RESULT STRUCTURE
// ============================================================================
struct TestResults {
  uint16_t totalTests;
  uint16_t passedTests;
  uint16_t failedTests;
  uint16_t skippedTests;
};

// ============================================================================
// TEST FRAMEWORK CLASS
// ============================================================================
class TestFramework {
public:
  /**
   * @brief Initialize test framework
   * @param baudRate Serial baud rate (default 9600)
   */
  static void init(uint32_t baudRate = 9600);
  
  /**
   * @brief Begin a test section
   * @param name Section name (stored in PROGMEM)
   */
  static void beginSection(const __FlashStringHelper* name);
  
  /**
   * @brief Begin a test case
   * @param name Test name (stored in PROGMEM)
   */
  static void beginTest(const __FlashStringHelper* name);
  
  /**
   * @brief Assert a condition
   * @param condition Boolean condition to test
   * @param message Failure message
   * @param file Source file name
   * @param line Source line number
   * @return true if passed, false if failed
   */
  static bool assert(bool condition, const __FlashStringHelper* message,
                     const __FlashStringHelper* file, int line);
  
  /**
   * @brief Assert equality
   * @param expected Expected value
   * @param actual Actual value
   * @param message Failure message
   * @param file Source file name
   * @param line Source line number
   * @return true if passed, false if failed
   */
  template<typename T>
  static bool assertEqual(T expected, T actual, const __FlashStringHelper* message,
                          const __FlashStringHelper* file, int line) {
    currentTest++;
    if (expected == actual) {
      passed++;
      printPass();
      return true;
    } else {
      failed++;
      printFail(message, file, line);
      Serial.print(F("  Expected: ")); Serial.println(expected);
      Serial.print(F("  Actual: ")); Serial.println(actual);
      return false;
    }
  }
  
  /**
   * @brief Assert value is within range
   * @param value Value to test
   * @param minVal Minimum acceptable value
   * @param maxVal Maximum acceptable value
   * @param message Failure message
   * @param file Source file name
   * @param line Source line number
   * @return true if passed, false if failed
   */
  template<typename T>
  static bool assertRange(T value, T minVal, T maxVal, const __FlashStringHelper* message,
                          const __FlashStringHelper* file, int line) {
    currentTest++;
    if (value >= minVal && value <= maxVal) {
      passed++;
      printPass();
      return true;
    } else {
      failed++;
      printFail(message, file, line);
      Serial.print(F("  Value: ")); Serial.println(value);
      Serial.print(F("  Range: [")); Serial.print(minVal);
      Serial.print(F(", ")); Serial.print(maxVal); Serial.println(F("]"));
      return false;
    }
  }
  
  /**
   * @brief Skip a test
   * @param reason Reason for skipping
   */
  static void skip(const __FlashStringHelper* reason);
  
  /**
   * @brief Print final test summary
   */
  static void printSummary();
  
  /**
   * @brief Get test results
   * @return TestResults structure
   */
  static TestResults getResults();
  
  /**
   * @brief Reset all counters
   */
  static void reset();
  
  /**
   * @brief Wait for user input to continue
   * @param prompt Message to display
   */
  static void waitForInput(const __FlashStringHelper* prompt);
  
  /**
   * @brief Delay with countdown display
   * @param seconds Number of seconds to wait
   * @param message Message to display
   */
  static void countdown(uint8_t seconds, const __FlashStringHelper* message);

private:
  static uint16_t currentTest;
  static uint16_t passed;
  static uint16_t failed;
  static uint16_t skipped;
  static const __FlashStringHelper* currentSection;
  static const __FlashStringHelper* currentTestName;
  
  static void printPass();
  static void printFail(const __FlashStringHelper* message,
                        const __FlashStringHelper* file, int line);
};

#endif // TEST_FRAMEWORK_H
