#pragma once

#include <charconv>
#include <clixx/clixx.hpp>
#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace zdotool {

// Description of one option understood by a subcommand.
struct OptSpec {
    std::string_view short_name;
    std::string_view long_name;
    bool takes_value = false;

    [[nodiscard]] bool matches(std::string_view name) const noexcept {
        return name == short_name || name == long_name;
    }
};

// clixx only tokenizes options; it has no schema and lets each option consume
// every following non-option token. This adapter adds arity, rejects unknown
// options, and collects the leftover tokens as positional arguments.
class Args {
public:
    Args(int argc, char** argv, std::span<const OptSpec> specs) : specs_(specs) {
        std::vector<std::string> tokens;
        tokens.reserve(static_cast<std::size_t>(argc) * 2);
        bool positional_only = false;

        for (int i = 0; i < argc; ++i) {
            std::string token = argv[i] != nullptr ? argv[i] : "";
            if (i > 0) {
                // Support the usual `--` end-of-options marker.
                if (!positional_only && token == "--") {
                    positional_only = true;
                    continue;
                }
                if (positional_only) {
                    token.insert(token.begin(), sentinel);
                } else if (token.starts_with("--")) {
                    if (const auto equals = token.find('='); equals != std::string::npos) {
                        tokens.push_back(token.substr(0, equals));
                        tokens.push_back(token.substr(equals + 1));
                        continue;
                    }
                }
            }
            tokens.push_back(std::move(token));
        }

        // clixx classifies "-5" (exactly two characters) as an option. Prefix
        // single-digit negative numbers with a sentinel so they stay values.
        for (std::size_t i = 1; i < tokens.size(); ++i) {
            if (is_single_digit_negative(tokens[i])) {
                tokens[i].insert(tokens[i].begin(), sentinel);
            }
        }

        storage_ = std::move(tokens);
        pointers_.reserve(storage_.size());
        for (auto& token : storage_) {
            pointers_.push_back(token.data());
        }
        cli_.emplace(static_cast<int>(pointers_.size()), pointers_.data());
    }

    bool parse() {
        if (!cli_) {
            fail("command line was not initialised");
            return false;
        }

        for (const auto& value : cli_->values()) {
            positional_.emplace_back(clean(value.str()));
        }

        for (const auto& option : cli_->options()) {
            const OptSpec* spec = find_spec(option.name());
            if (spec == nullptr) {
                fail("unknown option '" + option.name() + "'");
                return false;
            }

            const auto& values = option.values();
            const std::size_t consumed = spec->takes_value ? 1U : 0U;

            if (consumed == 1U && values.empty()) {
                fail("option '" + option.name() + "' requires a value");
                return false;
            }

            for (std::size_t i = consumed; i < values.size(); ++i) {
                positional_.emplace_back(clean(values[i].str()));
            }
        }

        return ok();
    }

    [[nodiscard]] bool has(std::string_view short_name, std::string_view long_name) const {
        return find_option(short_name, long_name) != nullptr;
    }

    [[nodiscard]] std::optional<std::string_view> value(std::string_view short_name,
                                                         std::string_view long_name) const {
        const clixx::Option* option = find_option(short_name, long_name);
        if (option == nullptr) {
            return std::nullopt;
        }

        const auto& values = option->values();
        if (values.empty()) {
            return std::nullopt;
        }
        return clean(values.front().str());
    }

    [[nodiscard]] std::optional<int> int_value(std::string_view short_name,
                                               std::string_view long_name) const {
        const auto text = value(short_name, long_name);
        if (!text) {
            return std::nullopt;
        }

        const char* first = text->data();
        const char* last = first + text->size();
        int parsed = 0;
        const auto [ptr, ec] = std::from_chars(first, last, parsed);
        if (ec != std::errc{} || ptr != last) {
            fail("invalid integer '" + std::string{*text} + "' for " + std::string{long_name});
            return std::nullopt;
        }
        return parsed;
    }

    [[nodiscard]] int get_int(std::string_view short_name, std::string_view long_name,
                              int fallback) const {
        return int_value(short_name, long_name).value_or(fallback);
    }

    [[nodiscard]] std::span<const std::string> positional() const noexcept { return positional_; }
    [[nodiscard]] std::string_view error() const noexcept { return error_; }
    [[nodiscard]] bool ok() const noexcept { return error_.empty(); }

private:
    static constexpr char sentinel = '\x01';

    static bool is_single_digit_negative(const std::string& token) noexcept {
        return token.size() == 2 && token[0] == '-' && token[1] >= '0' && token[1] <= '9';
    }

    static std::string_view clean(std::string_view value) noexcept {
        if (!value.empty() && value.front() == sentinel) {
            value.remove_prefix(1);
        }
        return value;
    }

    [[nodiscard]] const OptSpec* find_spec(std::string_view name) const noexcept {
        for (const auto& spec : specs_) {
            if (spec.matches(name)) {
                return &spec;
            }
        }
        return nullptr;
    }

    // Later occurrences win, so `-r 1 --repeat 2` behaves like getopt.
    [[nodiscard]] const clixx::Option* find_option(std::string_view short_name,
                                                   std::string_view long_name) const noexcept {
        const clixx::Option* found = nullptr;
        for (const auto& option : cli_->options()) {
            if (option.name() == short_name || option.name() == long_name) {
                found = &option;
            }
        }
        return found;
    }

    void fail(std::string message) const {
        if (error_.empty()) {
            error_ = std::move(message);
        }
    }

    std::span<const OptSpec> specs_;
    std::vector<std::string> storage_;
    std::vector<char*> pointers_;
    std::optional<clixx::CmdLine> cli_;
    std::vector<std::string> positional_;
    mutable std::string error_;
};

} // namespace zdotool
