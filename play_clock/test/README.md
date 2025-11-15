# Play Clock Module Tests

This directory contains comprehensive test suites for the Play Clock module using the ESP-IDF Unity test framework.

## 📋 Test Structure

### **Unit Tests**

#### `test_radio_comm.cpp` - Radio Communication Tests
- **CRC8 calculation** - Validates checksum computation
- **Frame parsing** - Tests valid/invalid status frame handling
- **System state updates** - Verifies state machine transitions
- **Display states** - Tests STOP/RUN/RESET state handling
- **Timeout detection** - Validates link timeout logic
- **Radio initialization** - Mock hardware initialization tests

#### `test_display_driver.cpp` - Display Driver Tests
- **Display initialization** - Tests LED strip setup
- **Digit rendering** - Validates 0-9 segment patterns
- **Invalid values** - Tests error handling for invalid input
- **Time display** - Tests seconds display functionality
- **Display modes** - Tests different visual modes
- **Link status** - Tests connection indicator
- **Color management** - Validates RGB color handling
- **LED constraints** - Tests hardware limits

### **Integration Tests**

#### `test_integration.cpp` - System Integration Tests
- **System initialization** - Complete startup sequence
- **Message flow** - End-to-end message to display
- **State transitions** - Controller state changes
- **Link timeout/recovery** - Connection management
- **Invalid messages** - Error handling integration
- **Rapid processing** - Performance under load
- **Display effects** - Visual effect integration
- **Memory usage** - Resource consumption
- **Error recovery** - System resilience

## 🚀 Running Tests

### **Prerequisites**
- ESP32 development board
- ESP-IDF development environment
- Unity test framework (included with ESP-IDF)

### **Using ESP-IDF**

```bash
# Navigate to test directory
cd play_clock/

# Build all tests
idf.py build

# Flash tests to ESP32
idf.py flash

# Monitor test output
idf.py monitor

# Run specific test (modify main.cpp in test file)
# Edit the test file's app_main() to run specific tests
```

### **Using PlatformIO**

```bash
# Navigate to play_clock directory
cd play_clock/

# Build test firmware
pio run

# Upload test firmware
pio run --target upload

# Monitor serial output
pio device monitor
```

### **Test Execution Modes**

#### **1. Individual Test Suites**
Edit the `app_main()` function in each test file to run specific tests:

```cpp
extern "C" void app_main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_specific_function);
    UNITY_END();
}
```

#### **2. Complete Test Suite**
Run all tests in a file by leaving `app_main()` unchanged.

#### **3. Hardware-in-the-Loop Tests**
Connect actual hardware for full integration testing.

## 📊 Test Categories

### **Mock Tests** 🧪
- **Purpose**: Test logic without hardware dependency
- **Requirements**: ESP32 development board only
- **Coverage**: ~80% of code logic
- **Speed**: Fast execution (< 1 second)

### **Hardware Tests** 🔌
- **Purpose**: Test with actual LED strips and radio modules
- **Requirements**: Complete hardware setup
- **Coverage**: ~95% of functionality
- **Speed**: Slower (requires hardware interaction)

### **Integration Tests** 🔗
- **Purpose**: Test complete system behavior
- **Requirements**: Full system setup
- **Coverage**: End-to-end scenarios
- **Speed**: Variable (depends on test complexity)

## 🎯 Test Coverage

### **Radio Communication Module**
- ✅ Frame validation
- ✅ CRC computation
- ✅ State management
- ✅ Timeout handling
- ✅ Error recovery

### **Display Driver Module**
- ✅ LED initialization
- ✅ Segment rendering
- ✅ Color management
- ✅ Mode switching
- ✅ Error display

### **System Integration**
- ✅ Message flow
- ✅ State synchronization
- ✅ Error handling
- ✅ Performance validation
- ✅ Memory management

## 🐛 Debugging Tests

### **Common Issues**

#### **Test Failures**
```bash
# Enable verbose logging
idf.py menuconfig
# Component config → Log output → Default log verbosity → Debug
```

#### **Hardware Issues**
- Check GPIO pin configurations
- Verify power supply connections
- Test individual components separately

#### **Memory Issues**
```bash
# Enable heap tracing
idf.py menuconfig
# Component config → ESP32-specific → Enable heap tracing
```

### **Debug Output**
Tests provide detailed logging:
- Test setup/teardown status
- Individual test results
- Performance metrics
- Error descriptions

## 📈 Performance Benchmarks

### **Expected Performance**
- **Radio message processing**: < 10ms
- **Display update**: < 50ms
- **Memory usage**: < 10KB total
- **Test suite execution**: < 30 seconds

### **Monitoring**
```bash
# Monitor memory usage
idf.py monitor | grep "MALLOC"

# Monitor task performance
idf.py monitor | grep "TASK"
```

## 🔧 Test Configuration

### **Test Parameters**
```cpp
// In test files
#define TEST_TIMEOUT_MS 5000
#define TEST_RETRIES 3
#define TEST_MEMORY_LIMIT 10240
```

### **Mock Configuration**
```cpp
// Enable/disable hardware mocking
#define MOCK_RADIO_HARDWARE 1
#define MOCK_LED_HARDWARE 1
```

## 📝 Adding New Tests

### **Test Structure Template**
```cpp
void test_new_functionality(void) {
    ESP_LOGI(TAG, "Testing new functionality");

    // Setup test conditions
    // Execute test
    // Verify results

    TEST_ASSERT_TRUE(condition);
}

extern "C" void app_main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_new_functionality);
    UNITY_END();
}
```

### **Best Practices**
1. **Descriptive test names** - Explain what is being tested
2. **Isolated tests** - Each test should be independent
3. **Clear assertions** - Use appropriate TEST_ASSERT macros
4. **Logging** - Add ESP_LOG statements for debugging
5. **Cleanup** - Use tearDown() for resource cleanup

## 🚨 Limitations

### **Hardware Dependencies**
- Some tests require actual hardware
- Mock tests have limited coverage
- Real-world conditions may vary

### **Test Environment**
- Tests run on ESP32, not development machine
- Serial output required for monitoring
- Timing-dependent tests may vary

## 📚 References

- [ESP-IDF Unit Testing Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/unit-testing.html)
- [Unity Test Framework](https://github.com/ThrowTheSwitch/Unity)
- [ESP32 GPIO Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html)
- [RMT LED Strip Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/rmt.html)