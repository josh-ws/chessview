#ifndef VIEWER_HPP
#define VIEWER_HPP

#include <string>
#include <vector>

struct ViewerOptions {
    std::string title;
    int width;
    int height;
    std::vector<std::string> players;
};

inline void RunViewer(const ViewerOptions &options) {}

#endif
