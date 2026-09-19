#include "formatter.hpp"
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <utility>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

namespace isb::cli::presentation {

namespace {

bool is_tty_stream(std::ostream& os) {
    if (&os == &std::cout) {
#ifdef _WIN32
        return _isatty(_fileno(stdout)) != 0;
#else
        return isatty(fileno(stdout)) != 0;
#endif
    }
    if (&os == &std::cerr) {
#ifdef _WIN32
        return _isatty(_fileno(stderr)) != 0;
#else
        return isatty(fileno(stderr)) != 0;
#endif
    }
    return false;
}

} // anonymous namespace

Formatter::Formatter(ColorPolicy policy) 
    : policy_(policy), is_tty_(detect_tty()) {}

void Formatter::set_policy(ColorPolicy policy) {
    policy_ = policy;
}

bool Formatter::colors_enabled() const {
    switch (policy_) {
        case ColorPolicy::Always:
            return true;
        case ColorPolicy::Never:
            return false;
        case ColorPolicy::Auto:
            return is_tty_;
    }
    return false;
}

bool Formatter::detect_tty() {
    return is_tty_stream(std::cout);
}

std::string Formatter::ansi_color(Color color) const {
    if (!colors_enabled()) {
        return "";
    }
    
    switch (color) {
        case Color::Reset: return "\033[0m";
        case Color::Bold: return "\033[1m";
        case Color::Dim: return "\033[2m";
        case Color::Black: return "\033[30m";
        case Color::Red: return "\033[31m";
        case Color::Green: return "\033[32m";
        case Color::Yellow: return "\033[33m";
        case Color::Blue: return "\033[34m";
        case Color::Magenta: return "\033[35m";
        case Color::Cyan: return "\033[36m";
        case Color::White: return "\033[37m";
        case Color::BrightBlack: return "\033[90m";
        case Color::BrightRed: return "\033[91m";
        case Color::BrightGreen: return "\033[92m";
        case Color::BrightYellow: return "\033[93m";
        case Color::BrightBlue: return "\033[94m";
        case Color::BrightMagenta: return "\033[95m";
        case Color::BrightCyan: return "\033[96m";
        case Color::BrightWhite: return "\033[97m";
    }
    return "";
}

std::ostream& Formatter::color(std::ostream& os, Color color) const {
    const auto code = ansi_color(color);
    if (!code.empty()) {
        os << code;
    }
    return os;
}

std::ostream& Formatter::reset(std::ostream& os) const {
    return color(os, Color::Reset);
}

std::string Formatter::reset() const {
    return ansi_color(Color::Reset);
}

std::string Formatter::badge(BadgeType type, const std::string& text) const {
    std::ostringstream oss;
    
    switch (type) {
        case BadgeType::Success:
            oss << ansi_color(Color::BrightGreen) << "✓";
            break;
        case BadgeType::Warning:
            oss << ansi_color(Color::BrightYellow) << "⚠";
            break;
        case BadgeType::Error:
            oss << ansi_color(Color::BrightRed) << "✗";
            break;
        case BadgeType::Info:
            oss << ansi_color(Color::BrightBlue) << "ℹ";
            break;
        case BadgeType::Unknown:
            oss << ansi_color(Color::BrightBlack) << "?";
            break;
    }
    
    if (!text.empty()) {
        oss << ansi_color(Color::Reset) << " " << text;
    } else {
        oss << ansi_color(Color::Reset);
    }
    
    return oss.str();
}

std::string Formatter::section(const std::string& title) const {
    std::ostringstream oss;
    oss << "\n" << ansi_color(Color::Bold) << ansi_color(Color::BrightCyan)
        << title << ansi_color(Color::Reset) << "\n";
    return oss.str();
}

std::string Formatter::subsection(const std::string& title) const {
    std::ostringstream oss;
    oss << "\n" << ansi_color(Color::Bold) << title 
        << ansi_color(Color::Reset) << "\n";
    return oss.str();
}

std::string Formatter::kv(const std::string& key, const std::string& value,
                          std::size_t width) const {
    std::ostringstream oss;
    oss << ansi_color(Color::Dim) << std::setw(width) << std::left << key
        << ansi_color(Color::Reset) << " " << value;
    return oss.str();
}

std::string Formatter::optional(const std::optional<std::string>& value,
                                const std::string& unknown_text) const {
    if (value.has_value()) {
        return value.value();
    }
    return ansi_color(Color::BrightBlack) + unknown_text + ansi_color(Color::Reset);
}

std::string Formatter::unit(int value, const std::string& unit_str) const {
    return std::to_string(value) + " " + unit_str;
}

std::string Formatter::temperature(std::optional<int> temp_c) const {
    if (!temp_c.has_value()) {
        return ansi_color(Color::BrightBlack) + "Unknown" + ansi_color(Color::Reset);
    }
    
    int t = temp_c.value();
    std::ostringstream oss;
    oss << t << " °C";
    
    // Color-code temperature
    if (t < 40) {
        oss << " " << ansi_color(Color::BrightGreen) << "(Good)" << ansi_color(Color::Reset);
    } else if (t < 70) {
        oss << " " << ansi_color(Color::BrightYellow) << "(Normal)" << ansi_color(Color::Reset);
    } else {
        oss << " " << ansi_color(Color::BrightRed) << "(Hot)" << ansi_color(Color::Reset);
    }
    
    return oss.str();
}

std::string Formatter::percent(std::optional<int> percent_val) const {
    if (!percent_val.has_value()) {
        return ansi_color(Color::BrightBlack) + "Unknown" + ansi_color(Color::Reset);
    }
    
    int p = percent_val.value();
    std::ostringstream oss;
    oss << p << "%";
    
    if (p < 30) {
        oss << " " << ansi_color(Color::BrightGreen) << "(Low)" << ansi_color(Color::Reset);
    } else if (p < 70) {
        oss << " " << ansi_color(Color::BrightYellow) << "(Moderate)" << ansi_color(Color::Reset);
    } else {
        oss << " " << ansi_color(Color::BrightRed) << "(High)" << ansi_color(Color::Reset);
    }
    
    return oss.str();
}

std::string Formatter::frequency(std::optional<int> mhz) const {
    if (!mhz.has_value()) {
        return ansi_color(Color::BrightBlack) + "Unknown" + ansi_color(Color::Reset);
    }
    return std::to_string(mhz.value()) + " MHz";
}

std::string Formatter::power(std::optional<int> watts) const {
    if (!watts.has_value()) {
        return ansi_color(Color::BrightBlack) + "Unknown" + ansi_color(Color::Reset);
    }
    return std::to_string(watts.value()) + " W";
}

std::string Formatter::memory(int mib) const {
    if (mib >= 1024) {
        double gib = static_cast<double>(mib) / 1024.0;
        std::ostringstream oss;
        oss.precision(1);
        oss << std::fixed << gib << " GiB";
        return oss.str();
    }
    return std::to_string(mib) + " MiB";
}

std::string Formatter::line(char c, std::size_t length) const {
    return std::string(length, c);
}

std::string Formatter::center(const std::string& text, std::size_t width) const {
    if (text.length() >= width) {
        return text;
    }
    
    std::size_t padding = (width - text.length()) / 2;
    std::ostringstream oss;
    oss << std::string(padding, ' ') << text;
    return oss.str();
}

// Banner implementations
std::string Banner::small(const Formatter& fmt) {
    std::ostringstream oss;
    oss << "\n";
    oss << "  ╔════════════════════════════════════╗\n";
    oss << "  ║     " << fmt.ansi_color(Color::BrightCyan) << "ISB V100 DRIVER" << fmt.ansi_color(Color::Reset) << "          ║\n";
    oss << "  ║   Intelligent Systems Bureau       ║\n";
    oss << "  ║   Tesla V100 Control Layer         ║\n";
    oss << "  ╚════════════════════════════════════╝\n";
    return oss.str();
}

std::string Banner::compact(const Formatter& fmt) {
    std::ostringstream oss;
    oss << fmt.ansi_color(Color::Bold) << fmt.ansi_color(Color::BrightCyan)
        << "ISB V100 DRIVER" << fmt.ansi_color(Color::Reset)
        << " | Intelligent Systems Bureau\n";
    return oss.str();
}

std::string Banner::minimal(const Formatter& fmt) {
    std::ostringstream oss;
    oss << fmt.ansi_color(Color::Bold) << "ISB" << fmt.ansi_color(Color::Reset)
        << "/" << fmt.ansi_color(Color::BrightCyan) << "V100" << fmt.ansi_color(Color::Reset)
        << " Driver\n";
    return oss.str();
}

// Table implementation
Table::Table(std::vector<std::string> headers)
    : headers_(std::move(headers)) {
    col_widths_.resize(headers_.size(), 0);
    calculate_widths();
}

void Table::add_row(const std::vector<std::string>& row) {
    rows_.push_back(row);
    calculate_widths();
}

void Table::calculate_widths() {
    for (std::size_t i = 0; i < headers_.size(); ++i) {
        col_widths_[i] = std::max(col_widths_[i], headers_[i].length());
    }
    
    for (const auto& row : rows_) {
        for (std::size_t i = 0; i < row.size() && i < col_widths_.size(); ++i) {
            col_widths_[i] = std::max(col_widths_[i], row[i].length());
        }
    }
}

std::string Table::render(const Formatter& fmt) const {
    std::ostringstream oss;
    
    // Header row
    oss << fmt.ansi_color(Color::Bold);
    for (std::size_t i = 0; i < headers_.size(); ++i) {
        if (i > 0) oss << " │ ";
        oss << std::setw(col_widths_[i]) << std::left << headers_[i];
    }
    oss << fmt.ansi_color(Color::Reset) << "\n";
    
    // Separator
    for (std::size_t i = 0; i < headers_.size(); ++i) {
        if (i > 0) oss << "─┼─";
        oss << std::string(col_widths_[i], '─');
    }
    oss << "\n";
    
    // Data rows
    for (const auto& row : rows_) {
        for (std::size_t i = 0; i < row.size() && i < col_widths_.size(); ++i) {
            if (i > 0) oss << " │ ";
            oss << std::setw(col_widths_[i]) << std::left << row[i];
        }
        oss << "\n";
    }
    
    return oss.str();
}

// Progress implementation
Progress::Progress(std::size_t total_steps)
    : total_(total_steps), current_(0) {}

void Progress::set_current(std::size_t current) {
    current_ = std::min(current, total_);
}

void Progress::set_message(const std::string& message) {
    message_ = message;
}

std::string Progress::render(const Formatter& fmt) const {
    std::ostringstream oss;
    
    const std::size_t bar_width = 30;
    const std::size_t filled = total_ > 0 ? (current_ * bar_width) / total_ : 0;
    const std::size_t empty = bar_width - filled;
    
    oss << "[";
    oss << fmt.ansi_color(Color::BrightGreen) << std::string(filled, '█') << fmt.ansi_color(Color::Reset);
    oss << fmt.ansi_color(Color::Dim) << std::string(empty, '░') << fmt.ansi_color(Color::Reset);
    oss << "] ";
    
    int percent = total_ > 0 ? (current_ * 100) / total_ : 0;
    oss << percent << "%";
    
    if (!message_.empty()) {
        oss << " " << fmt.ansi_color(Color::Dim) << message_ << fmt.ansi_color(Color::Reset);
    }
    
    return oss.str();
}

// Helper functions
std::string format_capability_state(const std::string& state, const Formatter& fmt) {
    if (state == "Available" || state == "Ready") {
        return fmt.badge(BadgeType::Success, state);
    } else if (state == "Unsupported" || state == "Unavailable") {
        return fmt.badge(BadgeType::Error, state);
    } else if (state == "Unknown") {
        return fmt.badge(BadgeType::Unknown, state);
    } else if (state == "PermissionDenied") {
        return fmt.badge(BadgeType::Warning, "Permission Denied");
    } else if (state == "Partial" || state == "Experimental") {
        return fmt.badge(BadgeType::Warning, state);
    }
    return state;
}

std::string format_provider_mode(const std::string& mode, const Formatter& fmt) {
    if (mode == "real") {
        return fmt.ansi_color(Color::BrightGreen) + "REAL" + fmt.ansi_color(Color::Reset);
    } else if (mode == "mock") {
        return fmt.ansi_color(Color::BrightYellow) + "MOCK" + fmt.ansi_color(Color::Reset);
    } else if (mode == "unavailable") {
        return fmt.ansi_color(Color::BrightRed) + "UNAVAILABLE" + fmt.ansi_color(Color::Reset);
    }
    return mode;
}

std::string format_timestamp(const std::string& timestamp, const Formatter& fmt) {
    if (timestamp.empty() || timestamp == "unknown") {
        return fmt.ansi_color(Color::Dim) + "Unknown" + fmt.ansi_color(Color::Reset);
    }
    return timestamp;
}

} // namespace isb::cli::presentation
