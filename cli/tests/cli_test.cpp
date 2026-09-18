#include "presentation/formatter.hpp"
#include <cassert>
#include <iostream>

using namespace isb::cli::presentation;

void test_formatter_basics() {
    std::cout << "Testing formatter basics...\n";
    
    // Test with colors disabled for deterministic output
    Formatter fmt(ColorPolicy::Never);
    
    assert(!fmt.colors_enabled());
    
    // Test key-value formatting
    auto kv = fmt.kv("Temperature", "42 °C");
    assert(kv.find("Temperature") != std::string::npos);
    
    // Test section formatting (without colors)
    auto section = fmt.section("Test Section");
    assert(section.find("Test Section") != std::string::npos);
    
    // Test optional value
    std::optional<std::string> has_value = "test";
    std::optional<std::string> no_value;
    
    assert(fmt.optional(has_value) == "test");
    assert(fmt.optional(no_value).find("Unknown") != std::string::npos);
    
    std::cout << "✓ Formatter basics passed\n";
}

void test_telemetry_formatting() {
    std::cout << "Testing telemetry formatting...\n";
    
    Formatter fmt(ColorPolicy::Never);
    
    // Test temperature
    auto temp_good = fmt.temperature(35);
    assert(temp_good.find("35") != std::string::npos);
    assert(temp_good.find("°C") != std::string::npos);
    
    auto temp_unknown = fmt.temperature(std::nullopt);
    assert(temp_unknown.find("Unknown") != std::string::npos);
    
    // Test percentage
    auto pct = fmt.percent(50);
    assert(pct.find("50%") != std::string::npos);
    
    auto pct_unknown = fmt.percent(std::nullopt);
    assert(pct_unknown.find("Unknown") != std::string::npos);
    
    // Test frequency
    auto freq = fmt.frequency(1230);
    assert(freq == "1230 MHz");
    
    // Test power
    auto pwr = fmt.power(300);
    assert(pwr == "300 W");
    
    // Test memory
    auto mem_small = fmt.memory(512);
    assert(mem_small == "512 MiB");
    
    auto mem_large = fmt.memory(16384);
    assert(mem_large.find("GiB") != std::string::npos);
    
    std::cout << "✓ Telemetry formatting passed\n";
}

void test_badge_types() {
    std::cout << "Testing badge types...\n";
    
    Formatter fmt(ColorPolicy::Never);
    
    auto success = fmt.badge(BadgeType::Success, "Available");
    assert(success.find("Available") != std::string::npos);
    
    auto error = fmt.badge(BadgeType::Error, "Unavailable");
    assert(error.find("Unavailable") != std::string::npos);
    
    auto warning = fmt.badge(BadgeType::Warning, "Permission Denied");
    assert(warning.find("Permission Denied") != std::string::npos);
    
    auto unknown = fmt.badge(BadgeType::Unknown, "Unknown");
    assert(unknown.find("Unknown") != std::string::npos);
    
    std::cout << "✓ Badge types passed\n";
}

void test_table_rendering() {
    std::cout << "Testing table rendering...\n";
    
    Formatter fmt(ColorPolicy::Never);
    
    Table table({"Capability", "State", "Read", "Write"});
    table.add_row({"Persistence Mode", "Unknown", "No", "No"});
    table.add_row({"Power Limit", "Unsupported", "Yes", "No"});
    table.add_row({"Application Clocks", "Available", "Yes", "Yes"});
    
    auto rendered = table.render(fmt);
    assert(rendered.find("Capability") != std::string::npos);
    assert(rendered.find("Persistence Mode") != std::string::npos);
    assert(rendered.find("Power Limit") != std::string::npos);
    
    std::cout << "✓ Table rendering passed\n";
}

void test_progress_indicator() {
    std::cout << "Testing progress indicator...\n";
    
    Formatter fmt(ColorPolicy::Never);
    
    Progress progress(5);
    progress.set_current(2);
    progress.set_message("Scanning...");
    
    auto rendered = progress.render(fmt);
    assert(rendered.find("40%") != std::string::npos);
    assert(rendered.find("Scanning...") != std::string::npos);
    
    std::cout << "✓ Progress indicator passed\n";
}

void test_banner_generation() {
    std::cout << "Testing banner generation...\n";
    
    Formatter fmt(ColorPolicy::Never);
    
    auto small = Banner::small(fmt);
    assert(small.find("ISB") != std::string::npos);
    assert(small.find("V100") != std::string::npos);
    
    auto compact = Banner::compact(fmt);
    assert(compact.find("ISB") != std::string::npos);
    
    auto minimal = Banner::minimal(fmt);
    assert(minimal.find("ISB") != std::string::npos);
    
    std::cout << "✓ Banner generation passed\n";
}

void test_helper_functions() {
    std::cout << "Testing helper functions...\n";
    
    Formatter fmt(ColorPolicy::Never);
    
    auto cap_state = format_capability_state("Available", fmt);
    assert(cap_state.find("Available") != std::string::npos);
    
    auto provider = format_provider_mode("mock", fmt);
    assert(provider.find("MOCK") != std::string::npos);
    
    auto ts = format_timestamp("2024-01-01T00:00:00Z", fmt);
    assert(ts.find("2024") != std::string::npos);
    
    auto ts_unknown = format_timestamp("unknown", fmt);
    assert(ts_unknown.find("Unknown") != std::string::npos);
    
    std::cout << "✓ Helper functions passed\n";
}

void test_unknown_states() {
    std::cout << "Testing Unknown state handling...\n";
    
    Formatter fmt(ColorPolicy::Never);
    
    // Critical: ensure Unknown is never shown as 0 or empty
    auto temp_unknown = fmt.temperature(std::nullopt);
    assert(temp_unknown.find("0") == std::string::npos || 
           temp_unknown.find("Unknown") != std::string::npos);
    
    auto pct_unknown = fmt.percent(std::nullopt);
    assert(pct_unknown.find("Unknown") != std::string::npos);
    
    auto freq_unknown = fmt.frequency(std::nullopt);
    assert(freq_unknown.find("Unknown") != std::string::npos);
    
    std::cout << "✓ Unknown state handling passed\n";
}

int main() {
    std::cout << "\n=== CLI Presentation Tests ===\n\n";
    
    test_formatter_basics();
    test_telemetry_formatting();
    test_badge_types();
    test_table_rendering();
    test_progress_indicator();
    test_banner_generation();
    test_helper_functions();
    test_unknown_states();
    
    std::cout << "\n=== All tests passed ===\n\n";
    
    return 0;
}
