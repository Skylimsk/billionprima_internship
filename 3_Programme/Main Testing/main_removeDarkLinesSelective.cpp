#include <iostream>
#include "darkline_pointer.h"

using namespace std;

int main()
{
    // Step 1: Setup ImageData with dimensions (provided externally)
    ImageData image;
    image.rows = rows; // Number of rows in the image (set externally)
    image.cols = cols; // Number of columns in the image (set externally)

    // Step 2: Detect dark lines in the image
    DarkLineArray *detectedLines = DarkLinePointerProcessor::detectDarkLines(image);

    // Step 3: Use selective removal of dark lines without order
    // The function allows us to specify whether to:
    // - Remove lines inside dark objects
    // - Remove isolated dark lines
    // - Use a specific method for line removal
    DarkLinePointerProcessor::removeDarkLinesSelective(
        image,
        detectedLines,
        true,  // Remove lines that are inside dark objects
        true,  // Remove isolated dark lines
        DarkLinePointerProcessor::RemovalMethod::NEIGHBOR_VALUES // Change to DIRECT_STITCH if needed
    );

    // Output success message
    std::cout << "Selective dark lines removed successfully." << std::endl;

    // Step 4: Clean up allocated memory for detected lines
    DarkLinePointerProcessor::destroyDarkLineArray(detectedLines);

    std::cout << "Process completed successfully!" << std::endl;

    return 0;
}
