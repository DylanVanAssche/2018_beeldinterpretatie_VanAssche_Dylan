#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, const char** argv)
{
    CommandLineParser parser(argc, argv,
    "{ help h usage ? | | At your service! Shows this message }"
    "{ images i | | Loads a list of images }"
    );

    // Help printing
    if(parser.has("help") || argc <= 1) {
        parser.printMessage();
        return 0;
    }

    // Parser fail
    if (!parser.check())
    {
        parser.printErrors();
        return -1;
    }

    // Check if the user has supplied supported parameters
    if(parser.has("images")) {
      String image = parser.get<String>(0);
      cout << image << endl;
      //Mat image;
      //image = imread( imageName, IMREAD_COLOR );
    }
    return 0;
}
