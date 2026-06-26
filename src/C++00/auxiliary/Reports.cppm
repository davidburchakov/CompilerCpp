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
        kSuccess,
        kFailure,
        kWarning,
        kSuggestion,
        kInfo,
    };

    class Report {
    public:
        explicit Report(std::string report_message, const ErrorCodeEnum error_code_enum = ErrorCodeEnum::kFailure)
            : error_code_(error_code_enum) {
            switch (error_code_) {
                case ErrorCodeEnum::kFailure:
                    report_msg_ = std::string(LogColor::RED) + "[Error] " + LogColor::RESET + std::move(report_message);
                    break;
                case ErrorCodeEnum::kWarning:
                    report_msg_ = std::string(LogColor::YELLOW) + "[Warning] " + LogColor::RESET + std::move(
                                    report_message);
                    break;
                case ErrorCodeEnum::kInfo:
                    report_msg_ = std::string(LogColor::BLUE) + "[Info] " + LogColor::RESET + std::move(report_message);
                    break;
                case ErrorCodeEnum::kSuggestion:
                    report_msg_ = std::string(LogColor::CYAN) + "[Suggestion] " + LogColor::RESET + std::move(
                                    report_message);
                    break;
                default:
                    report_msg_ = std::move(report_message);
                    break;
            }
        }

        std::string_view getMessage() const { return report_msg_; }

    private:
        std::string report_msg_;
        ErrorCodeEnum error_code_;

    };


    template<typename R>
    class Reports {
    public:
        std::vector<R> errors;
        std::vector<R> warnings;
        std::vector<R> suggestions;
        std::vector<R> info;

        Reports(const std::vector<R> &errors, const std::vector<R> &warnings, const std::vector<R> &suggestions,
                const std::vector<R> &info)
            : errors(errors), warnings(warnings), suggestions(suggestions), info(info) {
        }

        Reports() = default;
        [[nodiscard]] bool noErrors() const { return errors.empty(); }
        [[nodiscard]] bool noWarnings() const { return warnings.empty(); }
        [[nodiscard]] bool noSuggestions() const { return suggestions.empty(); }
        [[nodiscard]] bool noInfo() const { return info.empty(); }
    };
};
