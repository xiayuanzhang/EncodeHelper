#ifndef ENCODER_H
#define ENCODER_H

#include <string>
#include <vector>
#include <unordered_map>

class Encoder {
public:
    Encoder();
    ~Encoder();

    void OnSelectFolderClick();
    void OnSelectFilesClick();
    void PreviewFiles();
    void PreviewButtonClick();
    void OkButtonClick();

    void FilterCheckBoxChecked(const std::string& text);
    void FilterCheckBoxUnchecked(const std::string& text);
    void SingleChoiceGroupChecked(const std::string& selectedEncoding);

    static std::vector<std::string> SplitLines(const std::string& text);

    struct PathCheckResult {
        std::vector<std::string> Exists;
        std::vector<std::string> NotExists;
        bool AllExists;
    };

    PathCheckResult CheckPathsExist(const std::vector<std::string>& paths);

    struct CollectFilesResult {
        std::vector<std::string> Files;
        bool AllSuccess;
        std::vector<std::string> Errors;
    };

    CollectFilesResult CollectFilesByFilters(const std::vector<std::string>& pathsOrFolders, const std::vector<std::string>& filters);

    int ConvertFilesEncoding(const std::vector<std::string>& filePaths, const std::string& encodingName, std::unordered_map<std::string, std::string>& failedFiles);

private:
    std::vector<std::string> fileList;
    bool needPreview;
};

#endif // ENCODER_H
