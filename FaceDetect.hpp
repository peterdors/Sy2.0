#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include "ServoBlaster.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <atomic>

using namespace std;
using namespace cv;

struct CenterPoint
{
    int x, y;

    CenterPoint()
    {
        this->x = 0;
        this->y = 0;
    }

    CenterPoint(int _x, int _y)
    {
        this->x = _x;
        this->y = _y;
    }
};

class FaceDetect
{
private:
    Mat frame;
    CascadeClassifier face_cascade;
    VideoCapture capture;
    Rect fface;
    ServoBlaster sb;
    const int kFrameHeight = 480;
    const int kFrameWidth = 640;
    const string kFaceCascadeFile =
        "haarcascade_frontalface_alt.xml";

    atomic<CenterPoint *> center{new CenterPoint(kFrameWidth / 2, kFrameHeight / 2)};

public:
    FaceDetect();

    void Load();

    void Read();

    void Detect(bool& face_found);

    void MoveServos();

    Mat getFrame(); 
};
