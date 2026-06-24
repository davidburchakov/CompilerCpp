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



    class Reports {
    public:
        std::vector<Report> errors;
        std::vector<Report> warnings;
        std::vector<Report> suggestions;
        Reports(const std::vector<Report> &errors, const std::vector<Report> &warnings, const std::vector<Report> &suggestions)
            :errors(errors), warnings(warnings), suggestions(suggestions) { }
        Reports() = default;
    };
};

