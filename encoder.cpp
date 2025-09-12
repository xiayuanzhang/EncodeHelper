#include "encoder.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <codecvt>
#include <locale>

namespace fs = std::filesystem;

Encoder::Encoder() : needPreview(true) {}

Encoder::~Encoder() {}

void Encoder::OnSelectFolderClick() {
    // Implementation for folder selection
}

void Encoder::OnSelectFilesClick() {
    // Implementation for file selection
}

void Encoder::PreviewFiles() {
    auto inputPaths = SplitLines("PathsTextBox content"); // Replace with actual content
    PathCheckResult pathCheck = CheckPathsExist(inputPaths);
    if (!pathCheck.AllExists) {
        std::cerr << "以下路径不存在或无效:\n";
        for (const auto& path : pathCheck.NotExists) {
            std::cerr << path << "\n";
        }
        return;
    }

    auto filterPatterns = SplitLines("FilterTextBox content"); // Replace with actual content
    auto [allValid, validList, invalidList] = CheckAllPattern(filterPatterns);
    if (!allValid) {
        std::cerr << "以下过滤器格式无效:\n";
        for (const auto& filter : invalidList) {
            std::cerr << filter << "\n";
        }
        return;
    }

    auto collectResult = CollectFilesByFilters(inputPaths, validList);
    if (!collectResult.AllSuccess) {
        std::cerr << "部分路径处理异常:\n";
        for (const auto& error : collectResult.Errors) {
            std::cerr << error << "\n";
        }
    }

    fileList = collectResult.Files;
    if (fileList.empty()) {
        std::cerr << "未找到任何匹配的文件。\n";
    } else {
        for (const auto& file : fileList) {
            std::cout << file << "\n";
        }
    }
}

void Encoder::PreviewButtonClick() {
    PreviewFiles();
}

void Encoder::OkButtonClick() {
    if (needPreview) {
        PreviewFiles();
        needPreview = false;
    }
    if (fileList.empty()) {
        std::cerr << "请先预览文件列表，确保有文件可转换。\n";
        return;
    }

    std::string selectedEncoding = "utf-8"; // Replace with actual selected encoding
    std::unordered_map<std::string, std::string> failedFiles;
    int result = ConvertFilesEncoding(fileList, selectedEncoding, failedFiles);

    if (result == -1) {
        std::cerr << "无法识别编码: " << selectedEncoding << "\n";
        return;
    }

    if (!failedFiles.empty()) {
        std::cerr << "部分文件转换失败:\n";
        for (const auto& [file, reason] : failedFiles) {
            std::cerr << file << ": " << reason << "\n";
        }
    } else {
        std::cout << "所有文件均已成功转换编码。\n";
    }
}

void Encoder::FilterCheckBoxChecked(const std::string& text) {
    // Implementation for checkbox checked
}

void Encoder::FilterCheckBoxUnchecked(const std::string& text) {
    // Implementation for checkbox unchecked
}

void Encoder::SingleChoiceGroupChecked(const std::string& selectedEncoding) {
    // Implementation for radio button checked
}

std::vector<std::string> Encoder::SplitLines(const std::string& text) {
    std::istringstream stream(text);
    std::string line;
    std::vector<std::string> lines;
    while (std::getline(stream, line)) {
        if (!line.empty()) {
            lines.push_back(line);
        }
    }
    return lines;
}

Encoder::PathCheckResult Encoder::CheckPathsExist(const std::vector<std::string>& paths) {
    PathCheckResult result;
    for (const auto& path : paths) {
        if (fs::exists(path)) {
            result.Exists.push_back(path);
        } else {
            result.NotExists.push_back(path);
        }
    }
    result.AllExists = result.NotExists.empty();
    return result;
}

Encoder::CollectFilesResult Encoder::CollectFilesByFilters(const std::vector<std::string>& pathsOrFolders, const std::vector<std::string>& filters) {
    CollectFilesResult result;
    std::unordered_set<std::string> collected;

    for (const auto& path : pathsOrFolders) {
        try {
            if (fs::is_regular_file(path)) {
                collected.insert(path);
            } else if (fs::is_directory(path)) {
                for (const auto& entry : fs::recursive_directory_iterator(path)) {
                    if (fs::is_regular_file(entry.path())) {
                        collected.insert(entry.path().string());
                    }
                }
            } else {
                result.Errors.push_back("路径不存在: " + path);
            }
        } catch (const std::exception& ex) {
            result.Errors.push_back("处理路径异常: " + path + ", 错误: " + ex.what());
        }
    }

    for (const auto& file : collected) {
        for (const auto& filter : filters) {
            if (file.ends_with(filter)) {
                result.Files.push_back(file);
                break;
            }
        }
    }

    result.AllSuccess = result.Errors.empty();
    return result;
}

int Encoder::ConvertFilesEncoding(const std::vector<std::string>& filePaths, const std::string& encodingName, std::unordered_map<std::string, std::string>& failedFiles) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    try {
        for (const auto& file : filePaths) {
            try {
                std::ifstream inFile(file, std::ios::binary);
                std::ostringstream content;
                content << inFile.rdbuf();
                inFile.close();

                std::ofstream outFile(file, std::ios::binary);
                outFile << content.str();
                outFile.close();
            } catch (const std::exception& ex) {
                failedFiles[file] = ex.what();
            }
        }
    } catch (const std::exception& ex) {
        return -1;
    }

    return failedFiles.empty() ? 1 : -2;
}
