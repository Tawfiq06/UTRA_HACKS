/**
 * @file test_framework.cpp
 * @brief Lightweight test framework implementation
 */

#include "test_framework.h"

// Static member initialization
uint16_t TestFramework::currentTest = 0;
uint16_t TestFramework::passed = 0;
uint16_t TestFramework::failed = 0;
uint16_t TestFramework::skipped = 0;
const __FlashStringHelper* TestFramework::currentSection = nullptr;
const __FlashStringHelper* TestFramework::currentTestName = nullptr;

// ============================================================================
// INITIALIZATION
// ============================================================================
void TestFramework::init(uint32_t baudRate) {
  Serial.begin(baudRate);
  while (!Serial) { ; }  // Wait for serial port
  
  delay(1000);
  
  Serial.println();
  Serial.println(F("╔═══════════════════════════════════════════════════════════╗"));
  Serial.println(F("║           UTRA HACKS ROBOT TEST FRAMEWORK                 ║"));
  Serial.println(F("╚═══════════════════════════════════════════════════════════╝"));
  Serial.println();
  
  reset();
}

void TestFramework::reset() {
  currentTest = 0;
  passed = 0;
  failed = 0;
  skipped = 0;
}

// ============================================================================
// SECTION AND TEST MARKERS
// ============================================================================
void TestFramework::beginSection(const __FlashStringHelper* name) {
  currentSection = name;
  Serial.println();
  Serial.println(F("───────────────────────────────────────────────────────────"));
  Serial.print(F("│ SECTION: ")); Serial.println(name);
  Serial.println(F("───────────────────────────────────────────────────────────"));
}

void TestFramework::beginTest(const __FlashStringHelper* name) {
  currentTestName = name;
  Serial.print(F("\n  TEST: "));
  Serial.println(name);
}

// ============================================================================
// ASSERTIONS
// ============================================================================
bool TestFramework::assert(bool condition, const __FlashStringHelper* message,
                           const __FlashStringHelper* file, int line) {
  currentTest++;
  if (condition) {
    passed++;
    printPass();
    return true;
  } else {
    failed++;
    printFail(message, file, line);
    return false;
  }
}

void TestFramework::skip(const __FlashStringHelper* reason) {
  currentTest++;
  skipped++;
  Serial.print(F("    [SKIP] "));
  Serial.println(reason);
}

// ============================================================================
// OUTPUT HELPERS
// ============================================================================
void TestFramework::printPass() {
  Serial.println(F("    [PASS] ✓"));
}

void TestFramework::printFail(const __FlashStringHelper* message,
                              const __FlashStringHelper* file, int line) {
  Serial.print(F("    [FAIL] ✗ "));
  Serial.println(message);
  Serial.print(F("    at ")); Serial.print(file);
  Serial.print(F(":")); Serial.println(line);
}

// ============================================================================
// SUMMARY
// ============================================================================
void TestFramework::printSummary() {
  Serial.println();
  Serial.println(F("═══════════════════════════════════════════════════════════"));
  Serial.println(F("                     TEST SUMMARY                          "));
  Serial.println(F("═══════════════════════════════════════════════════════════"));
  
  Serial.print(F("  Total:   ")); Serial.println(currentTest);
  Serial.print(F("  Passed:  ")); Serial.print(passed);
  Serial.print(F(" (")); Serial.print((passed * 100) / max(currentTest, (uint16_t)1));
  Serial.println(F("%)"));
  Serial.print(F("  Failed:  ")); Serial.println(failed);
  Serial.print(F("  Skipped: ")); Serial.println(skipped);
  
  Serial.println();
  if (failed == 0) {
    Serial.println(F("  ★★★ ALL TESTS PASSED ★★★"));
  } else {
    Serial.println(F("  ✗✗✗ SOME TESTS FAILED ✗✗✗"));
  }
  Serial.println(F("═══════════════════════════════════════════════════════════"));
}

TestResults TestFramework::getResults() {
  return {currentTest, passed, failed, skipped};
}

// ============================================================================
// INTERACTIVE HELPERS
// ============================================================================
void TestFramework::waitForInput(const __FlashStringHelper* prompt) {
  Serial.println();
  Serial.print(F(">> ")); Serial.print(prompt);
  Serial.println(F(" (Press Enter to continue)"));
  
  while (!Serial.available()) {
    delay(100);
  }
  while (Serial.available()) {
    Serial.read();
  }
}

void TestFramework::countdown(uint8_t seconds, const __FlashStringHelper* message) {
  Serial.print(F(">> ")); Serial.print(message);
  Serial.print(F(" in "));
  
  for (uint8_t i = seconds; i > 0; i--) {
    Serial.print(i); Serial.print(F("... "));
    delay(1000);
  }
  Serial.println(F("GO!"));
}
