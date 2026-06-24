//
// Created by David Burchakov on 6/24/26.
//

module;
#include <utility>
#include <vector>
#include <string>
export module Reports;
import LogColor;

export namespace CppZero {
    enum class ErrorCodeEnum {
        SUCCESS,
        FAILURE,
        WARNING,
        SUGGESTION,
    };

    class Report {
    public:
        std::string reportMsg;
        ErrorCodeEnum error_code;

        explicit Report(std::string report_message, const ErrorCodeEnum error_code_enum = ErrorCodeEnum::FAILURE)
            : error_code(error_code_enum) {

            // Direct string buffer modification via conditional rvalue moving
            switch (error_code) {
                case ErrorCodeEnum::FAILURE:
                    reportMsg = std::string(LogColor::RED) + "[Error] " + LogColor::RESET + std::move(report_message);
                    break;
                case ErrorCodeEnum::WARNING:
                    reportMsg = std::string(LogColor::YELLOW) + "[Warning] " + LogColor::RESET + std::move(report_message);
                    break;
                case ErrorCodeEnum::SUGGESTION:
                    reportMsg = std::string(LogColor::CYAN) + "[Suggestion] " + LogColor::RESET + std::move(report_message);
                    break;
                default:
                    reportMsg = std::move(report_message);
                    break;
            }
        }
    };


template <typename R>
    class Reports {
    public:
        std::vector<R> errors;
        std::vector<R> warnings;
        std::vector<R> suggestions;
        Reports(const std::vector<R> &errors, const std::vector<R> &warnings, const std::vector<R> &suggestions)
            :errors(errors), warnings(warnings), suggestions(suggestions) { }
        Reports() = default;

        bool noErrors() const { return errors.empty(); }
        bool noWarnings() const { return warnings.empty(); }
        bool noSuggestions() const { return suggestions.empty(); }
    };
};

