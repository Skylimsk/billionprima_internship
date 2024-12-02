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

    // Step 3: Prepare an array to store the detected lines for processing
    DarkLine **selectedLines = new DarkLine *[detectedLines->rows];
    for (int i = 0; i < detectedLines->rows; ++i)
    {
        selectedLines[i] = new DarkLine[1];
        selectedLines[i][0] = detectedLines->lines[i][0]; // Copy each detected line
    }

    // Step 4: Remove detected dark lines sequentially
    // Here, the method can either be NEIGHBOR_VALUES or DIRECT_STITCH
    // NEIGHBOR_VALUES: Fills the removed lines with pixel values interpolated from their surroundings.
    // DIRECT_STITCH: Directly stitches the image parts together, removing the lines by collapsing the image dimensions.
    DarkLinePointerProcessor::removeDarkLinesSequential(
        image,
        detectedLines,
        selectedLines,
        detectedLines->rows,
        true,  // Remove lines that are inside dark objects
        false, // Do not remove isolated dark lines
        DarkLinePointerProcessor::RemovalMethod::NEIGHBOR_VALUES // Change to DIRECT_STITCH if needed
    );

    std::cout << "Dark lines removed successfully." << std::endl;

    // Step 5: Clean up allocated memory for the selected lines
    for (int i = 0; i < detectedLines->rows; ++i)
    {
        delete[] selectedLines[i];
    }
    delete[] selectedLines;

    // Clean up the detectedLines array
    DarkLinePointerProcessor::destroyDarkLineArray(detectedLines);

    std::cout << "Process completed successfully!" << std::endl;

    return 0;
}
