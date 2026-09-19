#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <optional>
#include <iomanip>

namespace isb::cli::presentation {

/// Color codes for terminal output
enum class Color {
    Reset,
    Bold,
    Dim,
    
    // Primary colors
    Black,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White,
    
    // Bright variants
    BrightBlack,
    BrightRed,
    BrightGreen,
    BrightYellow,
    BrightBlue,
    BrightMagenta,
    BrightCyan,
    BrightWhite
};

/// Status badge types for visual indicators
enum class BadgeType {
    Success,    // Green checkmark
    Warning,    // Yellow exclamation
    Error,      // Red X
    Info,       // Blue info
    Unknown     // Gray question mark
};

/// Color policy for controlling ANSI output
enum class ColorPolicy {
    Auto,       // Enable colors only on TTY
    Always,     // Always enable colors
    Never       // Disable colors completely
};

/// Terminal formatting helper
class Formatter {
public:
    explicit Formatter(ColorPolicy policy = ColorPolicy::Auto);
    
    /// Set color policy
    void set_policy(ColorPolicy policy);
    
    /// Check if colors are enabled
    bool colors_enabled() const;
    
    /// Apply color to stream
    std::ostream& color(std::ostream& os, Color color) const;
    
    /// Reset formatting
    std::ostream& reset(std::ostream& os) const;
    
    /// Format a status badge
    std::string badge(BadgeType type, const std::string& text = "") const;
    
    /// Format section header
    std::string section(const std::string& title) const;
    
    /// Format subsection header
    std::string subsection(const std::string& title) const;
    
    /// Format key-value pair with alignment
    std::string kv(const std::string& key, const std::string& value, 
                   std::size_t width = 12) const;
    
    /// Format optional value (shows "Unknown" if empty)
    std::string optional(const std::optional<std::string>& value,
                        const std::string& unknown_text = "Unknown") const;
    
    /// Format integer with unit
    std::string unit(int value, const std::string& unit_str) const;
    
    /// Format temperature
    std::string temperature(std::optional<int> temp_c) const;
    
    /// Format percentage
    std::string percent(std::optional<int> percent_val) const;
    
    /// Format frequency (MHz)
    std::string frequency(std::optional<int> mhz) const;
    
    /// Format power (Watts)
    std::string power(std::optional<int> watts) const;
    
    /// Format memory size (MiB/GiB)
    std::string memory(int mib) const;
    
    /// Create horizontal line from a single-byte character
    std::string line(char c = '-', std::size_t length = 60) const;

    /// Create horizontal line from a UTF-8 string token
    std::string line(const std::string& token, std::size_t length = 60) const;
    
    /// Center text
    std::string center(const std::string& text, std::size_t width = 60) const;
    
    /// Get ANSI color code directly (for inline use)
    std::string ansi_color(Color color) const;
    
    /// Reset formatting
    std::string reset() const;
    
    /// Bold accessor
    std::string bold() const { return ansi_color(Color::Bold); }
    
    /// Dim accessor  
    std::string dim() const { return ansi_color(Color::Dim); }
    
private:
    ColorPolicy policy_;
    bool is_tty_;
    
    static bool detect_tty();
};

/// Banner display helper
class Banner {
public:
    static std::string small(const Formatter& fmt);
    static std::string compact(const Formatter& fmt);
    static std::string minimal(const Formatter& fmt);
};

/// Table formatter for structured data
class Table {
public:
    explicit Table(std::vector<std::string> headers);
    
    void add_row(const std::vector<std::string>& row);
    
    std::string render(const Formatter& fmt) const;
    
private:
    std::vector<std::string> headers_;
    std::vector<std::vector<std::string>> rows_;
    std::vector<std::size_t> col_widths_;
    
    void calculate_widths();
};

/// Progress indicator for long-running operations
class Progress {
public:
    explicit Progress(std::size_t total_steps);
    
    void set_current(std::size_t current);
    void set_message(const std::string& message);
    
    std::string render(const Formatter& fmt) const;
    
private:
    std::size_t total_;
    std::size_t current_;
    std::string message_;
};

/// Helper functions for common formatting tasks
std::string format_capability_state(const std::string& state, const Formatter& fmt);
std::string format_provider_mode(const std::string& mode, const Formatter& fmt);
std::string format_timestamp(const std::string& timestamp, const Formatter& fmt);

} // namespace isb::cli::presentation
