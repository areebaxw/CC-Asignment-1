#ifndef GRAPHVIZCONVERTER_H
#define GRAPHVIZCONVERTER_H

#include <string>
#include <fstream>

using namespace std;

// Helper class to convert graphviz dot files to PNG
class GraphvizConverter {
public:
    // Convert .dot file to .png and delete the .dot file
    static bool convertToPNG(const string& dotFilePath, const string& pngFilePath);
    
    // Check if graphviz is installed
    static bool isGraphvizInstalled();
};

#endif
