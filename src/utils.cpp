#include "machikado.h"
#include <algorithm>
#include <fstream>
#include <set>

namespace machikado {
    void FileMapping::insert(const std::string& target_path, const std::string& source_path) {
        map_[target_path] = source_path;
    }

    std::size_t FileMapping::size() const noexcept {
        return map_.size();
    }

    bool FileMapping::empty() const noexcept {
        return map_.empty();
    }

    FileMapping::iterator FileMapping::begin() { return map_.begin(); }
    FileMapping::iterator FileMapping::end() { return map_.end(); }
    FileMapping::const_iterator FileMapping::begin() const { return map_.begin(); }
    FileMapping::const_iterator FileMapping::end() const { return map_.end(); }
    FileMapping::const_iterator FileMapping::cbegin() const { return map_.cbegin(); }
    FileMapping::const_iterator FileMapping::cend() const { return map_.cend(); }

    const std::map<std::string, std::string>& FileMapping::map() const noexcept {
        return map_;
    }

    FileMapping FileMapping::from_pair(const std::string& target, const std::string& source) {
        FileMapping m;
        m.insert(target, source);
        return m;
    }

    FileMapping FileMapping::from_pairs(
        std::initializer_list<std::pair<std::string, std::string>> pairs) {
        FileMapping m;
        for (const auto& [target, source] : pairs) {
            m.insert(target, source);
        }
        return m;
    }

    namespace fs = std::filesystem;

    static std::string normalize_path(const fs::path& path) {
        std::string s = path.string();
        std::replace(s.begin(), s.end(), '\\', '/');
        return s;
    }

    std::vector<FileEntry> load_folder_files(
        const fs::path& folder,
        const std::vector<std::string>& ignore_prefixes,
        const std::vector<std::string>& ignore_names,
        const FileMapping* mapping) {

        std::vector<FileEntry> entries;

        std::set<std::string> mapped_sources;
        std::set<std::string> mapped_targets;

        if (mapping && !mapping->empty()) {
            for (const auto& [target, source] : mapping->map()) {
                mapped_sources.insert(source);
                mapped_targets.insert(target);
            }

            for (const auto& [target_path, source_path] : mapping->map()) {
                fs::path full_source = folder / source_path;

                std::ifstream file(full_source, std::ios::binary);
                if (!file.is_open()) {
                    throw std::runtime_error(
                        "failed to read mapped source '" + source_path +
                        "' (-> target '" + target_path + "')");
                }

                std::vector<std::uint8_t> content(
                    (std::istreambuf_iterator<char>(file)),
                    std::istreambuf_iterator<char>());

                entries.push_back(FileEntry{target_path, std::move(content)});
            }
        }

        if (!fs::exists(folder) || !fs::is_directory(folder)) {
            throw std::runtime_error(
                "folder does not exist or is not a directory: " + folder.string());
        }

        for (auto it = fs::recursive_directory_iterator(
                folder, fs::directory_options::skip_permission_denied);
            it != fs::recursive_directory_iterator(); ++it) {

            if (!it->is_regular_file()) continue;

            fs::path rel_path = fs::relative(it->path(), folder);
            std::string rel_str = normalize_path(rel_path);

            if (mapping) {
                if (mapped_sources.count(rel_str) > 0 || mapped_targets.count(rel_str) > 0) {
                    continue;
                }
            }

            bool ignored = false;
            for (const auto& prefix : ignore_prefixes) {
                if (rel_str.size() >= prefix.size() &&
                    rel_str.substr(0, prefix.size()) == prefix) {
                    ignored = true;
                    break;
                }
            }
            if (ignored) continue;

            for (const auto& name : ignore_names) {
                if (rel_str == name) {
                    ignored = true;
                    break;
                }
            }
            if (ignored) continue;

            std::ifstream file(it->path(), std::ios::binary);
            if (!file.is_open()) {
                throw std::runtime_error("failed to read file: " + it->path().string());
            }

            std::vector<std::uint8_t> content(
                (std::istreambuf_iterator<char>(file)),
                std::istreambuf_iterator<char>());

            entries.push_back(FileEntry{rel_str, std::move(content)});
        }

        std::sort(entries.begin(), entries.end(),
                [](const FileEntry& a, const FileEntry& b) {
                    return a.relative_path < b.relative_path;
                });

        return entries;
    }

} // namespace machikado
